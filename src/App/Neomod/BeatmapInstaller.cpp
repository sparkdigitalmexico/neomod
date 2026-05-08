// Copyright (c) 2026, WH, All rights reserved.

#include "BeatmapInstaller.h"

#include "Database.h"
#include "DatabaseBeatmap.h"
#include "Downloader.h"
#include "Engine.h"
#include "Logging.h"
#include "NotificationOverlay.h"
#include "Osu.h"
#include "OsuConfig.h"
#include "SongBrowser/SongBrowser.h"
#include "UI.h"

void BeatmapInstaller::enqueue(i32 set_id, bool auto_select) {
    if(set_id <= 0) return;

    auto [it, inserted] = this->entries.try_emplace(set_id);
    Entry& e = it->second;
    if(inserted) {
        e.stage = Stage::Queued;
        e.auto_select = auto_select;
        return;
    }

    // already tracked: OR in the auto_select intent so re-clicking before completion still navigates on Done
    if(auto_select) e.auto_select = true;

    // if a previous attempt failed, allow retry
    if(e.stage == Stage::Failed) {
        e.dl_handle.reset();
        e.progress = 0.f;
        e.finished_time = 0.0;
        e.stage = Stage::Queued;
    }
}

void BeatmapInstaller::cancel(i32 set_id) {
    if(auto it = this->entries.find(set_id); it != this->entries.end()) {
        // dropping the entry releases our DownloadHandle. if the network callback is still in flight,
        // it keeps the underlying Request alive until the response arrives, then the bytes are discarded.
        this->entries.erase(it);
    }
}

BeatmapInstaller::State BeatmapInstaller::get_state(i32 set_id) const {
    auto it = this->entries.find(set_id);
    if(it == this->entries.end()) return {};
    return {it->second.stage, it->second.progress};
}

void BeatmapInstaller::clear() { this->entries.clear(); }

void BeatmapInstaller::update() {
    if(this->entries.empty()) return;

    const f64 now = engine->getTime();

    for(auto it = this->entries.begin(); it != this->entries.end();) {
        const i32 set_id = it->first;
        Entry& e = it->second;

        switch(e.stage) {
            case Stage::Queued:
            case Stage::Downloading: {
                // download_beatmapset lazily creates the handle on first call (when e.dl_handle is null),
                // then on each subsequent call polls completion and performs extraction once bytes arrive.
                const bool ready = Downloader::download_beatmapset(static_cast<u32>(set_id), e.dl_handle);
                if(ready) {
                    // either freshly extracted, or the directory already existed on disk
                    e.progress = 1.f;
                    e.stage = Stage::Installing;
                } else if(e.dl_handle.failed()) {
                    e.stage = Stage::Failed;
                    e.finished_time = now;
                    this->on_failed(set_id);
                } else {
                    e.progress = e.dl_handle.progress();
                    e.stage = Stage::Downloading;
                }
                break;
            }

            case Stage::Installing: {
                // defensive: only import while db isn't being rebuilt. db->isFinished() flips back to <1
                // during a refresh, and addBeatmapSet would race the loader.
                if(!db->isFinished() || db->isCancelled()) break;

                std::string mapset_path = fmt::format(NEOMOD_MAPS_PATH "/{}/", set_id);
                const DatabaseBeatmap* set = db->addBeatmapSet(mapset_path, set_id);
                if(set == nullptr) {
                    e.stage = Stage::Failed;
                    e.finished_time = now;
                    this->on_failed(set_id);
                } else {
                    e.stage = Stage::Done;
                    e.finished_time = now;
                    this->on_done(set_id, e, set);
                }
                break;
            }

            case Stage::Done:
            case Stage::Failed:
            case Stage::None:
                break;
        }

        // housekeeping: drop Done entries (db is now authoritative; listings query db->getBeatmapSet directly),
        // and expire Failed entries after a TTL so retries are possible without manual reset.
        const bool erase = (e.stage == Stage::Done) ||
                           (e.stage == Stage::Failed && (now - e.finished_time) > FAILED_ENTRY_TTL_S);

        if(erase) {
            it = this->entries.erase(it);
        } else {
            ++it;
        }
    }
}

void BeatmapInstaller::on_done(i32 set_id, const Entry& e, const DatabaseBeatmap* set) {
    debugLog("Finished installing beatmapset {:d}", set_id);

    ui->getNotificationOverlay()->addToast(fmt::format("Downloaded beatmapset #{:d}", set_id), SUCCESS_TOAST);

    if(e.auto_select) {
        const auto& diffs = set->getDifficulties();
        if(diffs.empty()) return;
        ui->getSongBrowser()->onDifficultySelected(diffs[0].get(), false);
    }
}

void BeatmapInstaller::on_failed(i32 set_id) {
    ui->getNotificationOverlay()->addToast(fmt::format("Failed to download beatmapset #{:d} :(", set_id), ERROR_TOAST);
}

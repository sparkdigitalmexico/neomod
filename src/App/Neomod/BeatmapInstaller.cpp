// Copyright (c) 2026, WH, All rights reserved.

#include "BeatmapInstaller.h"

#include "AsyncPool.h"
#include "AsyncTypes.h"
#include "Database.h"
#include "DatabaseBeatmap.h"
#include "Downloader.h"
#include "Engine.h"
#include "Environment.h"
#include "i18n.h"
#include "Logging.h"
#include "NotificationOverlay.h"
#include "Osu.h"
#include "OsuConfig.h"
#include "SongBrowser/SongBrowser.h"
#include "UI.h"

void BeatmapInstaller::enqueue(i32 set_id, bool auto_select, std::string_view display_name) {
    if(set_id <= 0) return;

    // de-dupe against other downloads only: a local import that resolved to the same set coexists
    // harmlessly (try_import/addBeatmapSet are idempotent, the second one just finds a duplicate).
    for(Entry& e : this->entries) {
        if(e.is_local() || e.set_id != set_id) continue;

        // already tracked: OR in the auto_select intent so re-clicking before completion still navigates on Done
        if(auto_select) e.auto_select = true;

        // upgrade an empty name with whatever the new caller has (don't clobber an existing name)
        if(e.display_name.empty() && !display_name.empty()) e.display_name.assign(display_name);

        // if a previous attempt failed, allow retry
        if(e.stage == MapInstallStage::Failed) {
            e.dl_handle.reset();
            e.progress = 0.f;
            e.finished_time = 0.0;
            e.stage = MapInstallStage::Queued;
        }
        return;
    }

    Entry e;
    e.uid = this->next_uid++;
    e.set_id = set_id;
    e.auto_select = auto_select;
    if(!display_name.empty()) e.display_name.assign(display_name);
    this->entries.push_back(std::move(e));
}

void BeatmapInstaller::enqueue_local(std::string osz_path, bool auto_select, bool delete_after) {
    if(osz_path.empty()) return;

    // de-dupe by path so the same file scanned/dropped twice doesn't import twice; a later request
    // to navigate to it still takes effect once it lands.
    bool any_local = false;
    for(Entry& existing : this->entries) {
        if(!existing.is_local()) continue;
        any_local = true;
        if(existing.osz_path == osz_path) {
            if(auto_select) existing.auto_select = true;
            return;
        }
    }

    Entry e;
    e.uid = this->next_uid++;
    e.osz_path = std::move(osz_path);
    e.display_name = Environment::getFileNameFromFilePath(e.osz_path);
    e.delete_after = delete_after;

    // auto-select first enqueued map if we get > 1 at a time
    // NOTE: this logic will work "incorrectly" if we ever try to enqueue a local beatmap to import with auto_select is false
    // and the queue already has non-auto_select local entries in it,
    // but that currently never happens, and this is simpler than re-scanning it or adding more bookkeeping
    e.auto_select = auto_select && !any_local;

    this->entries.push_back(std::move(e));
}

void BeatmapInstaller::cancel(i32 set_id) {
    for(auto it = this->entries.begin(); it != this->entries.end(); ++it) {
        if(it->is_local() || it->set_id != set_id) continue;
        // abort the in-flight transfer (if any), then drop the entry
        Downloader::abort_download(it->dl_handle);
        this->entries.erase(it);
        return;
    }
}

void BeatmapInstaller::cancel_entry(u32 uid) {
    for(auto it = this->entries.begin(); it != this->entries.end(); ++it) {
        if(it->uid != uid) continue;
        // for downloads this aborts the transfer; for local imports the extract future is simply
        // abandoned (a finished extraction leaves maps/<id>/ orphaned, same as quitting mid-extract,
        // and the source .osz stays put for a later import pass)
        Downloader::abort_download(it->dl_handle);
        this->entries.erase(it);
        return;
    }
}

BeatmapInstaller::State BeatmapInstaller::get_state(i32 set_id) const {
    if(set_id <= 0) return {};
    for(const Entry& e : this->entries) {
        if(e.set_id == set_id) return {e.stage, e.progress};
    }
    return {};
}

void BeatmapInstaller::snapshot(std::vector<BeatmapInstaller::EntryView>& out) const {
    out.clear();
    out.reserve(this->entries.size());
    for(const Entry& e : this->entries) {
        out.push_back({.uid = e.uid,
                       .set_id = e.set_id,
                       .stage = e.stage,
                       .progress = e.progress,
                       .display_name = e.display_name});
    }
}

std::vector<BeatmapInstaller::EntryView> BeatmapInstaller::snapshot() const {
    std::vector<EntryView> out;
    snapshot(out);
    return out;
}

void BeatmapInstaller::update() {
    if(this->entries.empty()) return;

    const f64 now = engine->getTime();

    for(auto it = this->entries.begin(); it != this->entries.end();) {
        Entry& e = *it;
        bool severed = false;

        switch(e.stage) {
            using enum MapInstallStage;
            case Queued:
                if(e.is_local()) {
                    // read + decompress + extract on a worker so the main thread never blocks on it.
                    e.extract_future =
                        Async::submit([path = e.osz_path]() -> i32 { return Downloader::read_and_extract_osz(path); },
                                      Lane::Background);
                    e.stage = Extracting;
                    break;
                }
                [[fallthrough]];
            case Downloading: {
                // download_beatmapset lazily creates the handle on first call (when e.dl_handle is null),
                // then on each subsequent call polls completion and performs extraction once bytes arrive.
                const bool ready = Downloader::download_beatmapset(static_cast<u32>(e.set_id), e.dl_handle);
                if(ready) {
                    // either freshly extracted, or the directory already existed on disk
                    e.progress = 1.f;
                    e.stage = Installing;
                } else if(e.dl_handle.failed()) {
                    this->fail(e, now);
                } else if(e.dl_handle.cancelled()) {
                    // transfer aborted externally (Downloader::abort_downloads() on bancho disconnect):
                    // it will never complete or fail, so drop the entry silently.
                    severed = true;
                } else {
                    e.progress = e.dl_handle.progress();
                    e.stage = Downloading;
                }
                break;
            }

            case Extracting: {
                if(!e.extract_future.is_ready()) break;
                e.set_id = e.extract_future.get();
                if(e.set_id <= 0) {
                    this->fail(e, now);
                } else {
                    e.stage = Installing;
                }
                break;
            }

            case Installing: {
                auto [set, imported, ready] = this->try_import(e.set_id);
                if(!ready) break;  // db busy/rebuilding; retry next tick

                if(set == nullptr) {
                    this->fail(e, now);
                } else {
                    e.stage = Done;
                    e.finished_time = now;
                    if(e.is_local()) {
                        if(e.delete_after) env->deleteFile(e.osz_path);
                    } else {
                        ui->getNotificationOverlay()->addToast(tformat("Downloaded beatmapset #{:d}", e.set_id),
                                                               SUCCESS_TOAST);
                    }
                    this->on_done(set, e.set_id, imported, e.auto_select);
                }
                break;
            }

            case Done:
            case Failed:
            case None:
                break;
        }

        // housekeeping: drop Done entries (db is now authoritative; listings query db->getBeatmapSet directly)
        // and severed downloads, and expire Failed entries after a TTL so retries are possible without manual reset.
        if(severed || (e.stage == MapInstallStage::Done) ||
           (e.stage == MapInstallStage::Failed && (now - e.finished_time) > FAILED_ENTRY_TTL_S)) {
            it = this->entries.erase(it);
        } else {
            ++it;
        }
    }
}

BeatmapInstaller::ImportResult BeatmapInstaller::try_import(i32 set_id) {
    // db->isFinished() drops below 1 during a refresh; importing then would race the loader thread
    // appending to beatmapsets, so wait for it to settle (the caller retries next tick).
    if(!db->isFinished() || db->isCancelled()) return {};

    auto [set, added] = db->addBeatmapSet(fmt::format(NEOMOD_MAPS_PATH "/{}/", set_id), set_id);
    return {set, added, true};
}

void BeatmapInstaller::on_done(BeatmapSet* set, i32 set_id, bool added, bool auto_select) {
    // we may have added a duplicate
    debugLog("Finished installing beatmapset {:d}{:s}", set_id, !added ? " (duplicate)" : "");
    if(added) {
        ui->getSongBrowser()->addBeatmapSet(set);
    }

    if(auto_select) {
        const auto& diffs = set->getDifficulties();
        assert(!diffs.empty());  // if we successfully added it, we must have difficulties!

        // TODO: spaghetti
        // (onDifficultySelected just plays music, i.e. we can call it when we are still in online beatmaps screen)
        // otherwise actually select it
        if(ui->getActiveScreen() == ui->getSongBrowserBase()) {
            ui->getSongBrowser()->selectBeatmapset(set);
        } else {
            ui->getSongBrowser()->onDifficultySelected(diffs[0].get(), false);
        }
    }
}

void BeatmapInstaller::fail(Entry& e, f64 now) {
    e.stage = MapInstallStage::Failed;
    e.finished_time = now;
    if(e.is_local()) {
        ui->getNotificationOverlay()->addToast(tformat("Failed to import {}", e.display_name), ERROR_TOAST);
    } else {
        ui->getNotificationOverlay()->addToast(tformat("Failed to download beatmapset #{:d} :(", e.set_id),
                                               ERROR_TOAST);
    }
}

// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "noinclude.h"

#include "types.h"
#include "AsyncFuture.h"
#include "DownloadHandle.h"

#include <string>
#include <string_view>
#include <vector>

class DatabaseBeatmap;
using BeatmapSet = DatabaseBeatmap;

enum class MapInstallStage : u8 {
    None = (1 << 0),
    Queued = (1 << 1),
    Downloading = (1 << 2),
    Installing = (1 << 3),
    Done = (1 << 4),
    Failed = (1 << 5),
    Extracting = (1 << 6),  // local .osz imports only: decompress on a worker before Installing
};

// drives beatmapset import (downloaded sets and local .osz files) as a single async unit, decoupled
// from any UI element. ticked from Osu::update(). both kinds share one queue and one stage machine;
// downloads know their set_id up front, local files resolve theirs on a worker thread (Extracting).
class BeatmapInstaller final {
    NOCOPY_NOMOVE(BeatmapInstaller)
   public:
    struct State {
        MapInstallStage stage{MapInstallStage::None};
        f32 progress{0.f};  // 0..1, only meaningful while Downloading
    };

    // snapshot row used by the global install overlay (and any future consumers).
    // Done entries are erased immediately, so they don't appear; Failed entries linger
    // for FAILED_ENTRY_TTL_S so listings can render the red state.
    struct EntryView {
        u32 uid{0};  // stable per-entry identity (set_id is -1 for unresolved local imports)
        i32 set_id{-1};
        MapInstallStage stage{MapInstallStage::None};
        f32 progress{0.f};
        std::string display_name;  // filename for local imports; empty if caller didn't supply one
    };

    BeatmapInstaller() = default;
    ~BeatmapInstaller() = default;

    // main thread; tick from Osu::update()
    void update();

    // idempotent. if the set is already tracked as a download, only the auto_select bit is OR'd in
    // and display_name is filled iff currently empty (so a later caller with a name
    // upgrades an entry queued earlier without one).
    void enqueue(i32 set_id, bool auto_select, std::string_view display_name = {});

    // import a local .osz file (drag-drop, file association, maps/ watcher). the set id is unknown
    // until the archive is cracked open on a worker thread. delete_after removes the source file once
    // imported (used for the maps/ drop-zone, never for files the user passed in by path).
    // NOTE: auto_select has some quirks you might want to read about if you set it to false (see enqueue_local function body)
    void enqueue_local(std::string osz_path, bool auto_select, bool delete_after);

    // aborts the in-flight transfer (if any) and drops the entry. downloads only; screens
    // key their cancel requests by set id.
    void cancel(i32 set_id);

    // like cancel(), but by entry identity (install overlay X button), so it also reaches local
    // imports and not-yet-resolved entries.
    void cancel_entry(u32 uid);

    [[nodiscard]] State get_state(i32 set_id) const;

    // copy of all current entries (typically <= 5), one row each.
    void snapshot(std::vector<EntryView>& out) const;  // "out" is immediately cleared
    [[nodiscard]] std::vector<EntryView> snapshot() const;

   private:
    // one queued import. two kinds share the stage machine, discriminated by is_local():
    // - download: set_id known up front, dl_handle drives Queued/Downloading
    // - local .osz: osz_path set, extract_future drives Extracting and resolves set_id (-1 until then)
    struct Entry {
        u32 uid{0};
        i32 set_id{-1};
        std::string display_name;
        Downloader::DownloadHandle dl_handle;
        std::string osz_path;
        Async::Future<i32> extract_future;  // resolved set_id (>0) on success, <= 0 on failure
        MapInstallStage stage{MapInstallStage::Queued};
        f32 progress{0.f};
        bool auto_select{false};
        bool delete_after{false};
        f64 finished_time{0.0};

        [[nodiscard]] bool is_local() const { return !this->osz_path.empty(); }
    };

    // result of try_import(): set/imported are valid only when ready == true. ready == false means the
    // db is mid-(re)build and the caller should retry next tick (addBeatmapSet would race the loader).
    struct ImportResult {
        BeatmapSet* set{nullptr};
        bool imported{false};
        bool ready{false};
    };

    // shared Installing-stage tail for downloads and local imports: imports the already-extracted
    // maps/<set_id>/ folder once the db is idle.
    ImportResult try_import(i32 set_id);

    void on_done(BeatmapSet* set, i32 set_id, bool added, bool auto_select);
    void fail(Entry& e, f64 now);

    std::vector<Entry> entries;  // typically <= 5 entries, so linear scans throughout
    u32 next_uid{1};

    // how long to keep Failed entries around so listings can render the red state
    static constexpr f64 FAILED_ENTRY_TTL_S = 60.0;
};

MAKE_FLAG_ENUM(MapInstallStage)

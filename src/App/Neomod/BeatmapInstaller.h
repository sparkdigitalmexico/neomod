// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "DownloadHandle.h"
#include "Hashing.h"
#include "noinclude.h"
#include "types.h"

class DatabaseBeatmap;

// drives beatmapset download + extraction + database import as a single async unit,
// keyed by set_id and decoupled from any UI element. ticked from Osu::update().
class BeatmapInstaller final {
    NOCOPY_NOMOVE(BeatmapInstaller)
   public:
    enum class Stage : u8 { None, Queued, Downloading, Installing, Done, Failed };

    struct State {
        Stage stage{Stage::None};
        f32 progress{0.f};  // 0..1, only meaningful while Downloading
    };

    BeatmapInstaller() = default;
    ~BeatmapInstaller() = default;

    // main thread; tick from Osu::update()
    void update();

    // idempotent. if the set is already tracked, only the auto_select bit is OR'd in.
    void enqueue(i32 set_id, bool auto_select);

    // best-effort: drops the entry. an in-flight HTTP transfer keeps running and its bytes are discarded.
    void cancel(i32 set_id);

    [[nodiscard]] State get_state(i32 set_id) const;

    // drop everything (e.g. on bancho disconnect)
    void clear();

   private:
    struct Entry {
        Downloader::DownloadHandle dl_handle;
        Stage stage{Stage::Queued};
        f32 progress{0.f};
        bool auto_select{false};
        f64 finished_time{0.0};
    };

    void on_done(i32 set_id, const Entry& e, const DatabaseBeatmap* set);
    void on_failed(i32 set_id);

    Hash::flat::map<i32, Entry> entries;

    // how long to keep Failed entries around so listings can render the red state
    static constexpr f64 FAILED_ENTRY_TTL_S = 60.0;
};

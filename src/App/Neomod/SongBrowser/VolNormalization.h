#pragma once
// Copyright (c) 2024, kiwec, All rights reserved.
#include "noinclude.h"
#include "types.h"

#include <memory>
#include <vector>

class DatabaseBeatmap;
class VolNormalization {
    NOCOPY_NOMOVE(VolNormalization)
   public:
    VolNormalization() = default;
    ~VolNormalization();

    // start a batch calculation over the given maps. replaces any in-flight batch but
    // leaves the persistent priority worker and its queue untouched.
    static inline void start_calc(const std::vector<DatabaseBeatmap*>& maps_to_calc) {
        get_instance().start_calc_instance(maps_to_calc);
    }

    static inline u32 get_total() { return get_instance().get_total_instance(); }

    static inline u32 get_computed() { return get_instance().get_computed_instance(); }

    // abort the batch only. the persistent priority worker continues to serve one-off requests.
    static inline void abort() { get_instance().abort_instance(); }

    // enqueue a one-off priority request. returns immediately; the result lands in map->loudness
    // (real value on success, fallback loudness on failure). deduped against in-flight requests.
    // priority work bypasses shouldPauseBGThreads() so user-facing waits are short.
    static inline void request_priority(DatabaseBeatmap* map) { get_instance().request_priority_instance(map); }

    // drop pending priority requests without joining the worker. used by Database before tearing
    // down beatmap pointers on a reload (the queued raw pointers would dangle otherwise).
    static inline void flush_priority() { get_instance().flush_priority_instance(); }

    // full shutdown: abort batch and join the priority worker.
    static inline void shutdown() { get_instance().shutdown_instance(); }

    // convar callback
    static void loudness_cb();

   private:
    static VolNormalization& get_instance();

    void start_calc_instance(const std::vector<DatabaseBeatmap*>& maps_to_calc);

    u32 get_total_instance();
    u32 get_computed_instance();

    void abort_instance();
    void request_priority_instance(DatabaseBeatmap* map);
    void flush_priority_instance();
    void shutdown_instance();

    struct PriorityWorker;
    struct LoudnessCalcThread;
    std::vector<std::unique_ptr<LoudnessCalcThread>> threads;
    std::unique_ptr<PriorityWorker> prio;
};

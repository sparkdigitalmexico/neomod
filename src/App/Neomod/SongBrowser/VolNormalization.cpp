// Copyright (c) 2024, kiwec, All rights reserved.
#include "VolNormalization.h"

#include "OsuConVars.h"
#include "DatabaseBeatmap.h"
#include "Database.h"
#include "Osu.h"
#include "Engine.h"
#include "Sound.h"
#include "SoundEngine.h"
#include "Thread.h"
#include "Timing.h"
#include "Logging.h"
#include "SyncOnce.h"
#include "SyncJthread.h"
#include "SyncCV.h"
#include "SyncMutex.h"
#include "ContainerRanges.h"
#include "Hashing.h"
#include "UniString.h"

#ifdef MCENGINE_FEATURE_BASS
#include "BassManager.h"
#endif

#ifdef MCENGINE_FEATURE_SOLOUD
#include "soloud_wavstream.h"
#include "soloud_loudness.h"
#include "soloud_file.h"
#include "soloud_error.h"
#include "File.h"
#endif

#include <atomic>
#include <deque>
#include <unordered_map>
#include <utility>

namespace {
// the audio backend in use is fixed at startup (the soundEngine can be restarted but never
// swapped between BASS and SoLoud), so each worker picks its backend once and caches it.
enum class Backend : u8 { NONE, BASS, SOLOUD };

// per-worker scratch state. holds the dedup cache for the audio file we just decoded
// (different diffs in a beatmapset share an audio file), plus the BASS scratch buffer.
struct WorkerCtx {
    Backend backend{Backend::NONE};
    std::string last_song;
    f32 last_loudness{0.f};
#ifdef MCENGINE_FEATURE_BASS
    std::array<f32, 44100> bass_buf{};
#endif
#ifdef MCENGINE_FEATURE_SOLOUD
    SoLoud::WavStream soloud_ws{};
#endif
};

// one-shot per-worker setup: pick backend, refresh fallback from cvar, run BASS init if needed.
// must be called once before the worker's first process_one() call.
void init_worker_ctx(WorkerCtx &ctx) {
#ifdef MCENGINE_FEATURE_BASS
    if(soundEngine->getTypeId() == SoundEngine::BASS) {
        ctx.backend = Backend::BASS;
        while(!BassManager::isLoaded()) {  // this should never happen, but just in case
            Timing::sleepMS(100);
        }
        BASS_SetDevice(0);
        BASS_SetConfig(BASS_CONFIG_UPDATETHREADS, 0);
        return;
    }
#endif
#ifdef MCENGINE_FEATURE_SOLOUD
    if(soundEngine->getTypeId() == SoundEngine::SOLOUD) {
        ctx.backend = Backend::SOLOUD;
        ctx.soloud_ws.setLooping(false);
        ctx.soloud_ws.setAutoStop(true);
        return;
    }
#endif
}

#ifdef MCENGINE_FEATURE_BASS
// returns the integrated loudness (real or fallback). never returns 0.f so the caller's
// atomic store unambiguously means "calculated".
f32 calc_one_bass(DatabaseBeatmap *map, WorkerCtx &ctx, f32 fallback_loudness) {
    struct UString {
        UString(std::string_view path) : narrow(path) {
            if constexpr(Env::cfg(OS::WINDOWS)) {
                wide = UniString::to_wide(narrow);
            }
        }
        [[nodiscard]] auto plat_str() const {
            if constexpr(Env::cfg(OS::WINDOWS)) {
                return wide.c_str();
            } else {
                return narrow.c_str();
            }
        }
        std::string narrow;
        std::wstring wide;
    };

    const std::string song_path = map->getFullSoundFilePath();
    if(song_path == ctx.last_song) {
        return ctx.last_loudness;
    }

    UString song{song_path};

    constexpr unsigned int flags = BASS_STREAM_DECODE | BASS_SAMPLE_MONO | (Env::cfg(OS::WINDOWS) ? BASS_UNICODE : 0U);
    auto decoder = BASS_StreamCreateFile(BASS_FILE_NAME, song.plat_str(), 0, 0, flags);
    if(!decoder) {
        if(cv::debug_snd.getBool()) {
            BassManager::printBassError(fmt::format("BASS_StreamCreateFile({:s})", song.narrow), BASS_ErrorGetCode());
        }
        return fallback_loudness;
    }

    auto loudness = BASS_Loudness_Start(decoder, BASS_LOUDNESS_INTEGRATED, 0);
    if(!loudness) {
        BassManager::printBassError("BASS_Loudness_Start()", BASS_ErrorGetCode());
        BASS_ChannelFree(decoder);
        return fallback_loudness;
    }

    for(int res = 0; res >= 0; res = (int)BASS_ChannelGetData(decoder, ctx.bass_buf.data(), ctx.bass_buf.size())) {
    }

    BASS_ChannelFree(decoder);

    f32 integrated_loudness = fallback_loudness;
    const bool succeeded = BASS_Loudness_GetLevel(loudness, BASS_LOUDNESS_INTEGRATED, &integrated_loudness);
    const int errc = succeeded ? 0 : BASS_ErrorGetCode();

    BASS_Loudness_Stop(loudness);

    if(!succeeded || integrated_loudness == -HUGE_VAL) {
        debugLog("No loudness information available for '{:s}' {}", song.narrow,
                 !succeeded ? BassManager::getErrorString(errc) : "(silent song?)");
        integrated_loudness = fallback_loudness;
    }

    ctx.last_song = song_path;
    ctx.last_loudness = integrated_loudness;
    return integrated_loudness;
}
#endif

#ifdef MCENGINE_FEATURE_SOLOUD
f32 calc_one_soloud(DatabaseBeatmap *map, WorkerCtx &ctx, f32 fallback_loudness) {
    const std::string song_path = map->getFullSoundFilePath();
    if(song_path == ctx.last_song) {
        return ctx.last_loudness;
    }

    FILE *fp = File::fopen_c(song_path.c_str(), "rb");
    if(!fp) {
        logIfCV(debug_snd, "Failed to open '{:s}' for loudness calc", song_path.c_str());
        return fallback_loudness;
    }

    SoLoud::DiskFile df(fp);
    if(ctx.soloud_ws.loadFile(&df) != SoLoud::SO_NO_ERROR) {
        logIfCV(debug_snd, "Failed to decode '{:s}' for loudness calc", song_path.c_str());
        return fallback_loudness;
    }

    f32 integrated_loudness = fallback_loudness;
    SoLoud::result ret = SoLoud::Loudness::integratedLoudness(ctx.soloud_ws, integrated_loudness);

    if(ret != SoLoud::SO_NO_ERROR || integrated_loudness == -HUGE_VAL) {
        debugLog("No loudness information available for '{:s}' {}", song_path.c_str(),
                 ret != SoLoud::SO_NO_ERROR ? "(decode error)" : "(silent song?)");
        integrated_loudness = fallback_loudness;
    }

    ctx.last_song = song_path;
    ctx.last_loudness = integrated_loudness;
    return integrated_loudness;
}
#endif

// process a single map: handles skip-if-done, backend dispatch, atomic store.
// caller must have run init_worker_ctx(ctx) first. returns true if the map was actually
// processed (vs. skipped because loudness was already set).
bool process_one(DatabaseBeatmap *map, WorkerCtx &ctx) {
    if(!map) return false;
    if(map->loudness.load(std::memory_order_acquire) != 0.f) return false;

    f32 result = std::clamp<f32>(cv::loudness_fallback.getFloat(), -16.f, 0.f);

    switch(ctx.backend) {
#ifdef MCENGINE_FEATURE_BASS
        case Backend::BASS:
            result = calc_one_bass(map, ctx, result);
            break;
#endif
#ifdef MCENGINE_FEATURE_SOLOUD
        case Backend::SOLOUD:
            result = calc_one_soloud(map, ctx, result);
            break;
#endif
        default:
            break;
    }

    map->loudness.store(result, std::memory_order_release);
    return true;
}

}  // namespace

struct VolNormalization::LoudnessCalcThread {
    NOCOPY_NOMOVE(LoudnessCalcThread)
   public:
    std::atomic<u32> nb_computed{0};
    std::atomic<u32> nb_total;

    LoudnessCalcThread(std::vector<DatabaseBeatmap *> maps_to_calc)
        : nb_total(maps_to_calc.size() + 1),
          maps(std::move(maps_to_calc)),
          thr([this](const Sync::stop_token &stoken) { return this->run(stoken); }) {}

    ~LoudnessCalcThread() = default;

   private:
    std::vector<DatabaseBeatmap *> maps;
    Sync::jthread thr;

    void run(const Sync::stop_token &stoken) {
        McThread::set_current_thread_name("loudness_calc");
        McThread::set_current_thread_prio(McThread::Priority::LOW);

        WorkerCtx ctx;
        init_worker_ctx(ctx);

        for(auto *map : this->maps) {
            while(osu->shouldPauseBGThreads() && !stoken.stop_requested()) {
                Timing::sleepMS(100);
            }
            Timing::sleep(0);

            if(stoken.stop_requested()) return;

            process_one(map, ctx);
            this->nb_computed++;
        }

        this->nb_computed++;
    }
};

// persistent priority worker: a single long-lived thread that serves one-off requests
// queued by request_priority(). bypasses shouldPauseBGThreads() since these are on the
// critical path of "user clicks map -> hears music".
struct VolNormalization::PriorityWorker {
    NOCOPY_NOMOVE(PriorityWorker)
   public:
    PriorityWorker() : thr([this](const Sync::stop_token &stoken) { return this->run(stoken); }) {}

    // members are destroyed in reverse declaration order: thr first, whose destructor calls
    // request_stop() + join(). stoppable_condvar's wait wakes natively on stop_request via the
    // nsync stop_note, so no manual notify is needed.
    ~PriorityWorker() = default;

    void enqueue(DatabaseBeatmap *map) {
        if(!map) return;
        if(map->loudness.load(std::memory_order_acquire) != 0.f) return;

        {
            Sync::unique_lock lock(this->mtx);
            if(this->queued.contains(map)) return;
            this->queue.push_back(map);
            this->queued.insert(map);
        }
        this->cv.notify_one();
    }

    void drop_pending() {
        Sync::unique_lock lock(this->mtx);
        this->queue.clear();
        this->queued.clear();
    }

   private:
    Sync::mutex mtx;
    Sync::stoppable_condvar cv;
    std::deque<DatabaseBeatmap *> queue;
    Hash::flat::set<DatabaseBeatmap *> queued;  // dedup against in-flight queue contents
    Sync::jthread thr;

    void run(const Sync::stop_token &stoken) {
        McThread::set_current_thread_name("loudness_prio");
        McThread::set_current_thread_prio(McThread::Priority::LOW);

        WorkerCtx ctx;
        init_worker_ctx(ctx);

        while(!stoken.stop_requested()) {
            DatabaseBeatmap *map = nullptr;
            {
                Sync::unique_lock lock(this->mtx);
                this->cv.wait(lock, stoken, [this] { return !this->queue.empty(); });
                if(stoken.stop_requested()) return;

                map = this->queue.front();
                this->queue.pop_front();
                this->queued.erase(map);
            }

            process_one(map, ctx);
        }
    }
};

void VolNormalization::loudness_cb() {
    // Restart loudness calc.
    VolNormalization::abort();
    if(db && cv::normalize_loudness.getBool()) {
        VolNormalization::start_calc(db->loudness_to_calc);
    }
}

u32 VolNormalization::get_computed_instance() {
    u32 x = 0;
    for(const auto &thr : this->threads) {
        x += thr->nb_computed.load(std::memory_order_acquire);
    }
    return x;
}

u32 VolNormalization::get_total_instance() {
    u32 x = 0;
    for(const auto &thr : this->threads) {
        x += thr->nb_total.load(std::memory_order_acquire);
    }
    return x;
}

void VolNormalization::start_calc_instance(const std::vector<DatabaseBeatmap *> &maps_to_calc) {
    this->abort_instance();
    if(maps_to_calc.empty()) return;
    if(!cv::normalize_loudness.getBool()) return;

    // group maps by audio file so each file is only decoded once
    // (due to diffs in a beatmapset sharing the same audio file)
    std::unordered_map<std::string, std::vector<DatabaseBeatmap *>> by_file;
    by_file.reserve(maps_to_calc.size());
    for(auto map : maps_to_calc) {
        by_file[map->getFullSoundFilePath()].push_back(map);
    }

    // flatten into a list of groups, then distribute whole groups across threads
    // so no audio file is split between threads
    std::vector<std::vector<DatabaseBeatmap *>> groups;
    groups.reserve(by_file.size());
    for(auto &[_, maps] : by_file) {
        groups.push_back(std::move(maps));
    }

    i32 nb_threads = cv::loudness_calc_threads.getInt();
    if(nb_threads <= 0) {
        // dividing by 2 still burns cpu if hyperthreading is enabled, let's keep it at a sane amount of threads
        nb_threads = std::max(McThread::get_logical_cpu_count() / 3, 1);
    }
    const i32 nb_groups = static_cast<int>(groups.size());
    if(groups.size() < nb_threads) nb_threads = nb_groups;
    int chunk_size = nb_groups / nb_threads;
    int remainder = nb_groups % nb_threads;

    auto it = groups.begin();
    for(int i = 0; i < nb_threads; i++) {
        int cur_chunk_size = chunk_size + (i < remainder ? 1 : 0);

        std::vector<DatabaseBeatmap *> chunk;
        for(int j = 0; j < cur_chunk_size; j++) {
            auto &group = *(it + j);
            Mc::append_range(chunk, group);
        }
        it += cur_chunk_size;

        this->threads.emplace_back(std::make_unique<LoudnessCalcThread>(std::move(chunk)));
    }
}

void VolNormalization::abort_instance() { this->threads.clear(); }

void VolNormalization::request_priority_instance(DatabaseBeatmap *map) {
    if(!map) return;
    if(!cv::normalize_loudness.getBool()) return;
    if(!this->prio) {
        this->prio = std::make_unique<PriorityWorker>();
    }
    this->prio->enqueue(map);
}

void VolNormalization::flush_priority_instance() {
    if(this->prio) this->prio->drop_pending();
}

void VolNormalization::shutdown_instance() {
    this->abort_instance();
    this->prio.reset();
}

VolNormalization::~VolNormalization() {
    cv::loudness_calc_threads.removeAllCallbacks();
    // only clean up this instance's resources
    this->shutdown_instance();
}

VolNormalization &VolNormalization::get_instance() {
    static VolNormalization instance;
    static Sync::once_flag once;

    Sync::call_once(once, []() { cv::loudness_calc_threads.setCallback(CFUNC(VolNormalization::loudness_cb)); });

    return instance;
}

#pragma once
// Copyright (c) 2026, WH, All rights reserved.
// for processing stdin

#include "config.h"
#include "noinclude.h"

#ifndef MCENGINE_FEATURE_WASM
#include "SyncJthread.h"
#include "SyncMutex.h"
#endif

#include <deque>
#include <string>

namespace Mc {
class ConsoleReader final {
    NOCOPY_NOMOVE(ConsoleReader)
   public:
    ConsoleReader();
    ~ConsoleReader();

    void processStdin(double current_time);

   private:
    void stdinReaderThread(const Sync::stop_token &stopToken);

    int stdinWaitFrames{0};        // @wait support: skip N frames before processing more commands
    double stdinWaitDeadline{0.};  // @wait_secs support: wait N seconds before processing more commands

    std::deque<std::string> stdinQueue;
    // on WASM, stdin is polled from the main thread via JS (pthreads can't do blocking stdin reads)

#ifndef MCENGINE_FEATURE_WASM
    Sync::mutex stdinMutex;
    Sync::jthread stdinThread;  // only started if stdin wasn't fully consumed in the ctor pre-drain
#endif
};
}  // namespace Mc

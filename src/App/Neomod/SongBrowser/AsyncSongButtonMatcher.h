#pragma once
// Copyright (c) 2016 PG, All rights reserved.
#include "AsyncCancellable.h"

#include <string>
#include <vector>
#include <memory>

class SongButton;
namespace AsyncSongButtonMatcher {

// the match runs on a background thread, so the caller snapshots on the main thread everything
// the search touches that isn't immutable-after-load:
//   - the flat list of difficulty buttons to flag (so their children can't be re-sorted under us)
//   - each beatmap's star rating, since it depends on the global StarPrecalc::active_idx and mutates
//     a per-beatmap cache on read; all other beatmap metadata is fixed at load and safe to read async (probably!)
struct SearchEntry {
    SongButton *button;
    float stars;
};

Async::CancellableHandle<void> submitSearchMatch(std::vector<SearchEntry> entries, std::string searchString,
                                                 std::string hardcodedSearchString, float speedMultiplier);
}  // namespace AsyncSongButtonMatcher

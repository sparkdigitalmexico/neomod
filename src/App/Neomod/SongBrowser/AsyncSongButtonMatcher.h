#pragma once
// Copyright (c) 2016 PG, All rights reserved.
#include "AsyncCancellable.h"

#include <string>
#include <vector>
#include <memory>

class SongButton;
namespace AsyncSongButtonMatcher {

// in order to avoid the possible case where song buttons are having their children sorted
// while we are iterating over them (async), we need to pre-create the data to-be-iterated-over
// and pass that in
// TODO: still ugly (but safe(r))
Async::CancellableHandle<void> submitSearchMatch(std::vector<SongButton *> songButtons,
                                                 std::string searchString, std::string hardcodedSearchString,
                                                 float speedMultiplier);
}  // namespace AsyncSongButtonMatcher

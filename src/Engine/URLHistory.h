#pragma once
// Copyright (c) 2026, kiwec, All rights reserved.
#include "config.h"
#include <string>

// Partially implements Web History API on wasm, or simulates it on other platforms
// See https://developer.mozilla.org/en-US/docs/Web/API/History_API
//
// "Partially" does heavy lifting here, for simplicity we only allow replacing the current URL path.
// And even then, we don't care about the "state" part of replaceState.

namespace Mc::URLHistory {

void replaceState(const std::string &path);

}  // namespace Mc::URLHistory

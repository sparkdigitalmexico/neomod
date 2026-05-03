// Copyright (c) 2026, kiwec, All rights reserved.
#include "URLHistory.h"

#ifdef __EMSCRIPTEN__

#include <emscripten/em_js.h>

EM_JS(void, set_path_internal, (const char* path), { history.replaceState({}, "", UTF8ToString(path)); })

void URLHistory::replaceState(std::string path) { set_path_internal(path.c_str()); }

#else

void URLHistory::replaceState(std::string /*path*/) {}

#endif

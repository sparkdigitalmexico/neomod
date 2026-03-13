// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#pragma once

#include <span>

using SCANCODE = unsigned short;
class ConVar;

namespace OsuKeyBinds {
struct Bind {
	ConVar *cvar;
	SCANCODE sc;
};

extern std::span<const Bind> getAll();
};

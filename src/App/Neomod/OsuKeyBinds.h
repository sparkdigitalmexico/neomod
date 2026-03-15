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

namespace keys {
extern ConVar BOSS_KEY;
extern ConVar DECREASE_LOCAL_OFFSET;
extern ConVar DECREASE_VOLUME;
extern ConVar DISABLE_MOUSE_BUTTONS;
extern ConVar FPOSU_ZOOM;
extern ConVar GAME_PAUSE;
extern ConVar INCREASE_LOCAL_OFFSET;
extern ConVar INCREASE_VOLUME;
extern ConVar INSTANT_REPLAY;
extern ConVar LEFT_CLICK;
extern ConVar LEFT_CLICK_2;
extern ConVar MOD_AUTO;
extern ConVar MOD_AUTOPILOT;
extern ConVar MOD_DOUBLETIME;
extern ConVar MOD_EASY;
extern ConVar MOD_FLASHLIGHT;
extern ConVar MOD_HALFTIME;
extern ConVar MOD_HARDROCK;
extern ConVar MOD_HIDDEN;
extern ConVar MOD_NOFAIL;
extern ConVar MOD_RELAX;
extern ConVar MOD_SCOREV2;
extern ConVar MOD_SPUNOUT;
extern ConVar MOD_SUDDENDEATH;
extern ConVar OPEN_SKIN_SELECT_MENU;
extern ConVar QUICK_LOAD;
extern ConVar QUICK_RETRY;
extern ConVar QUICK_SAVE;
extern ConVar RANDOM_BEATMAP;
extern ConVar RIGHT_CLICK;
extern ConVar RIGHT_CLICK_2;
extern ConVar SAVE_SCREENSHOT;
extern ConVar SEEK_TIME;
extern ConVar SEEK_TIME_BACKWARD;
extern ConVar SEEK_TIME_FORWARD;
extern ConVar SMOKE;
extern ConVar SKIP_CUTSCENE;
extern ConVar TOGGLE_CHAT;
extern ConVar TOGGLE_EXTENDED_CHAT;
extern ConVar TOGGLE_MAP_BACKGROUND;
extern ConVar TOGGLE_MODSELECT;
extern ConVar TOGGLE_SCOREBOARD;
}  // namespace key

};  // namespace OsuKeyBinds

namespace keys = OsuKeyBinds::keys;

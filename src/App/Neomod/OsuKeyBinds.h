// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#pragma once

#include <span>

using SCANCODE = unsigned short;
class ConVar;
class KeyboardEvent;

enum MC_Scancode : unsigned int;

namespace OsuKeyBinds {
struct Bind final {
    ConVar *cvar;

    [[nodiscard]] bool isDefault() const;
    [[nodiscard]] SCANCODE getDefault() const;
    [[nodiscard]] SCANCODE get() const;

    void reset();
    void set(SCANCODE bindKey);

    [[nodiscard]] bool operator==(const Bind &rhs) const;
    [[nodiscard]] bool operator!=(const Bind &rhs) const;

    [[nodiscard]] bool operator==(SCANCODE rhs) const;
    [[nodiscard]] bool operator!=(SCANCODE rhs) const;

    [[nodiscard]] inline bool operator==(MC_Scancode rhs) const { return operator==((SCANCODE)rhs); }
    [[nodiscard]] inline bool operator!=(MC_Scancode rhs) const { return operator!=((SCANCODE)rhs); }

    [[nodiscard]] inline bool operator==(int rhs) const { return operator==((SCANCODE)rhs); }
    [[nodiscard]] inline bool operator!=(int rhs) const { return operator!=((SCANCODE)rhs); }

    [[nodiscard]] bool operator==(const KeyboardEvent &rhs) const;
    [[nodiscard]] bool operator!=(const KeyboardEvent &rhs) const;

    operator SCANCODE() const;
};

extern std::span<Bind *> getAll();

namespace binds {
extern Bind BOSS_KEY;
extern Bind DECREASE_LOCAL_OFFSET;
extern Bind DECREASE_VOLUME;
extern Bind DISABLE_MOUSE_BUTTONS;
extern Bind FPOSU_ZOOM;
extern Bind GAME_PAUSE;
extern Bind INCREASE_LOCAL_OFFSET;
extern Bind INCREASE_VOLUME;
extern Bind INSTANT_REPLAY;
extern Bind LEFT_CLICK;
extern Bind LEFT_CLICK_2;
extern Bind MOD_AUTO;
extern Bind MOD_AUTOPILOT;
extern Bind MOD_DOUBLETIME;
extern Bind MOD_EASY;
extern Bind MOD_FLASHLIGHT;
extern Bind MOD_HALFTIME;
extern Bind MOD_HARDROCK;
extern Bind MOD_HIDDEN;
extern Bind MOD_NOFAIL;
extern Bind MOD_RELAX;
extern Bind MOD_SCOREV2;
extern Bind MOD_SPUNOUT;
extern Bind MOD_SUDDENDEATH;
extern Bind OPEN_SKIN_SELECT_MENU;
extern Bind QUICK_LOAD;
extern Bind QUICK_RETRY;
extern Bind QUICK_SAVE;
extern Bind RANDOM_BEATMAP;
extern Bind RIGHT_CLICK;
extern Bind RIGHT_CLICK_2;
extern Bind SAVE_SCREENSHOT;
extern Bind SEEK_TIME;
extern Bind SEEK_TIME_BACKWARD;
extern Bind SEEK_TIME_FORWARD;
extern Bind SMOKE;
extern Bind SKIP_CUTSCENE;
extern Bind TOGGLE_CHAT;
extern Bind TOGGLE_EXTENDED_CHAT;
extern Bind TOGGLE_MAP_BACKGROUND;
extern Bind TOGGLE_MODSELECT;
extern Bind TOGGLE_SCOREBOARD;
}  // namespace binds

};  // namespace OsuKeyBinds

namespace binds = OsuKeyBinds::binds;

// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#include "OsuKeyBinds.h"
#include "ConVar.h"
#include "KeyBindings.h"

#include <array>

namespace OsuKeyBinds {
namespace keys {
using namespace cv;

ConVar BOSS_KEY("key_boss", (SCANCODE)KEY_INSERT, CLIENT);
ConVar DECREASE_LOCAL_OFFSET("key_decrease_local_offset", (SCANCODE)KEY_MINUS, CLIENT);
ConVar DECREASE_VOLUME("key_decrease_volume", (SCANCODE)KEY_DOWN, CLIENT);
ConVar DISABLE_MOUSE_BUTTONS("key_disable_mouse_buttons", (SCANCODE)KEY_F10, CLIENT);
ConVar FPOSU_ZOOM("key_fposu_zoom", 0, CLIENT);
ConVar GAME_PAUSE("key_game_pause", (SCANCODE)KEY_ESCAPE, CLIENT);
ConVar INCREASE_LOCAL_OFFSET("key_increase_local_offset", (SCANCODE)KEY_EQUALS, CLIENT);
ConVar INCREASE_VOLUME("key_increase_volume", (SCANCODE)KEY_UP, CLIENT);
ConVar INSTANT_REPLAY("key_instant_replay", (SCANCODE)KEY_F2, CLIENT);
ConVar LEFT_CLICK("key_left_click", (SCANCODE)KEY_Z, CLIENT);
ConVar LEFT_CLICK_2("key_left_click_2", 0, CLIENT);
ConVar MOD_AUTO("key_mod_auto", (SCANCODE)KEY_V, CLIENT);
ConVar MOD_AUTOPILOT("key_mod_autopilot", (SCANCODE)KEY_X, CLIENT);
ConVar MOD_DOUBLETIME("key_mod_doubletime", (SCANCODE)KEY_D, CLIENT);
ConVar MOD_EASY("key_mod_easy", (SCANCODE)KEY_Q, CLIENT);
ConVar MOD_FLASHLIGHT("key_mod_flashlight", (SCANCODE)KEY_G, CLIENT);
ConVar MOD_HALFTIME("key_mod_halftime", (SCANCODE)KEY_E, CLIENT);
ConVar MOD_HARDROCK("key_mod_hardrock", (SCANCODE)KEY_A, CLIENT);
ConVar MOD_HIDDEN("key_mod_hidden", (SCANCODE)KEY_F, CLIENT);
ConVar MOD_NOFAIL("key_mod_nofail", (SCANCODE)KEY_W, CLIENT);
ConVar MOD_RELAX("key_mod_relax", (SCANCODE)KEY_Z, CLIENT);
ConVar MOD_SCOREV2("key_mod_scorev2", (SCANCODE)KEY_B, CLIENT);
ConVar MOD_SPUNOUT("key_mod_spunout", (SCANCODE)KEY_C, CLIENT);
ConVar MOD_SUDDENDEATH("key_mod_suddendeath", (SCANCODE)KEY_S, CLIENT);
ConVar OPEN_SKIN_SELECT_MENU("key_open_skin_select_menu", 0, CLIENT);
ConVar QUICK_LOAD("key_quick_load", (SCANCODE)KEY_F7, CLIENT);
ConVar QUICK_RETRY("key_quick_retry", (SCANCODE)KEY_BACKSPACE, CLIENT);
ConVar QUICK_SAVE("key_quick_save", (SCANCODE)KEY_F6, CLIENT);
ConVar RANDOM_BEATMAP("key_random_beatmap", (SCANCODE)KEY_F2, CLIENT);
ConVar RIGHT_CLICK("key_right_click", (SCANCODE)KEY_X, CLIENT);
ConVar RIGHT_CLICK_2("key_right_click_2", 0, CLIENT);
ConVar SAVE_SCREENSHOT("key_save_screenshot", (SCANCODE)KEY_F12, CLIENT);
ConVar SEEK_TIME("key_seek_time", (SCANCODE)KEY_RSHIFT, CLIENT);
ConVar SEEK_TIME_BACKWARD("key_seek_time_backward", (SCANCODE)KEY_LEFT, CLIENT);
ConVar SEEK_TIME_FORWARD("key_seek_time_forward", (SCANCODE)KEY_RIGHT, CLIENT);
ConVar SMOKE("key_smoke", 0, CLIENT);
ConVar SKIP_CUTSCENE("key_skip_cutscene", (SCANCODE)KEY_SPACE, CLIENT);
ConVar TOGGLE_CHAT("key_toggle_chat", (SCANCODE)KEY_F8, CLIENT);
ConVar TOGGLE_EXTENDED_CHAT("key_toggle_extended_chat", (SCANCODE)KEY_F9, CLIENT);
ConVar TOGGLE_MAP_BACKGROUND("key_toggle_map_background", 0, CLIENT);
ConVar TOGGLE_MODSELECT("key_toggle_modselect", (SCANCODE)KEY_F1, CLIENT);
ConVar TOGGLE_SCOREBOARD("key_toggle_scoreboard", (SCANCODE)KEY_TAB, CLIENT);
}  // namespace keys

std::span<const Bind> getAll() {
    static std::array allBindsList{
        Bind{&keys::LEFT_CLICK, keys::LEFT_CLICK.getDefaultVal<SCANCODE>()},
        Bind{&keys::LEFT_CLICK_2, keys::LEFT_CLICK_2.getDefaultVal<SCANCODE>()},
        Bind{&keys::RIGHT_CLICK, keys::RIGHT_CLICK.getDefaultVal<SCANCODE>()},
        Bind{&keys::RIGHT_CLICK_2, keys::RIGHT_CLICK_2.getDefaultVal<SCANCODE>()},
        Bind{&keys::SMOKE, keys::SMOKE.getDefaultVal<SCANCODE>()},
        Bind{&keys::BOSS_KEY, keys::BOSS_KEY.getDefaultVal<SCANCODE>()},
        Bind{&keys::DECREASE_LOCAL_OFFSET, keys::DECREASE_LOCAL_OFFSET.getDefaultVal<SCANCODE>()},
        Bind{&keys::DECREASE_VOLUME, keys::DECREASE_VOLUME.getDefaultVal<SCANCODE>()},
        Bind{&keys::DISABLE_MOUSE_BUTTONS, keys::DISABLE_MOUSE_BUTTONS.getDefaultVal<SCANCODE>()},
        Bind{&keys::FPOSU_ZOOM, keys::FPOSU_ZOOM.getDefaultVal<SCANCODE>()},
        Bind{&keys::GAME_PAUSE, keys::GAME_PAUSE.getDefaultVal<SCANCODE>()},
        Bind{&keys::INCREASE_LOCAL_OFFSET, keys::INCREASE_LOCAL_OFFSET.getDefaultVal<SCANCODE>()},
        Bind{&keys::INCREASE_VOLUME, keys::INCREASE_VOLUME.getDefaultVal<SCANCODE>()},
        Bind{&keys::INSTANT_REPLAY, keys::INSTANT_REPLAY.getDefaultVal<SCANCODE>()},
        Bind{&keys::OPEN_SKIN_SELECT_MENU, keys::OPEN_SKIN_SELECT_MENU.getDefaultVal<SCANCODE>()},
        Bind{&keys::QUICK_LOAD, keys::QUICK_LOAD.getDefaultVal<SCANCODE>()},
        Bind{&keys::QUICK_RETRY, keys::QUICK_RETRY.getDefaultVal<SCANCODE>()},
        Bind{&keys::QUICK_SAVE, keys::QUICK_SAVE.getDefaultVal<SCANCODE>()},
        Bind{&keys::RANDOM_BEATMAP, keys::RANDOM_BEATMAP.getDefaultVal<SCANCODE>()},
        Bind{&keys::SAVE_SCREENSHOT, keys::SAVE_SCREENSHOT.getDefaultVal<SCANCODE>()},
        Bind{&keys::SEEK_TIME, keys::SEEK_TIME.getDefaultVal<SCANCODE>()},
        Bind{&keys::SEEK_TIME_BACKWARD, keys::SEEK_TIME_BACKWARD.getDefaultVal<SCANCODE>()},
        Bind{&keys::SEEK_TIME_FORWARD, keys::SEEK_TIME_FORWARD.getDefaultVal<SCANCODE>()},
        Bind{&keys::SKIP_CUTSCENE, keys::SKIP_CUTSCENE.getDefaultVal<SCANCODE>()},
        Bind{&keys::TOGGLE_CHAT, keys::TOGGLE_CHAT.getDefaultVal<SCANCODE>()},
        Bind{&keys::TOGGLE_EXTENDED_CHAT, keys::TOGGLE_EXTENDED_CHAT.getDefaultVal<SCANCODE>()},
        Bind{&keys::TOGGLE_MAP_BACKGROUND, keys::TOGGLE_MAP_BACKGROUND.getDefaultVal<SCANCODE>()},
        Bind{&keys::TOGGLE_MODSELECT, keys::TOGGLE_MODSELECT.getDefaultVal<SCANCODE>()},
        Bind{&keys::TOGGLE_SCOREBOARD, keys::TOGGLE_SCOREBOARD.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_AUTO, keys::MOD_AUTO.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_AUTOPILOT, keys::MOD_AUTOPILOT.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_DOUBLETIME, keys::MOD_DOUBLETIME.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_EASY, keys::MOD_EASY.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_FLASHLIGHT, keys::MOD_FLASHLIGHT.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_HALFTIME, keys::MOD_HALFTIME.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_HARDROCK, keys::MOD_HARDROCK.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_HIDDEN, keys::MOD_HIDDEN.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_NOFAIL, keys::MOD_NOFAIL.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_RELAX, keys::MOD_RELAX.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_SCOREV2, keys::MOD_SCOREV2.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_SPUNOUT, keys::MOD_SPUNOUT.getDefaultVal<SCANCODE>()},
        Bind{&keys::MOD_SUDDENDEATH, keys::MOD_SUDDENDEATH.getDefaultVal<SCANCODE>()}};
    return allBindsList;
}

}  // namespace OsuKeyBinds

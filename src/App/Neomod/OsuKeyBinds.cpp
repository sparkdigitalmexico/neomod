// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#include "OsuKeyBinds.h"
#include "ConVar.h"
#include "KeyBindings.h"
#include "KeyboardEvent.h"

#include <array>

namespace OsuKeyBinds {
namespace cv {
namespace {
using namespace ::cv;
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
}  // namespace
}  // namespace cv

namespace binds {

Bind BOSS_KEY{&cv::BOSS_KEY};
Bind DECREASE_LOCAL_OFFSET{&cv::DECREASE_LOCAL_OFFSET};
Bind DECREASE_VOLUME{&cv::DECREASE_VOLUME};
Bind DISABLE_MOUSE_BUTTONS{&cv::DISABLE_MOUSE_BUTTONS};
Bind FPOSU_ZOOM{&cv::FPOSU_ZOOM};
Bind GAME_PAUSE{&cv::GAME_PAUSE};
Bind INCREASE_LOCAL_OFFSET{&cv::INCREASE_LOCAL_OFFSET};
Bind INCREASE_VOLUME{&cv::INCREASE_VOLUME};
Bind INSTANT_REPLAY{&cv::INSTANT_REPLAY};
Bind LEFT_CLICK{&cv::LEFT_CLICK};
Bind LEFT_CLICK_2{&cv::LEFT_CLICK_2};
Bind MOD_AUTO{&cv::MOD_AUTO};
Bind MOD_AUTOPILOT{&cv::MOD_AUTOPILOT};
Bind MOD_DOUBLETIME{&cv::MOD_DOUBLETIME};
Bind MOD_EASY{&cv::MOD_EASY};
Bind MOD_FLASHLIGHT{&cv::MOD_FLASHLIGHT};
Bind MOD_HALFTIME{&cv::MOD_HALFTIME};
Bind MOD_HARDROCK{&cv::MOD_HARDROCK};
Bind MOD_HIDDEN{&cv::MOD_HIDDEN};
Bind MOD_NOFAIL{&cv::MOD_NOFAIL};
Bind MOD_RELAX{&cv::MOD_RELAX};
Bind MOD_SCOREV2{&cv::MOD_SCOREV2};
Bind MOD_SPUNOUT{&cv::MOD_SPUNOUT};
Bind MOD_SUDDENDEATH{&cv::MOD_SUDDENDEATH};
Bind OPEN_SKIN_SELECT_MENU{&cv::OPEN_SKIN_SELECT_MENU};
Bind QUICK_LOAD{&cv::QUICK_LOAD};
Bind QUICK_RETRY{&cv::QUICK_RETRY};
Bind QUICK_SAVE{&cv::QUICK_SAVE};
Bind RANDOM_BEATMAP{&cv::RANDOM_BEATMAP};
Bind RIGHT_CLICK{&cv::RIGHT_CLICK};
Bind RIGHT_CLICK_2{&cv::RIGHT_CLICK_2};
Bind SAVE_SCREENSHOT{&cv::SAVE_SCREENSHOT};
Bind SEEK_TIME{&cv::SEEK_TIME};
Bind SEEK_TIME_BACKWARD{&cv::SEEK_TIME_BACKWARD};
Bind SEEK_TIME_FORWARD{&cv::SEEK_TIME_FORWARD};
Bind SMOKE{&cv::SMOKE};
Bind SKIP_CUTSCENE{&cv::SKIP_CUTSCENE};
Bind TOGGLE_CHAT{&cv::TOGGLE_CHAT};
Bind TOGGLE_EXTENDED_CHAT{&cv::TOGGLE_EXTENDED_CHAT};
Bind TOGGLE_MAP_BACKGROUND{&cv::TOGGLE_MAP_BACKGROUND};
Bind TOGGLE_MODSELECT{&cv::TOGGLE_MODSELECT};
Bind TOGGLE_SCOREBOARD{&cv::TOGGLE_SCOREBOARD};

}  // namespace binds

bool Bind::isDefault() const { return cvar->getDefaultVal<SCANCODE>() == cvar->getVal<SCANCODE>(); }
SCANCODE Bind::getDefault() const { return cvar->getDefaultVal<SCANCODE>(); }
SCANCODE Bind::get() const { return cvar->getVal<SCANCODE>(); }

void Bind::reset() { set(cvar->getDefaultVal<SCANCODE>()); }
void Bind::set(SCANCODE bindKey) { cvar->setValue(bindKey); }

bool Bind::operator==(const Bind &rhs) const { return get() == rhs.get(); }
bool Bind::operator!=(const Bind &rhs) const { return get() != rhs.get(); }

bool Bind::operator==(SCANCODE rhs) const { return get() == rhs; }
bool Bind::operator!=(SCANCODE rhs) const { return get() != rhs; }

Bind::operator SCANCODE() const { return get(); }

bool Bind::operator==(const KeyboardEvent &e) const { return get() == e.getScanCode(); }
bool Bind::operator!=(const KeyboardEvent &e) const { return get() != e.getScanCode(); }

std::span<Bind *> getAll() {
    static std::array allBindsList{&binds::LEFT_CLICK,
                                   &binds::LEFT_CLICK_2,
                                   &binds::RIGHT_CLICK,
                                   &binds::RIGHT_CLICK_2,
                                   &binds::SMOKE,
                                   &binds::BOSS_KEY,
                                   &binds::DECREASE_LOCAL_OFFSET,
                                   &binds::DECREASE_VOLUME,
                                   &binds::DISABLE_MOUSE_BUTTONS,
                                   &binds::FPOSU_ZOOM,
                                   &binds::GAME_PAUSE,
                                   &binds::INCREASE_LOCAL_OFFSET,
                                   &binds::INCREASE_VOLUME,
                                   &binds::INSTANT_REPLAY,
                                   &binds::OPEN_SKIN_SELECT_MENU,
                                   &binds::QUICK_LOAD,
                                   &binds::QUICK_RETRY,
                                   &binds::QUICK_SAVE,
                                   &binds::RANDOM_BEATMAP,
                                   &binds::SAVE_SCREENSHOT,
                                   &binds::SEEK_TIME,
                                   &binds::SEEK_TIME_BACKWARD,
                                   &binds::SEEK_TIME_FORWARD,
                                   &binds::SKIP_CUTSCENE,
                                   &binds::TOGGLE_CHAT,
                                   &binds::TOGGLE_EXTENDED_CHAT,
                                   &binds::TOGGLE_MAP_BACKGROUND,
                                   &binds::TOGGLE_MODSELECT,
                                   &binds::TOGGLE_SCOREBOARD,
                                   &binds::MOD_AUTO,
                                   &binds::MOD_AUTOPILOT,
                                   &binds::MOD_DOUBLETIME,
                                   &binds::MOD_EASY,
                                   &binds::MOD_FLASHLIGHT,
                                   &binds::MOD_HALFTIME,
                                   &binds::MOD_HARDROCK,
                                   &binds::MOD_HIDDEN,
                                   &binds::MOD_NOFAIL,
                                   &binds::MOD_RELAX,
                                   &binds::MOD_SCOREV2,
                                   &binds::MOD_SPUNOUT,
                                   &binds::MOD_SUDDENDEATH};
    return allBindsList;
}

}  // namespace OsuKeyBinds

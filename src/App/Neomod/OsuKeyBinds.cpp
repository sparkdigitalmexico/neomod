// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#include "OsuKeyBinds.h"
#include "OsuConVars.h"

#include <array>

namespace OsuKeyBinds {

std::span<const Bind> getAll() {
    static std::array allBindsList{
        Bind{&cv::LEFT_CLICK, cv::LEFT_CLICK.getDefaultVal<SCANCODE>()},
        Bind{&cv::LEFT_CLICK_2, cv::LEFT_CLICK_2.getDefaultVal<SCANCODE>()},
        Bind{&cv::RIGHT_CLICK, cv::RIGHT_CLICK.getDefaultVal<SCANCODE>()},
        Bind{&cv::RIGHT_CLICK_2, cv::RIGHT_CLICK_2.getDefaultVal<SCANCODE>()},
        Bind{&cv::SMOKE, cv::SMOKE.getDefaultVal<SCANCODE>()},
        Bind{&cv::BOSS_KEY, cv::BOSS_KEY.getDefaultVal<SCANCODE>()},
        Bind{&cv::DECREASE_LOCAL_OFFSET, cv::DECREASE_LOCAL_OFFSET.getDefaultVal<SCANCODE>()},
        Bind{&cv::DECREASE_VOLUME, cv::DECREASE_VOLUME.getDefaultVal<SCANCODE>()},
        Bind{&cv::DISABLE_MOUSE_BUTTONS, cv::DISABLE_MOUSE_BUTTONS.getDefaultVal<SCANCODE>()},
        Bind{&cv::FPOSU_ZOOM, cv::FPOSU_ZOOM.getDefaultVal<SCANCODE>()},
        Bind{&cv::GAME_PAUSE, cv::GAME_PAUSE.getDefaultVal<SCANCODE>()},
        Bind{&cv::INCREASE_LOCAL_OFFSET, cv::INCREASE_LOCAL_OFFSET.getDefaultVal<SCANCODE>()},
        Bind{&cv::INCREASE_VOLUME, cv::INCREASE_VOLUME.getDefaultVal<SCANCODE>()},
        Bind{&cv::INSTANT_REPLAY, cv::INSTANT_REPLAY.getDefaultVal<SCANCODE>()},
        Bind{&cv::OPEN_SKIN_SELECT_MENU, cv::OPEN_SKIN_SELECT_MENU.getDefaultVal<SCANCODE>()},
        Bind{&cv::QUICK_LOAD, cv::QUICK_LOAD.getDefaultVal<SCANCODE>()},
        Bind{&cv::QUICK_RETRY, cv::QUICK_RETRY.getDefaultVal<SCANCODE>()},
        Bind{&cv::QUICK_SAVE, cv::QUICK_SAVE.getDefaultVal<SCANCODE>()},
        Bind{&cv::RANDOM_BEATMAP, cv::RANDOM_BEATMAP.getDefaultVal<SCANCODE>()},
        Bind{&cv::SAVE_SCREENSHOT, cv::SAVE_SCREENSHOT.getDefaultVal<SCANCODE>()},
        Bind{&cv::SEEK_TIME, cv::SEEK_TIME.getDefaultVal<SCANCODE>()},
        Bind{&cv::SEEK_TIME_BACKWARD, cv::SEEK_TIME_BACKWARD.getDefaultVal<SCANCODE>()},
        Bind{&cv::SEEK_TIME_FORWARD, cv::SEEK_TIME_FORWARD.getDefaultVal<SCANCODE>()},
        Bind{&cv::SKIP_CUTSCENE, cv::SKIP_CUTSCENE.getDefaultVal<SCANCODE>()},
        Bind{&cv::TOGGLE_CHAT, cv::TOGGLE_CHAT.getDefaultVal<SCANCODE>()},
        Bind{&cv::TOGGLE_EXTENDED_CHAT, cv::TOGGLE_EXTENDED_CHAT.getDefaultVal<SCANCODE>()},
        Bind{&cv::TOGGLE_MAP_BACKGROUND, cv::TOGGLE_MAP_BACKGROUND.getDefaultVal<SCANCODE>()},
        Bind{&cv::TOGGLE_MODSELECT, cv::TOGGLE_MODSELECT.getDefaultVal<SCANCODE>()},
        Bind{&cv::TOGGLE_SCOREBOARD, cv::TOGGLE_SCOREBOARD.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_AUTO, cv::MOD_AUTO.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_AUTOPILOT, cv::MOD_AUTOPILOT.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_DOUBLETIME, cv::MOD_DOUBLETIME.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_EASY, cv::MOD_EASY.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_FLASHLIGHT, cv::MOD_FLASHLIGHT.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_HALFTIME, cv::MOD_HALFTIME.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_HARDROCK, cv::MOD_HARDROCK.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_HIDDEN, cv::MOD_HIDDEN.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_NOFAIL, cv::MOD_NOFAIL.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_RELAX, cv::MOD_RELAX.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_SCOREV2, cv::MOD_SCOREV2.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_SPUNOUT, cv::MOD_SPUNOUT.getDefaultVal<SCANCODE>()},
        Bind{&cv::MOD_SUDDENDEATH, cv::MOD_SUDDENDEATH.getDefaultVal<SCANCODE>()}};
    return allBindsList;
}

}  // namespace OsuKeyBinds

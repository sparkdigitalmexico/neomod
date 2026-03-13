// Copyright (c) 2016, PG, 2025, kiwec, 2025, WH, All rights reserved.
#include "OsuKeyBinds.h"
#include "OsuConVars.h"

#include <array>

namespace OsuKeyBinds {

std::span<ConVar*> getAll() {
    static std::array allBindsList{&cv::LEFT_CLICK,
                                   &cv::LEFT_CLICK_2,
                                   &cv::RIGHT_CLICK,
                                   &cv::RIGHT_CLICK_2,
                                   &cv::SMOKE,

                                   &cv::BOSS_KEY,
                                   &cv::DECREASE_LOCAL_OFFSET,
                                   &cv::DECREASE_VOLUME,
                                   &cv::DISABLE_MOUSE_BUTTONS,
                                   &cv::FPOSU_ZOOM,
                                   &cv::GAME_PAUSE,
                                   &cv::INCREASE_LOCAL_OFFSET,
                                   &cv::INCREASE_VOLUME,
                                   &cv::INSTANT_REPLAY,

                                   &cv::OPEN_SKIN_SELECT_MENU,
                                   &cv::QUICK_LOAD,
                                   &cv::QUICK_RETRY,
                                   &cv::QUICK_SAVE,
                                   &cv::RANDOM_BEATMAP,

                                   &cv::SAVE_SCREENSHOT,
                                   &cv::SEEK_TIME,
                                   &cv::SEEK_TIME_BACKWARD,
                                   &cv::SEEK_TIME_FORWARD,
                                   &cv::SKIP_CUTSCENE,

                                   &cv::TOGGLE_CHAT,
                                   &cv::TOGGLE_EXTENDED_CHAT,
                                   &cv::TOGGLE_MAP_BACKGROUND,
                                   &cv::TOGGLE_MODSELECT,
                                   &cv::TOGGLE_SCOREBOARD,

                                   &cv::MOD_AUTO,
                                   &cv::MOD_AUTOPILOT,
                                   &cv::MOD_DOUBLETIME,
                                   &cv::MOD_EASY,
                                   &cv::MOD_FLASHLIGHT,
                                   &cv::MOD_HALFTIME,
                                   &cv::MOD_HARDROCK,
                                   &cv::MOD_HIDDEN,
                                   &cv::MOD_NOFAIL,
                                   &cv::MOD_RELAX,
                                   &cv::MOD_SCOREV2,
                                   &cv::MOD_SPUNOUT,
                                   &cv::MOD_SUDDENDEATH};
    return allBindsList;
}

}  // namespace OsuKeyBinds

// Copyright (c) 2026, kiwec, All rights reserved.
#include "Wasm.h"

#include "BeatmapInterface.h"
#include "Osu.h"
#include "RankingScreen.h"
#include "score.h"
#include "UI.h"
#include "URLHistory.h"

void Wasm::update_url() {
    // We only want to add URLs that are fully handled by the wasm frontend.
    // As in, if the user opens a new tab and goes to that URL, we would
    // land in relatively the same state as we are currently in.

    // These are TODOs because they're not yet implemented by the frontend
    // TODO: spectating
    // TODO: multiplayer lobbies

    // Score screen
    if(ui->getRankingScreen()->isVisible()) {
        i64 score_id = ui->getRankingScreen()->getScore().bancho_score_id;
        if(score_id != 0) {
            URLHistory::replaceState("/scores/" + std::to_string(score_id));
            return;
        }
    }

    // Watching online replay
    if(osu->getMapInterface()->is_watching) {
        i64 score_id = osu->getMapInterface()->replay_data.bancho_score_id;
        if(score_id != 0) {
            URLHistory::replaceState("/scores/" + std::to_string(score_id));
            return;
        }
    }

    // Default/fallback
    if(osu->isBleedingEdge()) {
        URLHistory::replaceState("/online/bleedingedge/");
    } else {
        URLHistory::replaceState("/online/");
    }
}

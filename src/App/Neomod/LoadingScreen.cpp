#include "LoadingScreen.h"

#include "ConVar.h"
#include "Engine.h"
#include "Font.h"
#include "KeyBindings.h"
#include "Graphics.h"
#include "Osu.h"
#include "OsuConVarDefs.h"
#include "Skin.h"
#include "UI.h"

#include <utility>

void LoadingScreen::update(CBaseUIEventCtx& /*c*/) {
    if(!this->isVisible()) {
        return;
    }

    this->progress = this->updateProgress();
    if(this->isFinished()) {
        this->onFinished();
    }
}

void LoadingScreen::drawBackground() {
    // black background
    g->setColor(0xff000000);
    g->fillRect(0, 0, osu->getVirtScreenWidth(), osu->getVirtScreenHeight());
}

void LoadingScreen::drawProgress() {
    auto* font = osu->getSubTitleFont();
    const f32 shadowOffset = std::round(1.0f * Osu::getUIScale());

    // progress message
    const UString loadingMessage = fmt::format("Loading ... ({} %)", (int)(this->progress * 100.0f));
    g->pushTransform();
    {
        g->translate((int)(osu->getVirtScreenWidth() / 2 - font->getStringWidth(loadingMessage) / 2),
                     osu->getVirtScreenHeight() - 15);
        g->drawString(font, loadingMessage,
                      TextShadow{.col_text = rgb(255, 255, 255), .col_shadow = rgb(0, 0, 0), .offs_px = shadowOffset});
    }
    g->popTransform();
}

void LoadingScreen::drawLoadingSpinner() {
    // spinner
    const float scale = Osu::getImageScale(osu->getSkin()->i_beatmap_import_spinner, 100);
    g->pushTransform();
    {
        g->rotate(engine->getTime() * 180, 0, 0, 1);
        g->scale(scale, scale);
        g->translate(osu->getVirtScreenWidth() / 2, osu->getVirtScreenHeight() / 2);
        g->drawImage(osu->getSkin()->i_beatmap_import_spinner);
    }
    g->popTransform();
}

void LoadingScreen::onKeyDown(KeyboardEvent& e) {
    if(e.isConsumed()) return;

    if(e == KEY_ESCAPE || e == cv::GAME_PAUSE.getVal<SCANCODE>()) {
        e.consume();
        this->onFinished();
    }
}

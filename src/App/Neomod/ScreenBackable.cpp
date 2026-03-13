// Copyright (c) 2016, PG, All rights reserved.
#include "ScreenBackable.h"

#include "Engine.h"
#include "Logging.h"
#include "NotificationOverlay.h"
#include "OsuConVars.h"
#include "KeyBindings.h"
#include "Osu.h"
#include "Skin.h"
#include "SoundEngine.h"
#include "UI.h"
#include "UIBackButton.h"
#include "MakeDelegateWrapper.h"
#include "Mouse.h"

ScreenBackable::ScreenBackable() : UIScreen(), backButton(std::make_unique<UIBackButton>(-1.f, 0.f, 0.f, 0.f, "")) {
    this->backButton->setClickCallback(SA::MakeDelegate<&ScreenBackable::onBack>(this));
    this->updateLayout();

    if(Osu::isKioskMode()) {
        this->backable = false;
    }
}

ScreenBackable::~ScreenBackable() = default;

void ScreenBackable::draw() {
    if(!this->bVisible) return;
    UIScreen::draw();
    if(this->backable) this->backButton->draw();
}

void ScreenBackable::update(CBaseUIEventCtx &c) {
    if(!this->bVisible) return;
    if(this->backable) this->backButton->update(c);
    if(c.mouse_consumed()) return;
    UIScreen::update(c);
}

void ScreenBackable::onKeyDown(KeyboardEvent &e) {
    UIScreen::onKeyDown(e);
    if(!this->bVisible || e.isConsumed() || !this->backable) return;
    const bool isWaitingForKey = ui->getNotificationOverlay()->isWaitingForKey();
    debugLog("isWaitingForKey: {} e.key: {} game_pause: {} key_escape: {}", isWaitingForKey, e.getScanCode(),
             cv::GAME_PAUSE.getVal<SCANCODE>(), (SCANCODE)KEY_ESCAPE);
    if(e == KEY_ESCAPE || (e == cv::GAME_PAUSE.getVal<SCANCODE>() && !isWaitingForKey)) {
        soundEngine->play(osu->getSkin()->s_menu_back);
        this->onBack();
        e.consume();
        return;
    }
}

void ScreenBackable::updateLayout() {
    if(!this->backable) return;
    this->backButton->updateLayout();
    this->backButton->setPosY((float)osu->getVirtScreenHeight() - this->backButton->getSize().y);
}

void ScreenBackable::onResolutionChange(vec2 /*newResolution*/) { this->updateLayout(); }

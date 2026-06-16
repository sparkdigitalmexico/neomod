#pragma once
// Copyright (c) 2016, PG, All rights reserved.
#include "CBaseUIContainer.h"

#include <cstddef>
#include <cassert>

class KeyboardEvent;
struct UI;
class UIScreen : public CBaseUIContainer {
    NOCOPY_NOMOVE(UIScreen)
   public:
    UIScreen() { this->bVisible = false; }
    ~UIScreen() override = default;

    // stamps lastTickFrame for the ui_validate_ticks check; overrides must call through
    void tick() override;

    virtual void onResolutionChange(vec2 newResolution) { (void)newResolution; }

    [[nodiscard]] u64 getLastTickFrame() const { return this->lastTickFrame; }

    // declared layer-stack flags, constructor-set; the stack reads these instead
    // of screens sniffing each other:
    // - modal: while visible, the input/key walk stops below this layer
    // - closeOnScreenSwitch: hidden on every base-screen swap (UI::setScreen/hide)
    [[nodiscard]] bool isModal() const { return this->bModal; }
    [[nodiscard]] bool closesOnScreenSwitch() const { return this->bCloseOnScreenSwitch; }

    // a visible screen returning true reserves the bare arrow keys for its own navigation,
    // blocking VolumeOverlay's arrow-bound volume binds (see VolumeOverlay::canChangeVolume)
    [[nodiscard]] virtual bool claimsArrowKeys() { return false; }

   protected:
    bool bModal{false};
    bool bCloseOnScreenSwitch{false};

   private:
    u64 lastTickFrame{0};
};

class UIOverlay : public UIScreen {
    NOCOPY_NOMOVE(UIOverlay)
   private:
    UIScreen *parent{nullptr};

   public:
    UIOverlay() : UIScreen() {}
    UIOverlay(UIScreen *parent);
    ~UIOverlay() override = default;

    UIScreen *getParent();

    // setParent(nullptr) will make getParent return the current active screen
    void setParent(UIScreen *parent);
};

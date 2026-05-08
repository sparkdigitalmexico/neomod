#pragma once
// Copyright (c) 2016, PG, All rights reserved.
#include "CBaseUISlider.h"

class ConVar;

class UISlider final : public CBaseUISlider {
   public:
    UISlider(float xPos, float yPos, float xSize, float ySize, std::string name);

    bool isAvailable() const;
    void draw() override;
    void update(CBaseUIEventCtx& c) override;

    // options menu hack
    UISlider* setUpdateRelPosOnChange(bool doReset) {
        this->bNeedsRelativePositioningUpdateOnChange = doReset;
        return this;
    }
    [[nodiscard]] inline bool getUpdateRelPosOnChange() const { return this->bNeedsRelativePositioningUpdateOnChange; }
    ConVar* cvar = nullptr;

   private:
    bool bNeedsRelativePositioningUpdateOnChange{false};
};

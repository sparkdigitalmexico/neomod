#pragma once
// Copyright (c) 2016, PG, All rights reserved.
#include "CBaseUICheckbox.h"

class ConVar;

class UICheckbox final : public CBaseUICheckbox {
   public:
    UICheckbox(float xPos, float yPos, float xSize, float ySize, std::string name, std::string text);

    bool isAvailable() const;
    void update(CBaseUIEventCtx &c) override;

    void setTooltipText(std::string_view text);

    // HACKHACK (lazy)
    ConVar *cvar = nullptr;

   protected:
    void onPressed() override;

   private:
    void onFocusStolen() override;

    std::vector<std::string> tooltipTextLines;

    bool bFocusStolenDelay;
};

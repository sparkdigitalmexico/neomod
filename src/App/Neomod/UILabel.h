#pragma once
// Copyright (c) 2025, kiwec, 2026, WH, All rights reserved.
#include "CBaseUILabel.h"

class UILabel final : public CBaseUILabel {
   public:
    UILabel(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::string name = {},
            std::string text = {})
        : CBaseUILabel(xPos, yPos, xSize, ySize, std::move(name), std::move(text)) {}

    void update(CBaseUIEventCtx &c) override;
    void setTooltipText(std::string_view text);

    // debugging
    [[nodiscard]] std::string getTooltipText() const;

   private:
    void onFocusStolen() override;

    std::vector<std::string> tooltipTextLines;

    bool bFocusStolenDelay{false};
};

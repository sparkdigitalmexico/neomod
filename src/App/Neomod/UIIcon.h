#pragma once
// Copyright (c) 2025, kiwec, All rights reserved.
#include "CBaseUILabel.h"

class UIIcon : public CBaseUILabel {
   public:
    UIIcon(char32_t icon);

    void update(CBaseUIEventCtx &c) override;
    void setTooltipText(std::string_view text);

    // debugging
    [[nodiscard]] std::string getTooltipText() const;

   private:
    void onFocusStolen() override;

    std::vector<std::string> tooltipTextLines;

    bool bFocusStolenDelay{false};
};

// Copyright (c) 2026, WH, All rights reserved.
#pragma once
#include "CBaseUIButton.h"

class UIButtonRounded : public CBaseUIButton {
    NOCOPY_NOMOVE(UIButtonRounded)
   public:
    UIButtonRounded(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::string name = {},
                    std::string text = {}, int cornerRadius = 6);
    ~UIButtonRounded() override;
    UIButtonRounded* setCornerRadius(int radius);
    [[nodiscard]] inline int getCornerRadius() const { return this->cornerRadius; }

   protected:
    void drawBackground() override;
    void drawFrame() override;
    void drawHoverRect(int hoverRectOffset, bool isClickHeld) override;

    // based on font dpi (more rounded for higher dpi)
    [[nodiscard]] int getRealCornerRadius() const;

   private:
    int cornerRadius{6};
};

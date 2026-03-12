// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "CBaseUIContainer.h"

#include <span>
#include <string_view>
#include <string>

class McFont;

namespace neomod::mainmenu {

class WrappedText final : public CBaseUIContainer {
    NOCOPY_NOMOVE(WrappedText)
   public:
    WrappedText(std::nullptr_t, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0) = delete;
    WrappedText(McFont *font, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0);
    ~WrappedText() override;
    WrappedText *setFont(McFont *font);

    WrappedText *setText(std::string_view text);

    void setVisibleCallback(float visible) { (void)this->setVisible(!!static_cast<int>(visible)); }

   private:
    McFont *font;
    std::string_view lastText;
    float lastWrapWidth{0.f};
};

std::span<const std::string_view> getAllTips();
std::string_view getCurrentTip();
std::string_view getNextTip();

}  // namespace neomod::mainmenu

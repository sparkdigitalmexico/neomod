// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "CBaseUIContainer.h"

#include <span>
#include <string_view>
#include <string>
#include <functional>

class McFont;

namespace neomod::mainmenu {

class WrappedText final : public CBaseUIContainer {
    NOCOPY_NOMOVE(WrappedText)
   public:
    WrappedText(std::nullptr_t, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0) = delete;
    WrappedText(McFont *font, float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0);
    ~WrappedText() override;
    WrappedText *setFont(McFont *font);

    WrappedText *setText(const std::string &text);
    WrappedText *setOnMouseUpInsideCallback(std::function<void(bool, bool)> cb);

    void onMouseUpInside(bool left = true, bool right = false) override;

    void setVisibleCallback(float visible) { (void)this->setVisible(!!static_cast<int>(visible)); }

   private:
    McFont *font;
    std::string lastText;
    float lastWrapWidth{0.f};
    std::function<void(bool, bool)> onMouseUpCB{nullptr};
};

std::string getCurrentTip();

void cycleTip(int addIndex);
void cycleToPreviousTip();
void cycleToNextTip();

}  // namespace neomod::mainmenu

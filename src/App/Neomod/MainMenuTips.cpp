// Copyright (c) 2026, WH, All rights reserved.
#include "BaseEnvironment.h"
#include "MainMenuTips.h"
#include "CBaseUILabel.h"
#include "ConVar.h"
#include "Font.h"

#include <array>

namespace cv {
extern ConVar main_menu_last_tip_index;
}

namespace neomod::mainmenu {
namespace {
using std::string_view_literals::operator""sv;

#define TIPLEFT_ "Tip: "
#define TIPRIGHT_ ""sv,

#define TIP_(s)              \
    TIPLEFT_ MC_STRINGIZE(s) \
    TIPRIGHT_

// clang-format off
// NOTE: must have at least 1 tip in here even if it's empty
constexpr std::array s_tips{
    TIP_(Press Alt + Enter to toggle Fullscreen.)
    TIP_(Press Shift + F1 to open the in-game console.)
#if defined(MCENGINE_FEATURE_BASS) && defined(MCENGINE_FEATURE_SOLOUD)
#ifdef MCENGINE_PLATFORM_WINDOWS
	TIP_(Launch with the neomod-BASS shortcut to use BASS (ASIO/Exclusive mode) for audio.)
#else
	TIP_(Launch with "-sound bass" as a commandline argument to use BASS for audio.)
#endif
#endif
#ifdef MCENGINE_FEATURE_SDLGPU
#ifdef MCENGINE_PLATFORM_WINDOWS
    TIP_(Put "-gpu" after the "Target:" field in a shortcut to neomod to use the D3D12 renderer.)
    TIP_(Put "-gpu vk" after the "Target:" field in a shortcut to neomod to use the Vulkan renderer.)
#else
    TIP_(Launch with "-sdlgpu" as a commandline argument to use the Vulkan renderer.)
#endif
#endif
#ifdef MCENGINE_FEATURE_DIRECTX11
#ifdef MCENGINE_PLATFORM_WINDOWS
    TIP_(Put "-dx11" after the "Target:" field in a shortcut to neomod to use the D3D11 renderer.)
#else
    TIP_(Launch with "-dx11" as a commandline argument to use the D3D11 renderer.)
#endif
#endif
        // clang-format on
};

int s_currentIdx{-1};

}  // namespace

WrappedText::WrappedText(McFont *font, float xPos, float yPos, float xSize, float ySize)
    : CBaseUIContainer(xPos, yPos, xSize, ySize), font(font) {}
WrappedText::~WrappedText() = default;

WrappedText *WrappedText::setFont(McFont *font) {
    this->font = font;
    return this;
}

WrappedText *WrappedText::setText(std::string_view text) {
    const float containerWidth = this->getSize().x;
    if(text == this->lastText && containerWidth == this->lastWrapWidth) return this;
    this->lastText = text;
    this->lastWrapWidth = containerWidth;
    this->freeElements();
    const float yPad = std::round((float)this->font->getDPI() / 96.f) + 5.f;
    const float lineHeight = this->font->getHeight() + yPad;
    const auto lines = this->font->wrap(text, containerWidth);
    float yCounter = 0.f;
    for(const auto &line : lines) {
        auto *label = new CBaseUILabel(0, yCounter, containerWidth, lineHeight, "", line);
        label->setTextColor(rgb(200, 200, 200))
            ->setShadowColor(rgb(50, 50, 50))
            ->setTextJustification(TEXT_JUSTIFICATION::CENTERED)
            ->setDrawTextShadow(true)
            ->setDrawBackground(false)
            ->setDrawFrame(false);
        yCounter += lineHeight;
        CBaseUIContainer::addBaseUIElement(label);
    }
    this->setSizeY(yCounter);
    return this;
}

std::span<const std::string_view> getAllTips() { return s_tips; }

std::string_view getCurrentTip() {
    if(s_currentIdx == -1) return getNextTip();
    return s_tips[s_currentIdx];
}

std::string_view getNextTip() {
    static_assert(s_tips.size() > 0);
    const int lastTip = cv::main_menu_last_tip_index.getInt();
    s_currentIdx = (lastTip + 1) % static_cast<int>(s_tips.size());
    cv::main_menu_last_tip_index.setValue(s_currentIdx);
    return s_tips[s_currentIdx];
}

}  // namespace neomod::mainmenu

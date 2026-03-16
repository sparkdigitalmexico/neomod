// Copyright (c) 2026, WH, All rights reserved.
#include "BaseEnvironment.h"
#include "MainMenuTips.h"
#include "CBaseUILabel.h"
#include "ConVar.h"
#include "Environment.h"
#include "OsuKeyBinds.h"
#include "Font.h"

#include "fmt/format.h"

#include <array>
#include <memory>

namespace cv {
extern ConVar main_menu_last_tip_index;
}

namespace neomod::mainmenu {
namespace {
// using std::string_literals::operator""s;
// using std::string_view_literals::operator""sv;

// allow dynamic and static tips
using DynamicTipFunc = std::function<std::string()>;
struct Tip {
    Tip(std::string_view str) : staticString(str) {}
    Tip(DynamicTipFunc tipReturningFunc) : dynamicTipFunc(new DynamicTipFunc{std::move(tipReturningFunc)}) {}

    [[nodiscard]] std::string get() const { return dynamicTipFunc ? (*dynamicTipFunc)() : std::string{staticString}; }

   private:
    std::string_view staticString;
    std::unique_ptr<DynamicTipFunc> dynamicTipFunc{nullptr};
};

std::span<Tip> getAllTips() {
    // clang-format off
// NOTE: must have at least 1 non-empty tip in here
static std::array s_tips{
    Tip{R"(Press Alt + Enter to toggle Fullscreen.)"},
    Tip{R"(Press Shift + F1 to open the in-game console.)"},
    Tip{R"(Shift + Right Click on a skin in the dropdown list to change the source for base skin elements.)"},
    Tip{
        [&b = binds::TOGGLE_MODSELECT]() -> std::string
        { return fmt::format(R"(Press {:s} during gameplay to change mods in realtime.)", env->scanCodeToString(b.get())); }
    },
#if defined(MCENGINE_FEATURE_BASS) && defined(MCENGINE_FEATURE_SOLOUD)
#ifdef MCENGINE_PLATFORM_WINDOWS
	Tip{R"(Launch with the neomod-BASS shortcut to use BASS (ASIO/Exclusive mode) for audio.)"},
#else
	Tip{R"(Launch with "-sound bass" as a commandline argument to use BASS for audio.)"},
#endif
#endif
#ifdef MCENGINE_FEATURE_SDLGPU
#ifdef MCENGINE_PLATFORM_WINDOWS
    Tip{R"(Put "-gpu" after the "Target:" field in a shortcut to neomod to use the D3D12 renderer.)"},
    Tip{R"(Put "-gpu vk" after the "Target:" field in a shortcut to neomod to use the Vulkan renderer.)"},
#else
    Tip{R"(Launch with "-sdlgpu" as a commandline argument to use the Vulkan renderer.)"},
#endif
#endif
#ifdef MCENGINE_FEATURE_DIRECTX11
#ifdef MCENGINE_PLATFORM_WINDOWS
    Tip{R"(Put "-dx11" after the "Target:" field in a shortcut to neomod to use the D3D11 renderer.)"},
#else
    Tip{R"(Launch with "-dx11" as a commandline argument to use the D3D11 renderer.)"},
#endif
#endif
        // clang-format on
    };

    return s_tips;
}

int s_currentIdx{-1};

}  // namespace

WrappedText::WrappedText(McFont *font, float xPos, float yPos, float xSize, float ySize)
    : CBaseUIContainer(xPos, yPos, xSize, ySize), font(font) {}
WrappedText::~WrappedText() = default;

WrappedText *WrappedText::setFont(McFont *font) {
    this->font = font;
    return this;
}

WrappedText *WrappedText::setText(const std::string &text) {
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

std::string getCurrentTip() {
    std::string current;
    if(s_currentIdx == -1 || (current = getAllTips()[s_currentIdx].get()).empty()) {
        cycleToNextTip();
    }
    current = getAllTips()[s_currentIdx].get();
    assert(!current.empty());
    return fmt::format("Tip: {:s}", current);
}

void cycleToNextTip() {
    assert(getAllTips().size() > 0);
    bool foundNonEmpty = false;
    for(int iteratedOver = 0; !foundNonEmpty && iteratedOver < getAllTips().size(); ++iteratedOver) {
        const int lastTip = cv::main_menu_last_tip_index.getInt();
        s_currentIdx = (lastTip + 1) % static_cast<int>(getAllTips().size());
        cv::main_menu_last_tip_index.setValue(s_currentIdx);
        foundNonEmpty = !getAllTips()[s_currentIdx].get().empty();
    }
    return;
}

}  // namespace neomod::mainmenu

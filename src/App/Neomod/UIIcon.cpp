// Copyright (c) 2025, kiwec, All rights reserved.
#include "UIIcon.h"

#include "Osu.h"
#include "SString.h"
#include "TooltipOverlay.h"
#include "UI.h"
#include "UniString.h"

UIIcon::UIIcon(char32_t icon)
    : CBaseUILabel(0.f, 0.f, 0.f, 0.f, "", UniString::to_utf8(std::u32string_view{&icon, 1})) {
    this->setFont(osu->getFontIcons());
    this->setDrawBackground(false);
    this->setDrawFrame(false);
}

void UIIcon::update(CBaseUIEventCtx& c) {
    if(!this->bVisible) return;
    CBaseUILabel::update(c);

    if(this->isMouseInside() && this->tooltipTextLines.size() > 0 && !this->bFocusStolenDelay) {
        ui->getTooltipOverlay()->begin();
        for(const auto& tooltipTextLine : this->tooltipTextLines) {
            ui->getTooltipOverlay()->addLine(tooltipTextLine);
        }
        ui->getTooltipOverlay()->end();
    }

    this->bFocusStolenDelay = false;
}

void UIIcon::onFocusStolen() {
    CBaseUILabel::onFocusStolen();
    this->bMouseInside = false;
    this->bFocusStolenDelay = true;
}

void UIIcon::setTooltipText(std::string_view text) {
    this->tooltipTextLines = SString::split_newlines<std::string>(text);
}

std::string UIIcon::getTooltipText() const { return SString::join(this->tooltipTextLines, '\n'); }
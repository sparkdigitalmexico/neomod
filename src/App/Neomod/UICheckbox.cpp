// Copyright (c) 2016, PG, All rights reserved.
#include "UICheckbox.h"

#include <utility>

#include "ConVar.h"
#include "Engine.h"
#include "SString.h"
#include "TooltipOverlay.h"
#include "UI.h"

UICheckbox::UICheckbox(float xPos, float yPos, float xSize, float ySize, std::string name, std::string text)
    : CBaseUICheckbox(xPos, yPos, xSize, ySize, std::move(name), std::move(text)) {
    this->bFocusStolenDelay = false;
}

bool UICheckbox::isAvailable() const { return !this->cvar || this->cvar->getMaster() == CvarEditor::CLIENT; }

void UICheckbox::update(CBaseUIEventCtx& c) {
    if(!this->bVisible) return;
    CBaseUICheckbox::update(c);

    if(this->isMouseInside() && !this->bFocusStolenDelay) {
        auto lines = this->tooltipTextLines;
        if(this->cvar) {
            switch(this->cvar->getMaster()) {
                case CvarEditor::CLIENT:
                    break;
                case CvarEditor::SERVER:
                    lines = {"This setting is forced by the server."};
                    break;
                case CvarEditor::SKIN:
                    lines = {"This setting is forced by the current skin."};
                    break;
            }
        }

        if(lines.size() > 0) {
            auto* ttoverlay = ui->getTooltipOverlay();
            ttoverlay->begin();
            for(const auto& line : lines) {
                ttoverlay->addLine(line);
            }
            ttoverlay->end();
        }
    }

    this->bFocusStolenDelay = false;
}

void UICheckbox::onFocusStolen() {
    CBaseUICheckbox::onFocusStolen();

    this->bMouseInside = false;
    this->bFocusStolenDelay = true;
}

void UICheckbox::setTooltipText(std::string_view text) {
    this->tooltipTextLines = SString::split_newlines<std::string>(text);
}

void UICheckbox::onPressed() {
    if(this->isAvailable()) CBaseUICheckbox::onPressed();
}

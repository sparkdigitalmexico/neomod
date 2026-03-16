// Copyright (c) 2026, WH, All rights reserved.
#include "UIButtonRounded.h"
#include "Font.h"
#include "Graphics.h"

UIButtonRounded::UIButtonRounded(float xPos, float yPos, float xSize, float ySize, std::string name, std::string text,
                                 int cornerRadius)
    : CBaseUIButton(xPos, yPos, xSize, ySize, std::move(name), std::move(text)), cornerRadius(cornerRadius) {}
UIButtonRounded::~UIButtonRounded() = default;

UIButtonRounded* UIButtonRounded::setCornerRadius(int radius) {
    this->cornerRadius = radius;
    return this;
}

void UIButtonRounded::drawBackground() {
    g->setColor(this->backgroundColor);
    g->fillRoundedRect((int)this->getPos().x + 1, (int)this->getPos().y + 1, (int)this->getSize().x - 1,
                       (int)this->getSize().y - 1, this->getRealCornerRadius() - 1);
}

void UIButtonRounded::drawFrame() {
    g->setColor(this->frameColor);
    g->drawRoundedRect(this->getPos(), this->getSize(), this->getRealCornerRadius());
}

void UIButtonRounded::drawHoverRect(int hoverRectOffset) {
    // small fudge (square hover rect distance is a bit too much)
    if(hoverRectOffset > 1) {
        hoverRectOffset = (int)std::ceil((f32)hoverRectOffset / 1.5f);
    }
    g->drawRoundedRect((int)this->getPos().x - hoverRectOffset, (int)this->getPos().y - hoverRectOffset,
                       (int)this->getSize().x + hoverRectOffset * 2, (int)this->getSize().y + hoverRectOffset * 2,
                       this->getRealCornerRadius() + hoverRectOffset);
}

// based on font dpi (more rounded for higher dpi)
int UIButtonRounded::getRealCornerRadius() const {
    if(!this->font) return this->cornerRadius;
    return (int)std::round((f32)this->cornerRadius * ((f32)this->font->getDPI() / 96.f));
}

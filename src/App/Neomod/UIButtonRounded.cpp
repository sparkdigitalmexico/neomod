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

void UIButtonRounded::drawHoverRect(int hoverRectOffset, bool isClickHeld) {
    // rounded buttons need about half the distance of square hover rects
    float distance = std::round((float)hoverRectOffset / 2.f);
    float thickness = 1.f;

    if(isClickHeld && distance > 1.f) {
        // thick outline close to the button for a "pressed" look;
        // band spans from ~button edge (distance - thickness/2) to just past hover distance
        distance = std::ceil(distance / 2.f);
        thickness = distance * 2.f + 1.f;
    }

    g->drawRectf(Graphics::RectOptions{
        .x = (float)(int)this->getPos().x - distance + 0.5f,
        .y = (float)(int)this->getPos().y - distance + 0.5f,
        .width = (float)(int)this->getSize().x + distance * 2.f,
        .height = (float)(int)this->getSize().y + distance * 2.f,
        .lineThickness = thickness,
        .cornerRadius = (float)this->getRealCornerRadius() + distance,
    });
}

// based on font dpi (more rounded for higher dpi)
int UIButtonRounded::getRealCornerRadius() const {
    if(!this->font) return this->cornerRadius;
    return (int)std::round((f32)this->cornerRadius * ((f32)this->font->getDPI() / 96.f));
}

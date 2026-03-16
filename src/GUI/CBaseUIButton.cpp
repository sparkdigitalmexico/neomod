// Copyright (c) 2013, PG, All rights reserved.
#include "CBaseUIButton.h"

#include <utility>

#include "Engine.h"
#include "Graphics.h"
#include "Mouse.h"
#include "ResourceManager.h"
#include "Font.h"

CBaseUIButton::CBaseUIButton(std::nullptr_t notext, f32 xPos, f32 yPos, f32 xSize, f32 ySize)
    : CBaseUIElement(xPos, yPos, xSize, ySize, notext) {
    // this->setGrabClicks(true); // shouldn't this be set?

    this->font = engine->getDefaultFont();
}

CBaseUIButton::CBaseUIButton(f32 xPos, f32 yPos, f32 xSize, f32 ySize, std::string name, std::string text)
    : CBaseUIElement(xPos, yPos, xSize, ySize, std::move(name)) {
    this->font = engine->getDefaultFont();

    if(!text.empty()) {
        this->setText(std::move(text));
    }
}

void CBaseUIButton::draw() {
    if(!this->isVisible() || !this->isVisibleOnScreen()) return;

    // draw background
    if(this->bDrawBackground) {
        this->drawBackground();
    }

    // draw frame
    if(this->bDrawFrame) {
        this->drawFrame();
    }

    // draw hover rects
    const int hoverRectOffset = (int)std::round(3.f * ((f32)this->font->getDPI() / 96.f));  // NOTE: abusing font dpi
    g->setColor(this->frameColor);
    if(this->bEnabled && this->isMouseInside() && (this->bActive || (!mouse->isLeftDown()))) {
        this->drawHoverRect(hoverRectOffset);
    }
    if(this->bActive && this->bEnabled) {
        this->drawHoverRect(hoverRectOffset * 2);
    }

    // draw text
    this->drawText();
}

void CBaseUIButton::drawBackground() {
    g->setColor(this->backgroundColor);
    g->fillRect((int)this->getPos().x + 1, (int)this->getPos().y + 1, (int)this->getSize().x - 1,
                (int)this->getSize().y - 1);
}

void CBaseUIButton::drawFrame() {
    g->setColor(this->frameColor);
    g->drawRect(this->getPos(), this->getSize());
}

void CBaseUIButton::drawText() {
    if(this->font == nullptr || !this->isVisible() || !this->isVisibleOnScreen() || this->getText().length() < 1)
        return;

    // NOTE: don't decrease the size of this clip rect, console box suggestion text gets cut off at the bottom
    g->pushClipRect(McRect(this->getPos().x + 1, this->getPos().y + 1, this->getSize().x - 1, this->getSize().y));
    {
        g->pushTransform();
        {
            // Justification logic pasted from CBaseUILabel::drawText()
            f32 xPosAdd = 0;
            switch(this->textJustification) {
                case TEXT_JUSTIFICATION::LEFT:
                    break;
                case TEXT_JUSTIFICATION::CENTERED:
                    xPosAdd = this->getSize().x / 2.0f - this->fStringWidth / 2.0f;
                    break;
                case TEXT_JUSTIFICATION::RIGHT:
                    xPosAdd = this->getSize().x - this->fStringWidth;
                    break;
            }

            g->translate((i32)(this->getPos().x + xPosAdd),
                         (i32)(this->getPos().y + this->getSize().y / 2.f + this->fStringHeight / 2.f));

            const Color actualTextColor{this->textBrightColor ? this->textBrightColor : this->textColor};
            if(this->bDrawShadow) {
                const f32 shadowOffset = std::round((f32)this->font->getDPI() / 96.f);  // NOTE: abusing font dpi
                const Color shadowColor{this->textDarkColor ? this->textDarkColor : Colors::invert(this->textColor)};
                g->drawString(
                    this->font, this->getText(),
                    TextShadow{.col_text = actualTextColor, .col_shadow = shadowColor, .offs_px = shadowOffset});
            } else {
                g->setColor(actualTextColor);
                g->drawString(this->font, this->getText());
            }
        }
        g->popTransform();
    }
    g->popClipRect();
}

void CBaseUIButton::drawHoverRect(int distance) {
    const ivec2 pos{this->getPos()};
    const ivec2 size{this->getSize()};
    g->drawLine(pos.x, pos.y - distance, pos.x + size.x + 1, pos.y - distance);
    g->drawLine(pos.x, pos.y + size.y + distance, pos.x + size.x + 1, pos.y + size.y + distance);
    g->drawLine(pos.x - distance, pos.y, pos.x - distance, pos.y + size.y + 1);
    g->drawLine(pos.x + size.x + distance, pos.y, pos.x + size.x + distance, pos.y + size.y + 1);
}

void CBaseUIButton::onMouseUpInside(bool left, bool right) { this->onClicked(left, right); }

void CBaseUIButton::onClicked(bool left, bool right) {
    if(this->clickCallback && (*this->clickCallback)) {
        (*this->clickCallback)(this, left, right);
    }
}

void CBaseUIButton::updateStringMetrics() {
    if(this->font == nullptr) return;

    this->fStringHeight = this->font->getHeight();
    this->fStringWidth = this->font->getStringWidth(this->getText());
}

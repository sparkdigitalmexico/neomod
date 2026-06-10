// Copyright (c) 2013, PG, All rights reserved.
#include "CBaseUIElement.h"

#include "Engine.h"
#include "Logging.h"
#include "ConVar.h"
#include "Mouse.h"

#include <utility>
#include <memory>
#include <typeinfo>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace CBaseUIDebug {
namespace {
bool dumpElems{false};
int traceLvl{0};
}  // namespace
void onDumpElemsChangeCallback(float newvalue) { dumpElems = !!static_cast<int>(newvalue); }
void onTraceChangeCallback(float newvalue) { traceLvl = static_cast<int>(newvalue); }

int traceLevel() { return traceLvl; }

void traceEvent(const CBaseUIElement *elem, std::string_view evt) {
    logRaw("uitrace frame={} evt={} elem={}", engine->getFrameCount(), evt, elemName(elem));
}

std::string elemName(const CBaseUIElement *elem) {
    if(!elem) return "<null>";
    if(!elem->getName().empty()) return std::string{elem->getName()};

    const char *mangled = typeid(*elem).name();
#if __has_include(<cxxabi.h>)
    int status = 0;
    char *demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    std::string ret{(status == 0 && demangled) ? demangled : mangled};
    free(demangled);
    return ret;
#else
    return std::string{mangled};
#endif
}
}  // namespace CBaseUIDebug

void CBaseUIEventCtx::consume_mouse() { this->propagate_clicks = this->propagate_hover = false; }

bool CBaseUIEventCtx::mouse_consumed() const {
    return this->propagate_clicks == this->propagate_hover && (this->propagate_hover == false);
}

CBaseUIElement::CBaseUIElement(float xPos, float yPos, float xSize, float ySize, std::nullptr_t /**/)
    : rect(xPos, yPos, xSize, ySize), relRect(this->rect) {}

CBaseUIElement::CBaseUIElement(float xPos, float yPos, float xSize, float ySize, std::string name)
    : sName(std::move(name)), rect(xPos, yPos, xSize, ySize), relRect(this->rect) {}
CBaseUIElement::~CBaseUIElement() = default;

// keyboard input
void CBaseUIElement::onKeyUp(KeyboardEvent &e) { (void)e; }
void CBaseUIElement::onKeyDown(KeyboardEvent &e) { (void)e; }
void CBaseUIElement::onChar(KeyboardEvent &e) { (void)e; }

// getters
std::string_view CBaseUIElement::getName() const { return this->sName; }

const McRect &CBaseUIElement::getRect() const { return this->rect; }

const vec2 &CBaseUIElement::getPos() const { return this->rect.getPos(); }
const vec2 &CBaseUIElement::getSize() const { return this->rect.getSize(); }

const McRect &CBaseUIElement::getRelRect() const { return this->relRect; }

const vec2 &CBaseUIElement::getRelPos() const { return this->relRect.getPos(); }
const vec2 &CBaseUIElement::getRelSize() const { return this->relRect.getSize(); }

bool CBaseUIElement::isActive() { return this->bActive || this->isBusy(); }
bool CBaseUIElement::isVisible() { return this->bVisible; }

// engine rectangle contains rect
bool CBaseUIElement::isVisibleOnScreen(const McRect &rect) { return engine->getScreenRect().intersects(rect); }
bool CBaseUIElement::isVisibleOnScreen(CBaseUIElement *elem) {
    return CBaseUIElement::isVisibleOnScreen(elem->getRect());
}
bool CBaseUIElement::isVisibleOnScreen() const { return CBaseUIElement::isVisibleOnScreen(this->getRect()); }

bool CBaseUIElement::isEnabled() { return this->bEnabled; }
bool CBaseUIElement::isBusy() { return this->bBusy && this->isVisible(); }
bool CBaseUIElement::isMouseInside() { return this->bMouseInside && this->isVisible(); }

CBaseUIElement *CBaseUIElement::setPos(vec2 newPos) {
    if(newPos != this->rect.getPos()) {
        this->rect.setPos(newPos);
        this->onMoved();
    }
    return this;
}
CBaseUIElement *CBaseUIElement::setPos(float xPos, float yPos) { return this->setPos({xPos, yPos}); }
CBaseUIElement *CBaseUIElement::setPosX(float xPos) { return this->setPos({xPos, this->rect.getPos().y}); }
CBaseUIElement *CBaseUIElement::setPosY(float yPos) { return this->setPos({this->rect.getPos().x, yPos}); }
CBaseUIElement *CBaseUIElement::setRelPos(vec2 newRelPos) {
    this->relRect.setPos(newRelPos);
    return this;
}
CBaseUIElement *CBaseUIElement::setRelPos(float xPos, float yPos) { return this->setRelPos({xPos, yPos}); }
CBaseUIElement *CBaseUIElement::setRelPosX(float xPos) {
    this->relRect.setPosX(xPos);
    return this;
}
CBaseUIElement *CBaseUIElement::setRelPosY(float yPos) {
    this->relRect.setPosY(yPos);
    return this;
}

CBaseUIElement *CBaseUIElement::setSize(vec2 newSize) {
    if(newSize != this->rect.getSize()) {
        this->rect.setSize(newSize);
        this->onResized();
        this->onMoved();
    }
    return this;
}
CBaseUIElement *CBaseUIElement::setSize(float xSize, float ySize) { return this->setSize({xSize, ySize}); }
CBaseUIElement *CBaseUIElement::setSizeX(float xSize) { return this->setSize({xSize, this->rect.getSize().y}); }
CBaseUIElement *CBaseUIElement::setSizeY(float ySize) { return this->setSize({this->rect.getSize().x, ySize}); }

CBaseUIElement *CBaseUIElement::setRect(McRect rect) {
    this->rect = std::move(rect);
    return this;
}

CBaseUIElement *CBaseUIElement::setRelRect(McRect rect) {
    this->relRect = std::move(rect);
    return this;
}

CBaseUIElement *CBaseUIElement::setVisible(bool visible) {
    this->bVisible = visible;
    return this;
}
CBaseUIElement *CBaseUIElement::setActive(bool active) {
    this->bActive = active;
    return this;
}
CBaseUIElement *CBaseUIElement::setKeepActive(bool keepActive) {
    this->bKeepActive = keepActive;
    return this;
}
CBaseUIElement *CBaseUIElement::setEnabled(bool enabled) {
    if(enabled != this->bEnabled) {
        this->bEnabled = enabled;
        if(this->bEnabled) {
            this->onEnabled();
        } else {
            this->onDisabled();
        }
    }
    return this;
}
CBaseUIElement *CBaseUIElement::setBusy(bool busy) {
    this->bBusy = busy;
    return this;
}
CBaseUIElement *CBaseUIElement::setName(std::string name) {
    this->sName = std::move(name);
    return this;
}
CBaseUIElement *CBaseUIElement::setHandleLeftMouse(bool handle) {
    this->bHandleLeftMouse = handle;
    return this;
}
CBaseUIElement *CBaseUIElement::setHandleRightMouse(bool handle) {
    this->bHandleRightMouse = handle;
    return this;
}

// TODO: remove this, changes behavior in more ways than just mouse handling
CBaseUIElement *CBaseUIElement::setGrabClicks(bool grabClicks) {
    this->grabs_clicks = grabClicks;
    return this;
}

void CBaseUIElement::onResized() { ; }
void CBaseUIElement::onMoved() { ; }

void CBaseUIElement::onFocusStolen() { ; }
void CBaseUIElement::onEnabled() { ; }
void CBaseUIElement::onDisabled() { ; }

void CBaseUIElement::onMouseInside() { ; }
void CBaseUIElement::onMouseOutside() { ; }
void CBaseUIElement::onMouseDownInside(bool /*left*/, bool /*right*/) { ; }
void CBaseUIElement::onMouseDownOutside(bool /*left*/, bool /*right*/) { ; }
void CBaseUIElement::onMouseUpInside(bool /*left*/, bool /*right*/) { ; }
void CBaseUIElement::onMouseUpOutside(bool /*left*/, bool /*right*/) { ; }

void CBaseUIElement::stealFocus() {
    this->mouseInsideCheck = (u8)(this->bHandleLeftMouse << 1) | (u8)this->bHandleRightMouse;
    this->bActive = false;
    this->onFocusStolen();
}

void CBaseUIElement::update(CBaseUIEventCtx &c) {
    if(unlikely(CBaseUIDebug::dumpElems)) this->dumpElem();
    if(!this->bVisible || !this->bEnabled) return;

    // TODO: hover "consumption"
    {
        const bool oldMouseInsideState = this->bMouseInside;

        // to avoid issues with mouse position right along the boundaries
        if(!oldMouseInsideState) {
            // going into strictly-contains area from outside
            if(this->getRect().containsStrict(mouse->getPos())) {
                this->bMouseInside = true;
            }
        } else {
            // leaving deadzone area from inside
            if(!this->getRect().contains(mouse->getPos())) {
                this->bMouseInside = false;
            }
        }

        // re-check to account for possible isMouseInside override
        if((this->bMouseInside = this->isMouseInside())) {
            c.propagate_hover = false;  // doesn't really do anything much atm
        }

        if(oldMouseInsideState != this->bMouseInside) {
            if(unlikely(CBaseUIDebug::traceLevel() > 0))
                CBaseUIDebug::traceEvent(this, this->bMouseInside ? "hoverIn" : "hoverOut");
            if(this->bMouseInside) {
                this->onMouseInside();
            } else {
                this->onMouseOutside();
            }
        }
    }

    const u8 rawButtonMask = (u8)((this->bHandleLeftMouse && mouse->isLeftDown()) << 1) |
                             (u8)(this->bHandleRightMouse && mouse->isRightDown());

    // if update() wasn't called last frame (e.g. element or a parent container was invisible),
    // treat any already-held buttons as stale and ignore them until released
    // TODO: this is a stop-gap until mouse handling is separated from update()
    const u32 currentFrame = (u32)engine->getFrameCount();
    if(currentFrame - this->lastUpdateFrame > 1) {
        this->staleButtons = rawButtonMask;
    }
    this->lastUpdateFrame = currentFrame;
    this->staleButtons &= rawButtonMask;
    const u8 buttonMask = rawButtonMask & ~this->staleButtons;

    if(buttonMask && c.propagate_clicks) {
        this->mouseUpCheck |= buttonMask;
        if(this->bMouseInside) {
            c.propagate_clicks &= !this->grabs_clicks;
        }

        // onMouseDownOutside
        if(!this->bMouseInside && !(this->mouseInsideCheck & buttonMask)) {
            this->mouseInsideCheck |= buttonMask;
            if(unlikely(CBaseUIDebug::traceLevel() > 1)) CBaseUIDebug::traceEvent(this, "downOutside");
            this->onMouseDownOutside((buttonMask & 0b10), (buttonMask & 0b01));
        }

        // onMouseDownInside
        if(this->bMouseInside && !(this->mouseInsideCheck & buttonMask)) {
            this->bActive = true;
            this->mouseInsideCheck |= buttonMask;
            if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(this, "downInside");
            this->onMouseDownInside((buttonMask & 0b10), (buttonMask & 0b01));
        }
    }

    // detect which buttons were released for mouse up events
    const u8 releasedButtons = this->mouseUpCheck & ~buttonMask;
    if(releasedButtons && this->bActive) {
        if(this->bMouseInside) {
            if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(this, "upInside");
            this->onMouseUpInside((releasedButtons & 0b10), (releasedButtons & 0b01));
        } else {
            if(unlikely(CBaseUIDebug::traceLevel() > 1)) CBaseUIDebug::traceEvent(this, "upOutside");
            this->onMouseUpOutside((releasedButtons & 0b10), (releasedButtons & 0b01));
        }

        if(!this->bKeepActive) this->bActive = false;
    }

    // remove released buttons from mouseUpCheck
    this->mouseUpCheck &= buttonMask;

    // reset mouseInsideCheck if all buttons are released
    if(!buttonMask) {
        this->mouseInsideCheck = 0b00;
    }
}

void CBaseUIElement::dumpElem() const {
    using namespace CBaseUIDebug;
    u64 currentFrame = engine->getFrameCount();
    logRaw(R"(==== UI ELEMENT {:p} DEBUG ====
frame:              {}
sName:              {}
bVisible:           {}
bActive:            {}
bBusy:              {}
bEnabled:           {}
bKeepActive:        {}
bMouseInside:       {}
bHandleLeftMouse:   {}
bHandleRightMouse:  {}
rect:               {}
relRect:            {}
mouseInsideCheck:   {:02b}
mouseUpCheck:       {:02b}
==== END UI ELEMENT DEBUG ====)",
           fmt::ptr(this), currentFrame, this->getName(), this->bVisible, this->bActive, this->bBusy, this->bEnabled,
           this->bKeepActive, this->bMouseInside, this->bHandleLeftMouse, this->bHandleRightMouse, this->rect,
           this->relRect, this->mouseInsideCheck, this->mouseUpCheck);
}

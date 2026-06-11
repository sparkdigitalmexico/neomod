// Copyright (c) 2013, PG, All rights reserved.
#include "CBaseUIElement.h"

#include "Engine.h"
#include "Logging.h"
#include "ConVar.h"
#include "Mouse.h"
#include "UIDispatch.h"

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

void CBaseUIEventCtx::beginHitGroup() { this->hitGroupStarts.push_back(this->hitCandidates.size()); }

void CBaseUIEventCtx::addHitCandidate(CBaseUIElement *elem) {
    if(this->hitGroupStarts.empty()) this->hitGroupStarts.push_back(0);  // implicit single group
    this->hitCandidates.push_back({.elem = elem, .tier = this->currentHitTier, .path = this->hitPath});
}

CBaseUIElement::CBaseUIElement(float xPos, float yPos, float xSize, float ySize, std::nullptr_t /**/)
    : rect(xPos, yPos, xSize, ySize), relRect(this->rect) {}

CBaseUIElement::CBaseUIElement(float xPos, float yPos, float xSize, float ySize, std::string name)
    : sName(std::move(name)), rect(xPos, yPos, xSize, ySize), relRect(this->rect) {}

CBaseUIElement::~CBaseUIElement() {
    // null during static teardown (Logger can keep the ConsoleBox alive past Engine shutdown (FIXME))
    if(auto *dispatch = UIDispatch::get()) dispatch->onElementDestroyed(this);
}

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
bool CBaseUIElement::onWheel(int /*deltaVertical*/, int /*deltaHorizontal*/) { return false; }
void CBaseUIElement::onMouseCancel() { ; }
void CBaseUIElement::onCapturedMouseMove() { ; }
void CBaseUIElement::onCapturedMoveThrough() { ; }
void CBaseUIElement::onCapturedEndThrough() { ; }

void CBaseUIElement::lockCapture() {
    if(auto *dispatch = UIDispatch::get()) dispatch->lockCapture(this);
}

void CBaseUIElement::stealCapture() {
    if(auto *dispatch = UIDispatch::get()) dispatch->stealCapture(this);
}

void CBaseUIElement::stealFocus() {
    this->bActive = false;
    this->onFocusStolen();
}

void CBaseUIElement::tick() {
    if(unlikely(CBaseUIDebug::dumpElems)) this->dumpElem();
}

void CBaseUIElement::updateInput(CBaseUIEventCtx &c) {
    if(!this->bVisible || !this->bEnabled) return;

    this->lastInputFrame = engine->getFrameCount();

    // TODO: hover "consumption"
    {
        const bool oldMouseInsideState = this->bMouseInside;

        // to avoid issues with mouse position right along the boundaries
        if(!oldMouseInsideState) {
            // going into strictly-contains area from outside; while a capture is held, nothing
            // but the captor may GAIN hover (losing it below stays allowed)
            const CBaseUIElement *captor = UIDispatch::get()->getCaptor();
            if((captor == nullptr || captor == this) && this->getRect().containsStrict(mouse->getPos())) {
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

    if(c.propagate_clicks && (this->bHandleLeftMouse || this->bHandleRightMouse)) {
        // hit candidacy: who MAY receive this frame's button events; the single-target delivery
        // happens in UIDispatch::dispatchEvents after the walk
        if(this->bMouseInside && !this->bClickThroughSelf) c.addHitCandidate(this);

        // outside-downs stay a per-element broadcast: they are the "pressed elsewhere" signal
        // (close-on-click-outside, focus loss, ...) until the phase 4 focus manager replaces
        // them. the captor is excluded, its events are routed in UIDispatch::dispatchEvents.
        const u8 pressedMask = (u8)((this->bHandleLeftMouse && mouse->isLeftPressed()) << 1) |
                               (u8)(this->bHandleRightMouse && mouse->isRightPressed());
        if(pressedMask && !this->bMouseInside && this != UIDispatch::get()->getCaptor()) {
            if(unlikely(CBaseUIDebug::traceLevel() > 1)) CBaseUIDebug::traceEvent(this, "downOutside");
            this->onMouseDownOutside((pressedMask & 0b10), (pressedMask & 0b01));
        }

        // preserve the click-blocking flag for elements/screens visited later in the walk
        // (cross-screen input-blocking idioms depend on it; the phase 3 layer stack replaces it)
        const bool buttonHeld =
            (this->bHandleLeftMouse && mouse->isLeftDown()) || (this->bHandleRightMouse && mouse->isRightDown());
        if(buttonHeld && this->bMouseInside) c.propagate_clicks &= !this->grabs_clicks;
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
==== END UI ELEMENT DEBUG ====)",
           fmt::ptr(this), currentFrame, this->getName(), this->bVisible, this->bActive, this->bBusy, this->bEnabled,
           this->bKeepActive, this->bMouseInside, this->bHandleLeftMouse, this->bHandleRightMouse, this->rect,
           this->relRect);
}

std::span<CBaseUIElement *const> CBaseUIElement::getAllChildren() const { return {}; }

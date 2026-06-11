#pragma once
// Copyright (c) 2013, PG, All rights reserved.

#include "KeyboardListener.h"
#include "Vectors.h"
#include "Rect.h"

#include <string>
#include <string_view>
#include <span>
#include <vector>

class CBaseUIElement;

class MouseListener;

// convar callbacks to avoid hammering atomic convar reads
namespace CBaseUIDebug {
void onDumpElemsChangeCallback(float newvalue);
void onTraceChangeCallback(float newvalue);

// element name for debug output: sName if set, demangled type name otherwise
std::string elemName(const CBaseUIElement *elem);

// ui_trace level (0 = off), see the ui_trace convar
[[nodiscard]] int traceLevel();

// logs "uitrace frame=N evt=<evt> elem=<name>" for scripted-test golden diffing
void traceEvent(const CBaseUIElement *elem, std::string_view evt);
}  // namespace CBaseUIDebug

// Guidelines for avoiding hair pulling:
// - Don't use m_vmSize
// - When an element is standalone, use getPos/setPos
// - In a container or in a scrollview, use getRelPos/setRelPos and call update_pos() on the container

enum class TEXT_JUSTIFICATION : u8 { LEFT, CENTERED, RIGHT };

class CBaseUIContainer;

struct CBaseUIEventCtx {
    bool propagate_clicks{true};
    bool propagate_hover{true};  // TODO: does not work quite right yet

    void consume_mouse();

    [[nodiscard]] bool mouse_consumed() const;

    // pass-A hit-candidate collection: elements under the cursor that may receive this frame's
    // button events; single-target delivery happens in CBaseUIElement::dispatchMouseEvents after
    // the walk. groups = top-level screens/overlays in input-priority order; within a group the
    // best (tier, latest visit) candidate wins, approximating top-most draw order until the
    // phase 3 layer stack provides real z
    struct HitCandidate {
        CBaseUIElement *elem;
        int tier;
    };
    std::vector<HitCandidate> hitCandidates;
    std::vector<size_t> hitGroupStarts;
    int currentHitTier{0};  // raised around children that draw above later-visited siblings

    void beginHitGroup();
    void addHitCandidate(CBaseUIElement *elem);
};

class CBaseUIElement : public KeyboardListener {
    NOCOPY_NOMOVE(CBaseUIElement)
   public:
    CBaseUIElement(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::nullptr_t = {});
    CBaseUIElement(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::string name = {});
    ~CBaseUIElement() override;

    // main
    virtual void draw() = 0;

    // logic/animations/async polling; always runs, regardless of visibility or input consumption
    virtual void tick();

    // mouse input pass: hover + hit-candidate registration; gated and priority-ordered by the caller
    virtual void updateInput(CBaseUIEventCtx &c);

    // which UI root a dispatch call serves; the engine root (guiContainer) dispatches before the
    // app root each frame and consumes the events it delivers
    enum class UIRoot : uint8_t { ENGINE, APP };

    // the UI layer's MouseListener: buffers the frame's button events off the same relay every
    // other consumer uses (registered once at engine startup, like the keyboard listeners).
    // Mouse itself knows nothing about UI routing; consumption state lives in the buffer here.
    static MouseListener &mouseEventSink();

    // pass B: routes this frame's buffered mouse button events to the candidates collected during
    // the updateInput walk. down -> best candidate + implicit capture, up -> whoever received the
    // down. call once per root, directly after its updateInput pass.
    static void dispatchMouseEvents(CBaseUIEventCtx &c, UIRoot root);

    // keyboard input (nothing by default)
    void onKeyUp(KeyboardEvent &e) override;
    void onKeyDown(KeyboardEvent &e) override;
    void onChar(KeyboardEvent &e) override;

    // getters
    [[nodiscard]] std::string_view getName() const;

    [[nodiscard]] const McRect &getRect() const;

    [[nodiscard]] const vec2 &getPos() const;
    [[nodiscard]] const vec2 &getSize() const;

    [[nodiscard]] const McRect &getRelRect() const;

    [[nodiscard]] const vec2 &getRelPos() const;
    [[nodiscard]] const vec2 &getRelSize() const;

    virtual bool isActive();
    virtual bool isVisible();

    // engine rectangle contains rect
    static bool isVisibleOnScreen(const McRect &rect);
    [[nodiscard]] static bool isVisibleOnScreen(CBaseUIElement *elem);
    [[nodiscard]] bool isVisibleOnScreen() const;

    virtual bool isEnabled();
    virtual bool isBusy();
    virtual bool isMouseInside();

    CBaseUIElement *setPos(vec2 newPos);

    CBaseUIElement *setPos(float xPos, float yPos);
    CBaseUIElement *setPosX(float xPos);
    CBaseUIElement *setPosY(float yPos);
    CBaseUIElement *setRelPos(vec2 newRelPos);
    CBaseUIElement *setRelPos(float xPos, float yPos);
    CBaseUIElement *setRelPosX(float xPos);
    CBaseUIElement *setRelPosY(float yPos);

    CBaseUIElement *setSize(vec2 newSize);
    CBaseUIElement *setSize(float xSize, float ySize);
    CBaseUIElement *setSizeX(float xSize);
    CBaseUIElement *setSizeY(float ySize);

    CBaseUIElement *setRect(McRect rect);

    CBaseUIElement *setRelRect(McRect rect);

    virtual CBaseUIElement *setVisible(bool visible);
    virtual CBaseUIElement *setActive(bool active);
    virtual CBaseUIElement *setKeepActive(bool keepActive);
    virtual CBaseUIElement *setEnabled(bool enabled);
    virtual CBaseUIElement *setBusy(bool busy);
    virtual CBaseUIElement *setName(std::string name);
    virtual CBaseUIElement *setHandleLeftMouse(bool handle);
    virtual CBaseUIElement *setHandleRightMouse(bool handle);

    // TODO: remove this, changes behavior in more ways than just mouse handling
    virtual CBaseUIElement *setGrabClicks(bool grabClicks);

    // actions
    virtual void stealFocus();

    void dumpElem() const;  // debug
    [[nodiscard]] virtual std::span<CBaseUIElement *const> getAllChildren() const;

   protected:
    friend class CBaseUIContainer;

    // events (default implementation does nothing for all of these)
    virtual void onResized();
    virtual void onMoved();

    virtual void onFocusStolen();
    virtual void onEnabled();
    virtual void onDisabled();

    virtual void onMouseInside();
    virtual void onMouseOutside();
    virtual void onMouseDownInside(bool /*left*/ = true, bool /*right*/ = false);
    virtual void onMouseDownOutside(bool /*left*/ = true, bool /*right*/ = false);
    virtual void onMouseUpInside(bool /*left*/ = true, bool /*right*/ = false);
    virtual void onMouseUpOutside(bool /*left*/ = true, bool /*right*/ = false);

    // vars
    std::string sName;

    // position and size
    McRect rect;
    McRect relRect;

    // vec2 &vPos;    // reference to rect.vMin
    // vec2 &vSize;   // reference to rect.vSize
    // vec2 &vmPos;   // reference to relRect.vMin
    // vec2 &vmSize;  // reference to relRect.vSize

    // attributes

   private:
    // stamped by every updateInput pass; an element is input-eligible for dispatch only if its
    // stamp is current (encodes all procedural screen gating without a declarative visibility model)
    u64 lastInputFrame{0};

   protected:
    bool grabs_clicks : 1 {false};  // TODO: remove this (confusing behavior)
    // transparent wrappers (containers) don't hit-candidate their own rect; real widgets with a
    // self-rect surface (scrollview, window) opt back in
    bool bClickThroughSelf : 1 {false};
    bool bVisible : 1 {true};
    bool bActive : 1 {false};  // we are doing something, e.g. textbox is blinking and ready to receive input
    bool bBusy : 1 {false};    // we demand the focus to be kept on us, e.g. click-drag scrolling in a scrollview
    bool bEnabled : 1 {true};

    bool bKeepActive : 1 {false};  // once clicked, don't lose m_bActive, we have to manually release it (e.g. textbox)
    bool bMouseInside : 1 {false};

    bool bHandleLeftMouse : 1 {true};
    bool bHandleRightMouse : 1 {false};
};

#pragma once
// Copyright (c) 2013, PG, All rights reserved.

#include "KeyboardListener.h"
#include "Vectors.h"
#include "Rect.h"

#include <string>
#include <string_view>

// convar callback to avoid hammering atomic convar reads
namespace CBaseUIDebug {
void onDumpElemsChangeCallback(float newvalue);
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
};

class CBaseUIElement : public KeyboardListener {
    NOCOPY_NOMOVE(CBaseUIElement)
   public:
    CBaseUIElement(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::nullptr_t = {});
    CBaseUIElement(float xPos = 0, float yPos = 0, float xSize = 0, float ySize = 0, std::string name = {});
    ~CBaseUIElement() override;

    // main
    virtual void draw() = 0;
    virtual void update(CBaseUIEventCtx &c);

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
    u32 lastUpdateFrame{0};
    u8 mouseInsideCheck : 2 {0};
    u8 mouseUpCheck : 2 {0};
    u8 staleButtons : 2 {0};

   protected:
    bool grabs_clicks : 1 {false};  // TODO: remove this (confusing behavior)
    bool bVisible : 1 {true};
    bool bActive : 1 {false};  // we are doing something, e.g. textbox is blinking and ready to receive input
    bool bBusy : 1 {false};    // we demand the focus to be kept on us, e.g. click-drag scrolling in a scrollview
    bool bEnabled : 1 {true};

    bool bKeepActive : 1 {false};  // once clicked, don't lose m_bActive, we have to manually release it (e.g. textbox)
    bool bMouseInside : 1 {false};

    bool bHandleLeftMouse : 1 {true};
    bool bHandleRightMouse : 1 {false};
};

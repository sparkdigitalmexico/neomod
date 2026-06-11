#pragma once
// Copyright (c) 2026, WH, All rights reserved.
// TODO: rename to CBaseUIDispatch for consistency

#include "MouseListener.h"
#include "Vectors.h"
#include "types.h"

#include <vector>

class CBaseUIElement;
struct CBaseUIEventCtx;

// pass-B mouse routing + capture state for the CBaseUI layer. one instance, owned by Engine
// alongside the engine gui root; it listens on the same Mouse relay as every other consumer
// (Mouse itself knows nothing about UI routing; consumption state lives in the buffer here).
class UIDispatch final : public MouseListener {
    NOCOPY_NOMOVE(UIDispatch)
   public:
    // which UI root a dispatch call serves; the engine root (guiContainer) dispatches before the
    // app root each frame and consumes the events it delivers
    enum class Root : uint8_t { ENGINE, APP };

    UIDispatch();
    ~UIDispatch() override;

    // the live instance (owned by Engine); null before engine gui startup and after shutdown
    // (e.g. Logger can keep the ConsoleBox alive past Engine teardown)
    // FIXME: avoid the need to ever null-check, ConsoleBox staying alive past shutdown is a hack
    [[nodiscard]] static UIDispatch *get();

    // buffers the frame's button events off the regular Mouse listener relay (events arrive
    // during the input-device update, but routing must wait until the updateInput walk has
    // collected the hit candidates). self-cleans: the first event of a new frame drops the
    // previous frame's.
    void onButtonChange(ButtonEvent &ev) override;

    // wheel deltas off the same relay, accumulated into per-frame totals (same self-cleaning
    // model); routed in dispatchEvents to the top-most wheel-accepting hit candidate, or to
    // the captor while a capture is held. consumed-state is shared across both roots like the
    // button events (the engine root dispatches first).
    void onWheelVertical(int delta) override;
    void onWheelHorizontal(int delta) override;

    // routes this frame's buffered mouse button events to the candidates collected during the
    // updateInput walk. down -> best candidate + implicit capture, up -> whoever received the
    // down. call once per root, directly after its updateInput pass.
    void dispatchEvents(CBaseUIEventCtx &c, Root root);

    // element dtors report here: bumps the mutation generation and releases a dead captor
    void onElementDestroyed(CBaseUIElement *elem);

    // a locked capture cannot be stolen (slider grab, scrollbar drag, window drag/resize);
    // only the current captor may lock
    void lockCapture(const CBaseUIElement *who);

    // an ancestor on the captured hit path takes an unlocked descendant capture (scrollview past
    // drag resistance): the old captor gets onMouseCancel (its press dies, no click), ancestors
    // below the thief get onCapturedEndThrough, the thief becomes the locked captor
    void stealCapture(CBaseUIElement *thief);

    [[nodiscard]] CBaseUIElement *getCaptor() const { return this->captor; }
    [[nodiscard]] bool isCaptureLocked() const { return this->captorLocked; }
    // which buttons the current capture holds (for captured-move/observe handlers; reconciled
    // against the device state every dispatch)
    [[nodiscard]] MouseButtonFlags getCaptorButtons() const { return this->captorButtons; }

   private:
    // capture ends without an up: onMouseCancel to the captor + observation end (cancelCapture),
    // or observation end alone (endObservation, captor handled separately by the caller)
    void cancelCapture();
    void endObservation();

    // one captured frame: move to the captor, then observation through its ancestors
    // (innermost first; an observer may steal or mutate the UI mid-loop)
    void observeCapturedFrame();

    std::vector<ButtonEvent> queue;
    u64 lastPushFrame{0};

    // per-frame wheel totals
    int wheelVertical{0};
    int wheelHorizontal{0};
    u64 lastWheelFrame{0};
    bool wheelConsumed{false};

    // mouse capture: whichever element receives a down receives the matching up(s). one captor
    // globally (there is one pointer device), tagged with the UI root that owns it so the other
    // root's dispatch leaves its events alone
    CBaseUIElement *captor{nullptr};
    Root captorRoot{Root::APP};
    MouseButtonFlags captorButtons{};
    bool captorLocked{false};

    // the captor's ancestor chain at down time (outermost first): observes the drag once per
    // frame and may steal; element dtors prune themselves out
    std::vector<CBaseUIElement *> capturePath;

    // last cursor position delivered as a captured move (for move-edge trace gating)
    vec2 lastCaptureMovePos{0.f};

    // bumped on every element destruction; dispatch abandons the frame's remaining deliveries
    // when it changes mid-loop (a handler deleted/rebuilt parts of the UI under the candidates)
    u64 elemGeneration{0};
};

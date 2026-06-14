// Copyright (c) 2026, WH, All rights reserved.
#include "UIDispatch.h"

#include "CBaseUIElement.h"
#include "Engine.h"
#include "Mouse.h"

#include <algorithm>

namespace {
UIDispatch *liveDispatch{nullptr};

// the top-most hit candidate matching pred: groups in input-priority order, within a group the best
// (tier, then latest visit), the first group with a match winning (= the top-most layer). the single
// ranking shared by hover resolution and button-down targeting (wheel walks the same order with its
// own per-group floor), so the three pickers cannot disagree on what is "on top".
template <typename Pred>
const CBaseUIEventCtx::HitCandidate *topCandidate(const CBaseUIEventCtx &c, Pred pred) {
    for(uSz g = 0; g < c.hitGroupStarts.size(); ++g) {
        const uSz begin = c.hitGroupStarts[g];
        const uSz end = (g + 1 < c.hitGroupStarts.size()) ? c.hitGroupStarts[g + 1] : c.hitCandidates.size();
        const CBaseUIEventCtx::HitCandidate *best = nullptr;
        int bestTier = 0;
        for(uSz i = begin; i < end; ++i) {
            const auto &cand = c.hitCandidates[i];
            if(!pred(cand)) continue;
            if(best == nullptr || cand.tier >= bestTier) {
                best = &cand;
                bestTier = cand.tier;
            }
        }
        if(best != nullptr) return best;
    }
    return nullptr;
}
}  // namespace

UIDispatch::UIDispatch() { liveDispatch = this; }

UIDispatch::~UIDispatch() {
    // TODO: why is it removed in the destructor but added outside of it
    if(mouse) mouse->removeListener(this);
    liveDispatch = nullptr;
}

UIDispatch *UIDispatch::get() { return liveDispatch; }

void UIDispatch::onButtonChange(ButtonEvent &ev) {
    const u64 frame = engine->getFrameCount();
    if(frame != this->lastPushFrame) {
        this->queue.clear();
        this->lastPushFrame = frame;
    }
    this->queue.push_back(ev);
}

void UIDispatch::onWheelVertical(int delta) {
    const u64 frame = engine->getFrameCount();
    if(frame != this->lastWheelFrame) {
        this->wheelVertical = this->wheelHorizontal = 0;
        this->wheelConsumed = this->wheelFloored = false;
        this->lastWheelFrame = frame;
    }
    this->wheelVertical += delta;
}

void UIDispatch::onWheelHorizontal(int delta) {
    const u64 frame = engine->getFrameCount();
    if(frame != this->lastWheelFrame) {
        this->wheelVertical = this->wheelHorizontal = 0;
        this->wheelConsumed = this->wheelFloored = false;
        this->lastWheelFrame = frame;
    }
    this->wheelHorizontal += delta;
}

void UIDispatch::setFocus(CBaseUIElement *elem) {
    if(this->focused == elem) return;
    CBaseUIElement *old = this->focused;
    this->focused = elem;
    // the old holder loses focus (bActive=false + onFocusStolen); its own clearFocusIf is a
    // no-op now that focused already moved, so this does not recurse
    if(old != nullptr) old->stealFocus();
}

void UIDispatch::onElementDestroyed(CBaseUIElement *elem) {
    ++this->elemGeneration;
    if(this->wheelSink == elem) this->wheelSink = nullptr;
    if(this->focused == elem) this->focused = nullptr;
    if(this->captor == elem) {
        // the captor is mid-destruction: no calls into it, but observers still disarm
        this->captor = nullptr;
        this->captorButtons = {};
        this->captorLocked = false;
        this->endObservation();
    } else {
        std::erase(this->capturePath, elem);
    }
}

void UIDispatch::lockCapture(const CBaseUIElement *who) {
    if(this->captor == who) this->captorLocked = true;
}

void UIDispatch::stealCapture(CBaseUIElement *thief) {
    if(this->captor == nullptr || this->captor == thief || this->captorLocked) return;

    const auto thiefIt = std::ranges::find(this->capturePath, thief);
    if(thiefIt == this->capturePath.end()) return;  // only observing ancestors may steal

    // detach the in-between ancestors before any handler runs (they may mutate the path);
    // the thief's own ancestors keep observing the new capture
    const std::vector<CBaseUIElement *> detached(thiefIt + 1, this->capturePath.end());
    this->capturePath.erase(thiefIt, this->capturePath.end());

    CBaseUIElement *old = this->captor;
    this->captor = thief;
    this->captorLocked = true;
    thief->bActive = true;
    if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(thief, "captureSteal");

    old->bActive = false;
    if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(old, "mouseCancel");
    old->onMouseCancel();
    for(auto it = detached.rbegin(); it != detached.rend(); ++it) (*it)->onCapturedEndThrough();
}

void UIDispatch::cancelCapture() {
    if(this->captor == nullptr) return;

    CBaseUIElement *old = this->captor;
    this->captor = nullptr;
    this->captorButtons = {};
    this->captorLocked = false;

    old->bActive = false;
    if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(old, "mouseCancel");
    old->onMouseCancel();
    this->endObservation();
}

void UIDispatch::endObservation() {
    // move out: end handlers may start new captures or destroy elements
    const std::vector<CBaseUIElement *> path = std::move(this->capturePath);
    this->capturePath.clear();
    for(auto it = path.rbegin(); it != path.rend(); ++it) (*it)->onCapturedEndThrough();
}

void UIDispatch::observeCapturedFrame() {
    const vec2 pos = mouse->getPos();
    if(pos != this->lastCaptureMovePos) {
        this->lastCaptureMovePos = pos;
        if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(this->captor, "captureMove");
    }
    this->captor->onCapturedMouseMove();

    // innermost first; an observer may steal mid-loop (the path shrinks to its own ancestors,
    // which are exactly the entries not yet visited) or mutate the UI (generation guard)
    const u64 observeGeneration = this->elemGeneration;
    const std::vector<CBaseUIElement *> path = this->capturePath;
    for(auto it = path.rbegin(); it != path.rend(); ++it) {
        if(this->captor == nullptr || this->elemGeneration != observeGeneration) break;
        if(std::ranges::find(this->capturePath, *it) == this->capturePath.end()) continue;
        (*it)->onCapturedMoveThrough();
    }
}

void UIDispatch::resolveHover(CBaseUIEventCtx &c, Root root) {
    const u64 frame = engine->getFrameCount();
    if(frame != this->lastHoverFrame) {
        this->hoverClaimed = false;
        this->lastHoverFrame = frame;
    }

    // a held capture freezes hover GAINS for its own root: nothing beneath the captor gains (the
    // captor keeps the hover it had at the down; losing-on-rect-leave is handled in updateInput).
    // leaving the candidates untouched keeps the gesture's hover state from churning.
    if(this->captor != nullptr && this->captorRoot == root) return;

    // the hovered set = the top-most candidate (+ its ancestor path), the same ranking the button
    // targeting uses. an upper root that already claimed hover (the engine console draws on top)
    // suppresses this root entirely (empty set -> everything beneath retracts).
    std::vector<CBaseUIElement *> set;
    if(!(root == Root::APP && this->hoverClaimed)) {
        const CBaseUIEventCtx::HitCandidate *top =
            topCandidate(c, [](const CBaseUIEventCtx::HitCandidate &cand) { return !cand.wheelOnly; });
        if(top != nullptr) {
            set = top->path;
            set.push_back(top->elem);
            this->hoverClaimed = true;
        }
    }

    // GAIN the set members not already hovered (a candidate's hover GAIN happens ONLY here, so an
    // occluded one never briefly gains it during the walk and plays a spurious hover sound), and
    // RETRACT candidates that fell out of the set (occluded by a higher candidate, or this root
    // suppressed by an upper one).
    for(const auto &cand : c.hitCandidates) {
        if(cand.wheelOnly) continue;
        CBaseUIElement *e = cand.elem;
        const bool want = std::ranges::find(set, e) != set.end();
        if(want && !e->bMouseInside) {
            e->bMouseInside = true;
            if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(e, "hoverIn");
            e->onMouseInside();
        } else if(!want && e->bMouseInside) {
            e->bMouseInside = false;
            if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(e, "hoverOut");
            e->onMouseOutside();
        }
    }
}

void UIDispatch::dispatchEvents(CBaseUIEventCtx &c, Root root) {
    using namespace flags::operators;

    const u64 frame = engine->getFrameCount();
    const bool hasEvents = (this->lastPushFrame == frame && !this->queue.empty());

    if(this->captor != nullptr && this->captorRoot == root) {
        // self-heal: a captured button can be released without an up event ever reaching us (e.g.
        // released while the window was unfocused; Mouse reconciles buttonsHeldMask on reset).
        // pending same-frame ups count as held so they still deliver below.
        MouseButtonFlags pendingUps{};
        if(hasEvents) {
            for(const auto &qe : this->queue) {
                if(!qe.down) pendingUps |= qe.btn;
            }
        }
        this->captorButtons &= (mouse->getHeldButtons() | pendingUps);
        if(!this->captorButtons) this->cancelCapture();
    }

    // captured phase: cancel-on-ineligible + the per-frame captured move and ancestor
    // observation. runs BEFORE the event loop so a release this frame acts on state that
    // already includes this frame's motion (e.g. scrollview kinetic launch).
    if(this->captor != nullptr && this->captorRoot == root) {
        // input-eligible this frame = the updateInput walk reached it (covers hidden, disabled
        // and procedurally input-blocked, none of which stamp)
        if(this->captor->lastInputFrame != frame) {
            this->cancelCapture();
        } else {
            this->observeCapturedFrame();
        }
    }

    // hover is resolved every frame, independent of button/wheel events (the cursor moves without
    // them); must run before the early-out below
    this->resolveHover(c, root);

    const bool hasWheel = (this->lastWheelFrame == frame && !this->wheelConsumed &&
                           (this->wheelVertical != 0 || this->wheelHorizontal != 0));
    if(!hasEvents && !hasWheel) return;

    const u64 startGeneration = this->elemGeneration;

    // wheel before buttons (pre-2.3 it acted during the input walk, ahead of click delivery):
    // the frame's totals go to ONE consumer, with decliners falling through (scroll chaining:
    // a button or a can't-scroll dropdown above its scrollview passes the wheel on). while a
    // capture is held the chain is the captor then its observing ancestors innermost-first
    // (mirrors the drag-steal chain); otherwise the top-most wheel-accepting candidate wins -
    // groups in input-priority order, within a group by (tier, latest visit) like the button
    // targeting
    if(hasWheel) {
        if(this->captor != nullptr) {
            if(this->captorRoot == root) {
                this->wheelConsumed = true;
                CBaseUIElement *acceptor = nullptr;
                if(this->captor->onWheel(this->wheelVertical, this->wheelHorizontal)) {
                    acceptor = this->captor;
                } else {
                    const std::vector<CBaseUIElement *> path = this->capturePath;
                    for(auto it = path.rbegin(); it != path.rend() && acceptor == nullptr; ++it) {
                        if(this->elemGeneration != startGeneration) break;
                        if((*it)->onWheel(this->wheelVertical, this->wheelHorizontal)) acceptor = *it;
                    }
                }
                if(acceptor != nullptr) {
                    if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(acceptor, "wheel");
                }
            }
        } else {
            for(uSz g = 0; g < c.hitGroupStarts.size() && !this->wheelConsumed && !this->wheelFloored; ++g) {
                const uSz begin = c.hitGroupStarts[g];
                const uSz end = (g + 1 < c.hitGroupStarts.size()) ? c.hitGroupStarts[g + 1] : c.hitCandidates.size();
                if(begin == end) continue;

                std::vector<uSz> order;
                order.reserve(end - begin);
                for(uSz i = begin; i < end; ++i) order.push_back(i);
                std::ranges::sort(order, [&c](uSz a, uSz b) {
                    return c.hitCandidates[a].tier != c.hitCandidates[b].tier
                               ? c.hitCandidates[a].tier > c.hitCandidates[b].tier
                               : a > b;
                });

                bool groupHasScrollSurface = false;
                for(const uSz i : order) {
                    if(this->elemGeneration != startGeneration) return;  // a handler mutated the UI
                    CBaseUIElement *elem = c.hitCandidates[i].elem;
                    groupHasScrollSurface |= elem->bWheelSurface;
                    if(elem->onWheel(this->wheelVertical, this->wheelHorizontal)) {
                        if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(elem, "wheel");
                        this->wheelConsumed = true;
                        break;
                    }
                }

                // wheel floor: a hovered scroll surface that declined (fits-content, scrolling
                // disabled, alt) still visually OWNS the point - the gesture must not act on
                // surfaces occluded by its layer. in-group chaining above already happened (an
                // anchored dropdown falls to the scrollview it rides), and the fall-through
                // sink below stays reachable (a dead wheel is still the global volume gesture).
                if(groupHasScrollSurface) this->wheelFloored = true;
            }

            // nothing wanted it: offer the totals to the fall-through sink. the app root
            // dispatches last each frame, so the sink sees only deltas neither root's
            // candidates consumed
            if(root == Root::APP && !this->wheelConsumed && this->wheelSink != nullptr) {
                if(this->wheelSink->onWheel(this->wheelVertical, this->wheelHorizontal)) {
                    if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(this->wheelSink, "wheel");
                    this->wheelConsumed = true;
                }
            }
        }
    }

    if(!hasEvents) return;
    auto &events = this->queue;

    for(auto &qe : events) {
        if(qe.consumed) continue;

        const MouseButtonFlags btn = qe.btn;
        const bool left = (btn == MouseButtonFlags::MF_LEFT);
        const bool right = (btn == MouseButtonFlags::MF_RIGHT);
        if(!left && !right) continue;  // middle/X buttons have no UI semantics (app code polls them)

        if(this->elemGeneration != startGeneration) break;  // a handler mutated the UI, candidates are stale

        if(this->captor != nullptr) {
            if(this->captorRoot != root) continue;  // the owning root's dispatch handles these
            qe.consumed = true;

            CBaseUIElement *elem = this->captor;
            // input-eligible this frame = the updateInput walk reached it (procedural gating)
            const bool eligible = (elem->lastInputFrame == frame);
            if(qe.down) {
                this->captorButtons |= btn;
                if(eligible) {
                    if(elem->bMouseInside) {
                        if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(elem, "downInside");
                        elem->onMouseDownInside(left, right);
                        elem->bActive = true;
                    } else {
                        if(unlikely(CBaseUIDebug::traceLevel() > 1)) CBaseUIDebug::traceEvent(elem, "downOutside");
                        elem->onMouseDownOutside(left, right);
                    }
                }
            } else {
                this->captorButtons &= ~btn;
                // bActive gate: a capture steal during the hold cancelled the press already
                if(eligible && elem->bActive) {
                    if(elem->bMouseInside) {
                        if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(elem, "upInside");
                        elem->onMouseUpInside(left, right);
                    } else {
                        if(unlikely(CBaseUIDebug::traceLevel() > 1)) CBaseUIDebug::traceEvent(elem, "upOutside");
                        elem->onMouseUpOutside(left, right);
                    }
                    if(!elem->bKeepActive) elem->bActive = false;
                } else if(!eligible) {
                    // captor went hidden/blocked between the captured-phase check and this event
                    // (mid-event-loop mutation): same cancel semantics, no click
                    this->captorButtons = {};
                    this->cancelCapture();
                    continue;
                }
                if(!this->captorButtons) {
                    this->captor = nullptr;
                    this->captorLocked = false;
                    this->endObservation();
                }
            }
        } else if(qe.down) {
            // route to the best candidate (the shared top-most ranking, filtered to handlers of
            // this button): groups in input-priority order, within a group by (tier, latest visit)
            const CBaseUIEventCtx::HitCandidate *target =
                topCandidate(c, [left, right](const CBaseUIEventCtx::HitCandidate &cand) {
                    if(cand.wheelOnly) return false;
                    if(left && !cand.elem->bHandleLeftMouse) return false;
                    if(right && !cand.elem->bHandleRightMouse) return false;
                    return true;
                });
            if(target == nullptr) continue;  // nothing under the cursor in this root; the other may deliver

            qe.consumed = true;
            this->captor = target->elem;
            this->captorRoot = root;
            this->captorButtons = btn;
            this->captorLocked = false;
            this->capturePath = target->path;
            this->lastCaptureMovePos = mouse->getPos();
            if(unlikely(CBaseUIDebug::traceLevel() > 0)) CBaseUIDebug::traceEvent(target->elem, "downInside");
            target->elem->onMouseDownInside(left, right);
            target->elem->bActive = true;
            // ancestors observe from the press frame (a scrollview arms its drag gesture here)
            if(this->captor == target->elem) this->observeCapturedFrame();
        }
        // up with no captor: nobody received the down (or the captor died); leave it alone
    }
}

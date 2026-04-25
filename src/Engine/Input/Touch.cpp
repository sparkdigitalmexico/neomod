// Copyright (c) 2026, kiwec, All rights reserved.
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_touch.h>

#include "Engine.h"
#include "Environment.h"
#include "Touch.h"

Touch::Touch() {}

void Touch::addListener(TouchListener* listener, bool insertOnTop) {
    if(listener == nullptr) {
        engine->showMessageError("Touch Error", "addListener(NULL)!");
        return;
    }

    if(insertOnTop)
        this->listeners.insert(this->listeners.begin(), listener);
    else
        this->listeners.push_back(listener);
}

void Touch::removeListener(TouchListener* listener) { std::erase(this->listeners, listener); }

void Touch::onFingerDown(SDL_TouchFingerEvent* ev) {
    assert(ev->type == SDL_EVENT_FINGER_DOWN);

    Finger finger{
        .id = ev->fingerID,
        .pos = vec2(ev->x, ev->y) * env->getWindowSize() * env->getPixelDensity(),
        .pressed_since_ns = ev->timestamp,
        .last_event_ns = ev->timestamp,
    };

    this->fingers.push_back(finger);

    for(auto* listener : this->listeners) {
        listener->onFingerPressed(finger);
    }
}

void Touch::onFingerMove(SDL_TouchFingerEvent* ev) {
    assert(ev->type == SDL_EVENT_FINGER_MOTION);

    for(auto& finger : this->fingers) {
        if(finger.id != ev->fingerID) continue;

        finger.pos = vec2(ev->x, ev->y) * env->getWindowSize() * env->getPixelDensity();
        finger.last_event_ns = ev->timestamp;

        for(auto* listener : this->listeners) {
            listener->onFingerMoved(finger);
        }

        break;
    }
}

void Touch::onFingerUp(SDL_TouchFingerEvent* ev) {
    assert(ev->type == SDL_EVENT_FINGER_UP);

    for(auto finger : this->fingers) {
        if(finger.id != ev->fingerID) continue;

        finger.pos = vec2(ev->x, ev->y) * env->getWindowSize() * env->getPixelDensity();
        finger.last_event_ns = ev->timestamp;

        std::erase_if(this->fingers, [&](const auto& f) { return f.id == ev->fingerID; });

        for(auto* listener : this->listeners) {
            listener->onFingerReleased(finger);
        }

        break;
    }
}

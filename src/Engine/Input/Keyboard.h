#pragma once
// Copyright (c) 2015, PG, 2026, WH, All rights reserved.
#include "types.h"

#include "InputDevice.h"
#include "KeyBindings.h"  // IWYU pragma: keep
#include "StaticPImpl.h"

#include <span>

typedef struct SDL_KeyboardEvent SDL_KeyboardEvent;
typedef struct SDL_TextInputEvent SDL_TextInputEvent;

using SCANCODE = uint16_t;
using KEYCODE = uint32_t;
using KEYMOD = uint16_t;

class KeyboardListener;
class SDLMain;

class Keyboard final : public InputDevice {
    NOCOPY_NOMOVE(Keyboard)

   public:
    Keyboard();
    ~Keyboard() override;

    void reset() override;
    void draw() override;
    void update() override;

    void addListener(KeyboardListener *keyboardListener, bool insertOnTop = false);
    void removeListener(KeyboardListener *keyboardListener);

    [[nodiscard]] bool isControlDown() const;
    [[nodiscard]] bool isAltDown() const;
    [[nodiscard]] bool isShiftDown() const;
    [[nodiscard]] bool isSuperDown() const;

    [[nodiscard]] bool areModsHeld(KEYMOD keymodMask) const;

    // don't use KEY_ prefixed keys here, use KEYMOD_ ones!
    [[nodiscard]] bool areModsHeld(KEYCODE keymodMask) = delete;

   private:
    // events received in main loop
    friend SDLMain;
    void onKey(SDL_KeyboardEvent keyEvent);
    void onChar(SDL_TextInputEvent textEvent);

    struct KeyboardImpl;
    StaticPImpl<KeyboardImpl, sizeof(void *) == 8 ? 64 : 32> m_impl;
};

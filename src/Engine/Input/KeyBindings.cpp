// Copyright (c) 2016, PG, All rights reserved.
#include "KeyBindings.h"
#include <SDL3/SDL_keyboard.h>

// clang-format off
#if defined(_WIN32)
#include "WinDebloatDefs.h"
#include <windows.h>
#elif defined __linux__
#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#endif

// alphabet
#define OLDKEY_A MC_SCANCODE_A
#define OLDKEY_B MC_SCANCODE_B
#define OLDKEY_C MC_SCANCODE_C
#define OLDKEY_D MC_SCANCODE_D
#define OLDKEY_E MC_SCANCODE_E
#define OLDKEY_F MC_SCANCODE_F
#define OLDKEY_G MC_SCANCODE_G
#define OLDKEY_H MC_SCANCODE_H
#define OLDKEY_I MC_SCANCODE_I
#define OLDKEY_J MC_SCANCODE_J
#define OLDKEY_K MC_SCANCODE_K
#define OLDKEY_L MC_SCANCODE_L
#define OLDKEY_M MC_SCANCODE_M
#define OLDKEY_N MC_SCANCODE_N
#define OLDKEY_O MC_SCANCODE_O
#define OLDKEY_P MC_SCANCODE_P
#define OLDKEY_Q MC_SCANCODE_Q
#define OLDKEY_R MC_SCANCODE_R
#define OLDKEY_S MC_SCANCODE_S
#define OLDKEY_T MC_SCANCODE_T
#define OLDKEY_U MC_SCANCODE_U
#define OLDKEY_V MC_SCANCODE_V
#define OLDKEY_W MC_SCANCODE_W
#define OLDKEY_X MC_SCANCODE_X
#define OLDKEY_Y MC_SCANCODE_Y
#define OLDKEY_Z MC_SCANCODE_Z

// numbers
#define OLDKEY_0 MC_SCANCODE_0
#define OLDKEY_1 MC_SCANCODE_1
#define OLDKEY_2 MC_SCANCODE_2
#define OLDKEY_3 MC_SCANCODE_3
#define OLDKEY_4 MC_SCANCODE_4
#define OLDKEY_5 MC_SCANCODE_5
#define OLDKEY_6 MC_SCANCODE_6
#define OLDKEY_7 MC_SCANCODE_7
#define OLDKEY_8 MC_SCANCODE_8
#define OLDKEY_9 MC_SCANCODE_9

// numpad
#define OLDKEY_NUMPAD0 MC_SCANCODE_KP_0
#define OLDKEY_NUMPAD1 MC_SCANCODE_KP_1
#define OLDKEY_NUMPAD2 MC_SCANCODE_KP_2
#define OLDKEY_NUMPAD3 MC_SCANCODE_KP_3
#define OLDKEY_NUMPAD4 MC_SCANCODE_KP_4
#define OLDKEY_NUMPAD5 MC_SCANCODE_KP_5
#define OLDKEY_NUMPAD6 MC_SCANCODE_KP_6
#define OLDKEY_NUMPAD7 MC_SCANCODE_KP_7
#define OLDKEY_NUMPAD8 MC_SCANCODE_KP_8
#define OLDKEY_NUMPAD9 MC_SCANCODE_KP_9
#define OLDKEY_MULTIPLY MC_SCANCODE_KP_MULTIPLY
#define OLDKEY_ADD MC_SCANCODE_KP_PLUS
#define OLDKEY_SEPARATOR MC_SCANCODE_KP_EQUALS
#define OLDKEY_SUBTRACT MC_SCANCODE_KP_MINUS
#define OLDKEY_DECIMAL MC_SCANCODE_KP_DECIMAL
#define OLDKEY_DIVIDE MC_SCANCODE_KP_DIVIDE

// function keys
#define OLDKEY_F1 MC_SCANCODE_F1
#define OLDKEY_F2 MC_SCANCODE_F2
#define OLDKEY_F3 MC_SCANCODE_F3
#define OLDKEY_F4 MC_SCANCODE_F4
#define OLDKEY_F5 MC_SCANCODE_F5
#define OLDKEY_F6 MC_SCANCODE_F6
#define OLDKEY_F7 MC_SCANCODE_F7
#define OLDKEY_F8 MC_SCANCODE_F8
#define OLDKEY_F9 MC_SCANCODE_F9
#define OLDKEY_F10 MC_SCANCODE_F10
#define OLDKEY_F11 MC_SCANCODE_F11
#define OLDKEY_F12 MC_SCANCODE_F12

// arrow keys
#define OLDKEY_LEFT MC_SCANCODE_LEFT
#define OLDKEY_RIGHT MC_SCANCODE_RIGHT
#define OLDKEY_UP MC_SCANCODE_UP
#define OLDKEY_DOWN MC_SCANCODE_DOWN

// special keys
#define OLDKEY_TAB MC_SCANCODE_TAB
#define OLDKEY_NUMPAD_ENTER MC_SCANCODE_KP_ENTER
#define OLDKEY_ENTER MC_SCANCODE_RETURN
#define OLDKEY_LSHIFT MC_SCANCODE_LSHIFT
#define OLDKEY_RSHIFT MC_SCANCODE_RSHIFT
#define OLDKEY_LCONTROL MC_SCANCODE_LCTRL
#define OLDKEY_RCONTROL MC_SCANCODE_RCTRL
#define OLDKEY_LALT MC_SCANCODE_LALT
#define OLDKEY_RALT MC_SCANCODE_RALT
#define OLDKEY_ESCAPE MC_SCANCODE_ESCAPE
#define OLDKEY_TILDE MC_SCANCODE_GRAVE
#define OLDKEY_SPACE MC_SCANCODE_SPACE
#define OLDKEY_BACKSPACE MC_SCANCODE_BACKSPACE
#define OLDKEY_END MC_SCANCODE_END
#define OLDKEY_INSERT MC_SCANCODE_INSERT
#define OLDKEY_DELETE MC_SCANCODE_DELETE
#define OLDKEY_HELP MC_SCANCODE_HELP
#define OLDKEY_HOME MC_SCANCODE_HOME
#define OLDKEY_LSUPER MC_SCANCODE_LGUI
#define OLDKEY_RSUPER MC_SCANCODE_RGUI
#define OLDKEY_PAGEUP MC_SCANCODE_PAGEUP
#define OLDKEY_PAGEDOWN MC_SCANCODE_PAGEDOWN

// media keys
#define OLDKEY_PLAY MC_SCANCODE_MEDIA_PLAY
#define OLDKEY_PAUSE MC_SCANCODE_MEDIA_PAUSE
#define OLDKEY_PLAYPAUSE MC_SCANCODE_MEDIA_PLAY_PAUSE
#define OLDKEY_STOP MC_SCANCODE_MEDIA_STOP
#define OLDKEY_PREV MC_SCANCODE_MEDIA_PREVIOUS_TRACK
#define OLDKEY_NEXT MC_SCANCODE_MEDIA_NEXT_TRACK
#define OLDKEY_MUTE MC_SCANCODE_MUTE
#define OLDKEY_VOLUMEDOWN MC_SCANCODE_VOLUMEDOWN
#define OLDKEY_VOLUMEUP MC_SCANCODE_VOLUMEUP

namespace KeyBindings {
i32 old_keycode_to_sdl_scancode(i32 key) {
    switch(key) {
#if defined(_WIN32)
        case 0x41: return OLDKEY_A;
        case 0x42: return OLDKEY_B;
        case 0x43: return OLDKEY_C;
        case 0x44: return OLDKEY_D;
        case 0x45: return OLDKEY_E;
        case 0x46: return OLDKEY_F;
        case 0x47: return OLDKEY_G;
        case 0x48: return OLDKEY_H;
        case 0x49: return OLDKEY_I;
        case 0x4A: return OLDKEY_J;
        case 0x4B: return OLDKEY_K;
        case 0x4C: return OLDKEY_L;
        case 0x4D: return OLDKEY_M;
        case 0x4E: return OLDKEY_N;
        case 0x4F: return OLDKEY_O;
        case 0x50: return OLDKEY_P;
        case 0x51: return OLDKEY_Q;
        case 0x52: return OLDKEY_R;
        case 0x53: return OLDKEY_S;
        case 0x54: return OLDKEY_T;
        case 0x55: return OLDKEY_U;
        case 0x56: return OLDKEY_V;
        case 0x57: return OLDKEY_W;
        case 0x58: return OLDKEY_X;
        case 0x59: return OLDKEY_Y;
        case 0x5A: return OLDKEY_Z;
        case 0x30: return OLDKEY_0;
        case 0x31: return OLDKEY_1;
        case 0x32: return OLDKEY_2;
        case 0x33: return OLDKEY_3;
        case 0x34: return OLDKEY_4;
        case 0x35: return OLDKEY_5;
        case 0x36: return OLDKEY_6;
        case 0x37: return OLDKEY_7;
        case 0x38: return OLDKEY_8;
        case 0x39: return OLDKEY_9;
        case VK_NUMPAD0: return OLDKEY_NUMPAD0;
        case VK_NUMPAD1: return OLDKEY_NUMPAD1;
        case VK_NUMPAD2: return OLDKEY_NUMPAD2;
        case VK_NUMPAD3: return OLDKEY_NUMPAD3;
        case VK_NUMPAD4: return OLDKEY_NUMPAD4;
        case VK_NUMPAD5: return OLDKEY_NUMPAD5;
        case VK_NUMPAD6: return OLDKEY_NUMPAD6;
        case VK_NUMPAD7: return OLDKEY_NUMPAD7;
        case VK_NUMPAD8: return OLDKEY_NUMPAD8;
        case VK_NUMPAD9: return OLDKEY_NUMPAD9;
        case VK_MULTIPLY: return OLDKEY_MULTIPLY;
        case VK_ADD: return OLDKEY_ADD;
        case VK_SEPARATOR: return OLDKEY_SEPARATOR;
        case VK_SUBTRACT: return OLDKEY_SUBTRACT;
        case VK_DECIMAL: return OLDKEY_DECIMAL;
        case VK_DIVIDE: return OLDKEY_DIVIDE;
        case VK_F1: return OLDKEY_F1;
        case VK_F2: return OLDKEY_F2;
        case VK_F3: return OLDKEY_F3;
        case VK_F4: return OLDKEY_F4;
        case VK_F5: return OLDKEY_F5;
        case VK_F6: return OLDKEY_F6;
        case VK_F7: return OLDKEY_F7;
        case VK_F8: return OLDKEY_F8;
        case VK_F9: return OLDKEY_F9;
        case VK_F10: return OLDKEY_F10;
        case VK_F11: return OLDKEY_F11;
        case VK_F12: return OLDKEY_F12;
        case VK_LEFT: return OLDKEY_LEFT;
        case VK_UP: return OLDKEY_UP;
        case VK_RIGHT: return OLDKEY_RIGHT;
        case VK_DOWN: return OLDKEY_DOWN;
        case VK_TAB: return OLDKEY_TAB;
        case VK_RETURN: return OLDKEY_ENTER;
        case VK_LSHIFT: return OLDKEY_LSHIFT;
        case VK_RSHIFT: return OLDKEY_RSHIFT;
        case VK_LCONTROL: return OLDKEY_LCONTROL;
        case VK_RCONTROL: return OLDKEY_RCONTROL;
        case VK_LMENU: return OLDKEY_LALT;
        case VK_RMENU: return OLDKEY_RALT;
        case VK_LWIN: return OLDKEY_LSUPER;
        case VK_RWIN: return OLDKEY_RSUPER;
        case VK_ESCAPE: return OLDKEY_ESCAPE;
        case VK_SPACE: return OLDKEY_SPACE;
        case VK_BACK: return OLDKEY_BACKSPACE;
        case VK_END: return OLDKEY_END;
        case VK_INSERT: return OLDKEY_INSERT;
        case VK_DELETE: return OLDKEY_DELETE;
        case VK_HELP: return OLDKEY_HELP;
        case VK_HOME: return OLDKEY_HOME;
        case VK_PRIOR: return OLDKEY_PAGEUP;
        case VK_NEXT: return OLDKEY_PAGEDOWN;
#elif defined __linux__
        case XK_A: return OLDKEY_A;
        case XK_B: return OLDKEY_B;
        case XK_C: return OLDKEY_C;
        case XK_D: return OLDKEY_D;
        case XK_E: return OLDKEY_E;
        case XK_F: return OLDKEY_F;
        case XK_G: return OLDKEY_G;
        case XK_H: return OLDKEY_H;
        case XK_I: return OLDKEY_I;
        case XK_J: return OLDKEY_J;
        case XK_K: return OLDKEY_K;
        case XK_L: return OLDKEY_L;
        case XK_M: return OLDKEY_M;
        case XK_N: return OLDKEY_N;
        case XK_O: return OLDKEY_O;
        case XK_P: return OLDKEY_P;
        case XK_Q: return OLDKEY_Q;
        case XK_R: return OLDKEY_R;
        case XK_S: return OLDKEY_S;
        case XK_T: return OLDKEY_T;
        case XK_U: return OLDKEY_U;
        case XK_V: return OLDKEY_V;
        case XK_W: return OLDKEY_W;
        case XK_X: return OLDKEY_X;
        case XK_Y: return OLDKEY_Y;
        case XK_Z: return OLDKEY_Z;
        case XK_equal: return OLDKEY_0;
        case XK_exclam: return OLDKEY_1;
        case XK_quotedbl: return OLDKEY_2;
        case XK_section: return OLDKEY_3;
        case XK_dollar: return OLDKEY_4;
        case XK_percent: return OLDKEY_5;
        case XK_ampersand: return OLDKEY_6;
        case XK_slash: return OLDKEY_7;
        case XK_quoteright: return OLDKEY_8;
        case XK_parenleft: return OLDKEY_9;
        case XK_KP_0: return OLDKEY_NUMPAD0;
        case XK_KP_1: return OLDKEY_NUMPAD1;
        case XK_KP_2: return OLDKEY_NUMPAD2;
        case XK_KP_3: return OLDKEY_NUMPAD3;
        case XK_KP_4: return OLDKEY_NUMPAD4;
        case XK_KP_5: return OLDKEY_NUMPAD5;
        case XK_KP_6: return OLDKEY_NUMPAD6;
        case XK_KP_7: return OLDKEY_NUMPAD7;
        case XK_KP_8: return OLDKEY_NUMPAD8;
        case XK_KP_9: return OLDKEY_NUMPAD9;
        case XK_KP_Multiply: return OLDKEY_MULTIPLY;
        case XK_KP_Add: return OLDKEY_ADD;
        case XK_KP_Separator: return OLDKEY_SEPARATOR;
        case XK_KP_Subtract: return OLDKEY_SUBTRACT;
        case XK_KP_Decimal: return OLDKEY_DECIMAL;
        case XK_KP_Divide: return OLDKEY_DIVIDE;
        case XK_F1: return OLDKEY_F1;
        case XK_F2: return OLDKEY_F2;
        case XK_F3: return OLDKEY_F3;
        case XK_F4: return OLDKEY_F4;
        case XK_F5: return OLDKEY_F5;
        case XK_F6: return OLDKEY_F6;
        case XK_F7: return OLDKEY_F7;
        case XK_F8: return OLDKEY_F8;
        case XK_F9: return OLDKEY_F9;
        case XK_F10: return OLDKEY_F10;
        case XK_F11: return OLDKEY_F11;
        case XK_F12: return OLDKEY_F12;
        case XK_Left: return OLDKEY_LEFT;
        case XK_Right: return OLDKEY_RIGHT;
        case XK_Up: return OLDKEY_UP;
        case XK_Down: return OLDKEY_DOWN;
        case XK_Tab: return OLDKEY_TAB;
        case XK_Return: return OLDKEY_ENTER;
        case XK_KP_Enter: return OLDKEY_NUMPAD_ENTER;
        case XK_Shift_L: return OLDKEY_LSHIFT;
        case XK_Shift_R: return OLDKEY_RSHIFT;
        case XK_Control_L: return OLDKEY_LCONTROL;
        case XK_Control_R: return OLDKEY_RCONTROL;
        case XK_Alt_L: return OLDKEY_LALT;
        case XK_Alt_R: return OLDKEY_RALT;
        case XK_Super_R: return OLDKEY_LSUPER;
        case XK_Super_L: return OLDKEY_RSUPER;
        case XK_Escape: return OLDKEY_ESCAPE;
        case XK_space: return OLDKEY_SPACE;
        case XK_BackSpace: return OLDKEY_BACKSPACE;
        case XK_End: return OLDKEY_END;
        case XK_Insert: return OLDKEY_INSERT;
        case XK_Delete: return OLDKEY_DELETE;
        case XK_Help: return OLDKEY_HELP;
        case XK_Home: return OLDKEY_HOME;
        case XK_Prior: return OLDKEY_PAGEUP;
        case XK_Next: return OLDKEY_PAGEDOWN;
#endif
    }

    return 0;
}

SCANCODE keycode_to_scancode(KEYCODE keycode) {
    return SDL_GetScancodeFromKey(keycode, nullptr);
}

KEYCODE scancode_to_keycode(SCANCODE scancode) {
    return SDL_GetKeyFromScancode((SDL_Scancode)scancode, 0, false);
}

}

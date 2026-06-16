#pragma once
// Copyright (c) 2026, kiwec, 2026, WH, All rights reserved.

#include "config.h"
#include "noinclude.h"

#include "OsuConfig.h"
#include "Vectors_fwd.h"

#include <memory>
#include <array>
#include <vector>
#include <string_view>

class CBaseUIElement;
class KeyboardEvent;
class RenderTarget;
class UIScreen;
class UIOverlay;

class AboutScreen;
class BeatmapInstallOverlay;
class Chat;
class HUD;
class Lobby;
class MainMenu;
class ModSelector;
class NotificationOverlay;
class OptionsOverlay;
class OsuDirectScreen;
class PauseOverlay;
class PromptOverlay;
class RankingScreen;
class RoomScreen;
class SongBrowser;
class SpectatorScreen;
class TooltipOverlay;
class UIUserContextMenuScreen;
class UserStatsScreen;
class VolumeOverlay;

// global for convenience, created in osu constructor, destroyed in osu constructor
struct UI;
extern UI* ui;

// UIScreens, manually created + added to the "overlays" array and destroyed in reverse order in dtor
class UIDebug;

// The screen registry: one row per always-alive screen, in STORAGE order (= construction order =
// SCREEN_NAMES order, NOT layer order). This single list derives the member fields, the typed
// getX()/getXBase() accessors, SCREEN_NAMES, SCREEN_RANK, NUM_SCREENS, the init() construction
// ladder, and the per-screen modal/closeOnScreenSwitch flags. Adding a screen = one row here.
//   Acc    : getAcc()/getAccBase() accessor names
//   Type   : concrete class -> member field type + `new Type` in init()
//   member : the typed pointer; its slot in screens[] is this row's position
//   name   : SCREEN_NAMES entry (set_active_ui_screen / ui_screens / findScreenByName)
//   rank   : bottom->top draw position, dense 0..NUM_SCREENS-1 (see SCREEN_RANK/LAYER_ORDER)
//   M / C  : bModal / bCloseOnScreenSwitch
// dummy (storage index 0, the file-local NullScreen active-screen sentinel) is the one row NOT
// here: its member type differs from its ctor type, it has no accessor, and it is built first.
// clang-format off
#define UI_SCREEN_REGISTRY(X)                                                                            \
    X(NotificationOverlay,   NotificationOverlay,     notificationOverlay,   "notificationoverlay",   20, 0, 0) \
    X(VolumeOverlay,         VolumeOverlay,           volumeOverlay,         "volumeoverlay",         19, 0, 0) \
    X(PromptOverlay,         PromptOverlay,           promptOverlay,         "promptoverlay",         16, 1, 1) \
    X(ModSelector,           ModSelector,             modSelector,           "modselector",           12, 1, 1) \
    X(UserActions,           UIUserContextMenuScreen, userActionsOverlay,    "useractions",           14, 0, 0) \
    X(Room,                  RoomScreen,              room,                  "room",                   1, 0, 0) \
    X(Chat,                  Chat,                    chatOverlay,           "chat",                  13, 0, 0) \
    X(OptionsOverlay,        OptionsOverlay,          optionsOverlay,        "optionsoverlay",        15, 0, 1) \
    X(RankingScreen,         RankingScreen,           rankingScreen,         "rankingscreen",          2, 0, 0) \
    X(UserStatsScreen,       UserStatsScreen,         userStatsScreen,       "userstatsscreen",        3, 0, 0) \
    X(SpectatorScreen,       SpectatorScreen,         spectatorScreen,       "spectatorscreen",        4, 0, 0) \
    X(PauseOverlay,          PauseOverlay,            pauseOverlay,          "pauseoverlay",          11, 1, 1) \
    X(HUD,                   HUD,                     hud,                   "hud",                   10, 0, 0) \
    X(SongBrowser,           SongBrowser,             songBrowser,           "songbrowser",            5, 0, 0) \
    X(OsuDirectScreen,       OsuDirectScreen,         osuDirectScreen,       "osudirectscreen",        6, 0, 0) \
    X(Lobby,                 Lobby,                   lobby,                 "lobby",                  7, 0, 0) \
    X(AboutScreen,           AboutScreen,             aboutScreen,           "aboutscreen",            8, 0, 0) \
    X(MainMenu,              MainMenu,                mainMenu,              "mainmenu",               9, 0, 0) \
    X(TooltipOverlay,        TooltipOverlay,          tooltipOverlay,        "tooltipoverlay",        18, 0, 0) \
    X(BeatmapInstallOverlay, BeatmapInstallOverlay,   beatmapInstallOverlay, "beatmapinstalloverlay", 17, 0, 0)
// clang-format on

struct UI final {
    NOCOPY_NOMOVE(UI)

   public:
    UI();
    ~UI();

    bool init();
    void hide();
    void show();

    void update();
    void draw();
    void onKeyDown(KeyboardEvent& key);
    void onKeyUp(KeyboardEvent& key);
    void onChar(KeyboardEvent& e);
    void onResolutionChange(vec2 newResolution);

    [[nodiscard]] inline UIScreen* getActiveScreen() const { return this->active_screen; }
    inline void setScreen(std::nullptr_t) { this->hide(); }
    void setScreen(UIScreen* screen);

    // stack query for the arrow-bound volume gesture: true while any visible screen declares
    // it needs the arrow keys for itself (UIScreen::claimsArrowKeys)
    [[nodiscard]] bool arrowKeysClaimed() const;

    // queryable with peekOverlay or removable with popOverlay
    // when pushed, the pushed overlay is set visible (but the parent is not set invisible)
    UIOverlay* pushOverlay(std::unique_ptr<UIOverlay> overlay);

    // returns false if overlay has been destroyed
    [[nodiscard]] bool peekOverlay(UIOverlay* overlay) const;

    // when popped, the parent is set visible/active
    std::unique_ptr<UIOverlay> popOverlay(UIOverlay* overlay);

    // typed accessors generated from the registry: getX() returns the concrete type (a forward
    // declaration suffices for a pointer return), getXBase() returns the UIScreen* base and is
    // defined out-of-line in UI.cpp because the derived->base pointer conversion needs the
    // complete type. (getNotificationOverlay is the one created early, in the ctor, for error
    // notifications; the rest are created in init().)
    // NOLINTBEGIN(bugprone-macro-parentheses): Type is a type name, can't be parenthesized
#define X(Acc, Type, member, name, rank, M, C)                           \
    [[nodiscard]] inline Type* get##Acc() const { return this->member; } \
    [[nodiscard]] UIScreen* get##Acc##Base() const;
    UI_SCREEN_REGISTRY(X)
#undef X
    // NOLINTEND(bugprone-macro-parentheses)

   private:
    friend UIScreen;
    friend UIDebug;  // see UIDebug.h

    // ui_validate_ticks support (debug builds): logs UITEST FAIL for screens skipped by the tick pass
    void validateTicks() const;

    // walks LAYER_ORDER top -> bottom (with extra_overlays spliced in) until consumed
    void routeKey(KeyboardEvent& e, void (CBaseUIElement::*handler)(KeyboardEvent&), std::string_view traceName);
    // draws LAYER_ORDER[from, to), skipping the active screen (drawn at the frame bottom)
    void drawLayerRange(size_t from, size_t to);
    void drawExtraOverlays();

    // dummy is the one screen not in UI_SCREEN_REGISTRY: storage index 0, a file-local NullScreen
    // (member type UIScreen* differs from its ctor type), no accessor, the active_screen sentinel,
    // built first in the ctor.
    UIScreen* dummy{nullptr};
    static constexpr const size_t EARLY_SCREENS{2};  // dummy + notificationOverlay, built in the ctor

    // one typed pointer per registry row (notificationOverlay is row 0, built early in the ctor)
#define X(Acc, Type, member, name, rank, M, C) Type* member{nullptr};  // NOLINT(bugprone-macro-parentheses)
    UI_SCREEN_REGISTRY(X)
#undef X

    // dummy + one per registry row, so adding a row updates the count automatically
#define X(...) +1  // NOLINT(bugprone-macro-parentheses)
    static constexpr size_t NUM_SCREENS{1 UI_SCREEN_REGISTRY(X)};
#undef X

    UIScreen* active_screen{nullptr};

    // "always-alive" screens
    std::array<UIScreen*, NUM_SCREENS> screens{};

    // additional overlays added by pushOverlay (owned by UI)
    std::vector<UIOverlay*> extra_overlays;

    // debugging
    std::unique_ptr<UIDebug> debuglayer{nullptr};

    // for idle cursor fade alpha
    f64 lastCursorMoveTime{0.};

    // index-aligned with screens[]/SCREEN_RANK; the lookup table for set_active_ui_screen /
    // ui_screens / findScreenByName (dummy is index 0, then one per registry row)
#define X(Acc, Type, member, name, rank, M, C) name,
    static constexpr std::array<std::string_view, NUM_SCREENS> SCREEN_NAMES{"dummy", UI_SCREEN_REGISTRY(X)};
#undef X

    // bottom -> top draw rank for each screen, index-aligned with SCREEN_NAMES/screens. layering is
    // DECLARED in the registry rows (each screen owns its rank there); this is the storage-order
    // projection of those ranks. LAYER_ORDER below is its inverse, derived; a duplicate or
    // out-of-range rank is a compile error (the permutation static_assert in UI.cpp + the constexpr
    // OOB in the inverse), never a silent mis-layer. dense 0..NUM_SCREENS-1: rank IS the bottom->top
    // walk position. (dummy is the base of the base band, rank 0.)
#define X(Acc, Type, member, name, rank, M, C) rank,
    static constexpr std::array<size_t, NUM_SCREENS> SCREEN_RANK{0, UI_SCREEN_REGISTRY(X)};
#undef X

    // canonical layer order, bottom -> top = the inverse of SCREEN_RANK (LAYER_ORDER[rank] =
    // storage index). draw walks it forward, input + key routing walk it in reverse, so
    // "input order = reverse draw order" holds by construction (one order replaces the old
    // screens-array input priority and the two hand-coded draw branches).
    // [0, OVERLAY_BAND_BEGIN) is the base band: the mutually-exclusive base screens
    // (exactly one visible, swapped by setScreen; relative order among them is inert)
    // plus hud, whose drawing belongs to the bespoke gameplay composite. the active
    // screen always draws at the bottom of the frame and is skipped by the band walk.
    // extra_overlays sit between LAYER_ORDER[EXTRAS_SPLICE - 1] and tooltip, last-pushed
    // topmost. notification sits ABOVE volume: VolumeOverlay::onKeyDown is ungated
    // (KEY_MUTE, volume binds) and must not eat keys ahead of notification's keybind
    // capture (see the decision log; the reverse of the locked draft's draw-parity pick).
    static constexpr std::array<size_t, NUM_SCREENS> LAYER_ORDER = [] {
        std::array<size_t, NUM_SCREENS> order{};
        for(size_t i = 0; i < NUM_SCREENS; ++i) order[SCREEN_RANK[i]] = i;
        return order;
    }();

    // the band boundaries are the ranks of named screens, not magic integers: change a screen's
    // SCREEN_RANK and these follow. the matching name-anchor static_asserts in UI.cpp prove the
    // derivation still lands on the intended screens. (computed in one IIFE so the name->rank
    // lookup is shared; a member helper can't be used in a sibling static-member initializer.)
    static constexpr std::array<size_t, 3> BAND_RANKS = [] {
        auto rankOf = [](std::string_view name) {
            for(size_t i = 0; i < NUM_SCREENS; ++i)
                if(SCREEN_NAMES[i] == name) return SCREEN_RANK[i];
            return NUM_SCREENS;  // unreachable for a valid name; an OOB use below is a hard compile error
        };
        return std::array<size_t, 3>{rankOf("pauseoverlay"), rankOf("optionsoverlay") + 1, rankOf("tooltipoverlay")};
    }();
    static constexpr size_t OVERLAY_BAND_BEGIN{BAND_RANKS[0]};  // first overlay-band layer above base/hud
    static constexpr size_t PLAY_OVERLAYS_END{BAND_RANKS[1]};   // one past options; pause..options render
                                                                // into the FPoSu playfield buffer in play mode
    static constexpr size_t EXTRAS_SPLICE{BAND_RANKS[2]};       // extra_overlays walk/draw below this layer
};

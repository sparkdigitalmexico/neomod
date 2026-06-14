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

    // created early, in ctor, for error notifications
    [[nodiscard]] inline NotificationOverlay* getNotificationOverlay() const { return this->notificationOverlay; }
    [[nodiscard]] UIScreen* getNotificationOverlayBase() const;

    // rest are created on init()
    [[nodiscard]] inline VolumeOverlay* getVolumeOverlay() const { return this->volumeOverlay; }
    [[nodiscard]] UIScreen* getVolumeOverlayBase() const;
    [[nodiscard]] inline PromptOverlay* getPromptOverlay() const { return this->promptOverlay; }
    [[nodiscard]] UIScreen* getPromptOverlayBase() const;
    [[nodiscard]] inline ModSelector* getModSelector() const { return this->modSelector; }
    [[nodiscard]] UIScreen* getModSelectorBase() const;
    [[nodiscard]] inline UIUserContextMenuScreen* getUserActions() const { return this->userActionsOverlay; }
    [[nodiscard]] UIScreen* getUserActionsBase() const;
    [[nodiscard]] inline RoomScreen* getRoom() const { return this->room; }
    [[nodiscard]] UIScreen* getRoomBase() const;
    [[nodiscard]] inline Chat* getChat() const { return this->chatOverlay; }
    [[nodiscard]] UIScreen* getChatBase() const;
    [[nodiscard]] inline OptionsOverlay* getOptionsOverlay() const { return this->optionsOverlay; }
    [[nodiscard]] UIScreen* getOptionsOverlayBase() const;
    [[nodiscard]] inline RankingScreen* getRankingScreen() const { return this->rankingScreen; }
    [[nodiscard]] UIScreen* getRankingScreenBase() const;
    [[nodiscard]] inline UserStatsScreen* getUserStatsScreen() const { return this->userStatsScreen; }
    [[nodiscard]] UIScreen* getUserStatsScreenBase() const;
    [[nodiscard]] inline SpectatorScreen* getSpectatorScreen() const { return this->spectatorScreen; }
    [[nodiscard]] UIScreen* getSpectatorScreenBase() const;
    [[nodiscard]] inline PauseOverlay* getPauseOverlay() const { return this->pauseOverlay; }
    [[nodiscard]] UIScreen* getPauseOverlayBase() const;
    [[nodiscard]] inline HUD* getHUD() const { return this->hud; }
    [[nodiscard]] UIScreen* getHUDBase() const;
    [[nodiscard]] inline SongBrowser* getSongBrowser() const { return this->songBrowser; }
    [[nodiscard]] UIScreen* getSongBrowserBase() const;
    [[nodiscard]] inline OsuDirectScreen* getOsuDirectScreen() const { return this->osuDirectScreen; }
    [[nodiscard]] UIScreen* getOsuDirectScreenBase() const;
    [[nodiscard]] inline Lobby* getLobby() const { return this->lobby; }
    [[nodiscard]] UIScreen* getLobbyBase() const;
    [[nodiscard]] inline AboutScreen* getAboutScreen() const { return this->aboutScreen; }
    [[nodiscard]] UIScreen* getAboutScreenBase() const;
    [[nodiscard]] inline MainMenu* getMainMenu() const { return this->mainMenu; }
    [[nodiscard]] UIScreen* getMainMenuBase() const;
    [[nodiscard]] inline TooltipOverlay* getTooltipOverlay() const { return this->tooltipOverlay; }
    [[nodiscard]] UIScreen* getTooltipOverlayBase() const;
    [[nodiscard]] inline BeatmapInstallOverlay* getBeatmapInstallOverlay() const { return this->beatmapInstallOverlay; }
    [[nodiscard]] UIScreen* getBeatmapInstallOverlayBase() const;

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

    UIScreen* dummy;
    NotificationOverlay* notificationOverlay;
    static constexpr const size_t EARLY_SCREENS{2};  // dummy+notificationOverlay

    VolumeOverlay* volumeOverlay{nullptr};
    PromptOverlay* promptOverlay{nullptr};
    ModSelector* modSelector{nullptr};
    UIUserContextMenuScreen* userActionsOverlay{nullptr};
    RoomScreen* room{nullptr};
    Chat* chatOverlay{nullptr};
    OptionsOverlay* optionsOverlay{nullptr};
    RankingScreen* rankingScreen{nullptr};
    UserStatsScreen* userStatsScreen{nullptr};
    SpectatorScreen* spectatorScreen{nullptr};
    PauseOverlay* pauseOverlay{nullptr};
    HUD* hud{nullptr};
    SongBrowser* songBrowser{nullptr};
    OsuDirectScreen* osuDirectScreen{nullptr};
    Lobby* lobby{nullptr};
    AboutScreen* aboutScreen{nullptr};
    MainMenu* mainMenu{nullptr};
    TooltipOverlay* tooltipOverlay{nullptr};
    BeatmapInstallOverlay* beatmapInstallOverlay{nullptr};

    static constexpr const size_t NUM_SCREENS{21};  // update this when adding screens

    UIScreen* active_screen{nullptr};

    // "always-alive" screens
    std::array<UIScreen*, NUM_SCREENS> screens{};

    // additional overlays added by pushOverlay (owned by UI)
    std::vector<UIOverlay*> extra_overlays;

    // debugging
    std::unique_ptr<UIDebug> debuglayer{nullptr};

    // for idle cursor fade alpha
    f64 lastCursorMoveTime{0.};

    // index-aligned with UI::screens, see the assignment order in UI::init()
    static constexpr std::array<std::string_view, NUM_SCREENS> SCREEN_NAMES{"dummy",
                                                                            "notificationoverlay",
                                                                            "volumeoverlay",
                                                                            "promptoverlay",
                                                                            "modselector",
                                                                            "useractions",
                                                                            "room",
                                                                            "chat",
                                                                            "optionsoverlay",
                                                                            "rankingscreen",
                                                                            "userstatsscreen",
                                                                            "spectatorscreen",
                                                                            "pauseoverlay",
                                                                            "hud",
                                                                            "songbrowser",
                                                                            "osudirectscreen",
                                                                            "lobby",
                                                                            "aboutscreen",
                                                                            "mainmenu",
                                                                            "tooltipoverlay",
                                                                            "beatmapinstalloverlay"};

    // canonical layer order (phase 3 layer stack): indices into screens/SCREEN_NAMES,
    // bottom -> top. draw walks it forward, input + key routing walk it in reverse, so
    // "input order = reverse draw order" holds by construction (one order replaces the
    // old screens-array input priority and the two hand-coded draw branches).
    // [0, OVERLAY_BAND_BEGIN) is the base band: the mutually-exclusive base screens
    // (exactly one visible, swapped by setScreen; relative order among them is inert)
    // plus hud, whose drawing belongs to the bespoke gameplay composite. the active
    // screen always draws at the bottom of the frame and is skipped by the band walk.
    // extra_overlays sit between LAYER_ORDER[EXTRAS_SPLICE - 1] and tooltip, last-pushed
    // topmost. notification sits ABOVE volume: VolumeOverlay::onKeyDown is ungated
    // (KEY_MUTE, volume binds) and must not eat keys ahead of notification's keybind
    // capture (see the decision log; the reverse of the locked draft's draw-parity pick).
    static constexpr std::array<size_t, NUM_SCREENS> LAYER_ORDER{
        0,   // dummy
        6,   // room
        9,   // rankingscreen
        10,  // userstatsscreen
        11,  // spectatorscreen
        14,  // songbrowser
        15,  // osudirectscreen
        16,  // lobby
        17,  // aboutscreen
        18,  // mainmenu
        13,  // hud (input slot only; drawn by the gameplay composite, not the band)
        12,  // pauseoverlay
        4,   // modselector
        7,   // chat
        5,   // useractions
        8,   // optionsoverlay
        3,   // promptoverlay
        20,  // beatmapinstalloverlay
        19,  // tooltipoverlay
        2,   // volumeoverlay
        1,   // notificationoverlay
    };
    static constexpr size_t OVERLAY_BAND_BEGIN{11};  // pause; first band layer above the base/hud
    static constexpr size_t PLAY_OVERLAYS_END{16};   // one past options; pause..options render
                                                     // into the FPoSu playfield buffer in play mode
    static constexpr size_t EXTRAS_SPLICE{18};       // extra_overlays walk/draw below this layer
};

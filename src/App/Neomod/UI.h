#pragma once
// Copyright (c) 2026, kiwec, All rights reserved.

#include "config.h"
#include "noinclude.h"

#include "OsuConfig.h"
#include "Vectors_fwd.h"

#include <memory>
#include <array>
#include <vector>
#include <string_view>

class CWindowManager;
class KeyboardEvent;
class RenderTarget;
class UIScreen;
class UIOverlay;

class Changelog;
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
    void stealFocus();

    [[nodiscard]] inline UIScreen* getActiveScreen() const { return this->active_screen; }
    inline void setScreen(std::nullptr_t) { this->hide(); }
    void setScreen(UIScreen* screen);

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
    [[nodiscard]] inline Changelog* getChangelog() const { return this->changelog; }
    [[nodiscard]] UIScreen* getChangelogBase() const;
    [[nodiscard]] inline MainMenu* getMainMenu() const { return this->mainMenu; }
    [[nodiscard]] UIScreen* getMainMenuBase() const;
    [[nodiscard]] inline TooltipOverlay* getTooltipOverlay() const { return this->tooltipOverlay; }
    [[nodiscard]] UIScreen* getTooltipOverlayBase() const;

   private:
    friend UIScreen;
    // for debugging
    void setScreenByName(std::string_view screenGetterNameWithoutGet);

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
    Changelog* changelog{nullptr};
    MainMenu* mainMenu{nullptr};
    TooltipOverlay* tooltipOverlay{nullptr};

    static constexpr const size_t NUM_SCREENS{20};  // update this when adding screens

    UIScreen* active_screen{nullptr};

    // "always-alive" screens
    std::array<UIScreen*, NUM_SCREENS> screens{};

    // additional overlays added by pushOverlay (owned by UI)
    std::vector<UIOverlay*> extra_overlays;

    // interfaces (debugging)
    // std::unique_ptr<CWindowManager> windowManager{nullptr};

    // for idle cursor fade alpha
    f64 lastCursorMoveTime{0.};
};

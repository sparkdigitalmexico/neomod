#include "UI.h"

#include "noinclude.h"
#include "Bancho.h"
#include "BeatmapInterface.h"
#include "CWindowManager.h"
#include "Engine.h"
#include "Graphics.h"
#include "ModFPoSu.h"
#include "Mouse.h"
#include "Osu.h"
#include "OsuConVars.h"
#include "RenderTarget.h"
#include "ResourceManager.h"
#include "UIScreen.h"
#include "Logging.h"
#include "SString.h"

#include "Changelog.h"
#include "Chat.h"
#include "HUD.h"
#include "Lobby.h"
#include "MainMenu.h"
#include "ModSelector.h"
#include "NotificationOverlay.h"
#include "OptionsOverlay.h"
#include "OsuDirectScreen.h"
#include "PauseOverlay.h"
#include "PromptOverlay.h"
#include "RankingScreen.h"
#include "RoomScreen.h"
#include "SongBrowser/SongBrowser.h"
#include "SpectatorScreen.h"
#include "TooltipOverlay.h"
#include "UIUserContextMenu.h"
#include "UIContextMenu.h"
#include "UserStatsScreen.h"
#include "VolumeOverlay.h"

#include <ranges>

namespace {
class NullScreen final : public UIScreen {
    NOCOPY_NOMOVE(NullScreen)
   public:
    NullScreen() : UIScreen() {}
    ~NullScreen() final = default;

    forceinline CBaseUIElement *setVisible(bool /*visible*/) final {
        this->bVisible = false;
        return this;
    }
    forceinline bool isVisible() final { return false; }
};
}  // namespace

namespace cv {
// callback only set after initialized
// for debugging
static ConVar set_active_ui_screen("set_active_ui_screen", CLIENT | NOLOAD | NOSAVE);
}  // namespace cv

void UI::setScreenByName(std::string_view screenGetterNameWithoutGet) {
    UIScreen *toSet = nullptr;
    const std::string lowerName = SString::to_lower(screenGetterNameWithoutGet);

    if(lowerName == "notificationoverlay"sv) {
        toSet = this->notificationOverlay;
    } else if(lowerName == "volumeoverlay"sv) {
        toSet = this->volumeOverlay;
    } else if(lowerName == "promptoverlay"sv) {
        toSet = this->promptOverlay;
    } else if(lowerName == "modselector"sv) {
        toSet = this->modSelector;
    } else if(lowerName == "useractions"sv) {
        toSet = this->userActionsOverlay;
    } else if(lowerName == "room"sv) {
        toSet = this->room;
    } else if(lowerName == "chat"sv) {
        toSet = this->chatOverlay;
    } else if(lowerName == "optionsoverlay"sv) {
        toSet = this->optionsOverlay;
    } else if(lowerName == "rankingscreen"sv) {
        toSet = this->rankingScreen;
    } else if(lowerName == "userstatsscreen"sv) {
        toSet = this->userStatsScreen;
    } else if(lowerName == "spectatorscreen"sv) {
        toSet = this->spectatorScreen;
    } else if(lowerName == "pauseoverlay"sv) {
        toSet = this->pauseOverlay;
    } else if(lowerName == "hud"sv) {
        toSet = this->hud;
    } else if(lowerName == "songbrowser"sv) {
        toSet = this->songBrowser;
    } else if(lowerName == "osudirectscreen"sv) {
        toSet = this->osuDirectScreen;
    } else if(lowerName == "lobby"sv) {
        toSet = this->lobby;
    } else if(lowerName == "changelog"sv) {
        toSet = this->changelog;
    } else if(lowerName == "mainmenu"sv) {
        toSet = this->mainMenu;
    } else if(lowerName == "tooltipoverlay"sv) {
        toSet = this->tooltipOverlay;
    }

    if(toSet) {
        this->setScreen(toSet);
    } else {
        debugLog("Invalid screen {}", screenGetterNameWithoutGet);
    }
}

UIScreen *UI::getNotificationOverlayBase() const { return this->notificationOverlay; }
UIScreen *UI::getVolumeOverlayBase() const { return this->volumeOverlay; }
UIScreen *UI::getPromptOverlayBase() const { return this->promptOverlay; }
UIScreen *UI::getModSelectorBase() const { return this->modSelector; }
UIScreen *UI::getUserActionsBase() const { return this->userActionsOverlay; }
UIScreen *UI::getRoomBase() const { return this->room; }
UIScreen *UI::getChatBase() const { return this->chatOverlay; }
UIScreen *UI::getOptionsOverlayBase() const { return this->optionsOverlay; }
UIScreen *UI::getRankingScreenBase() const { return this->rankingScreen; }
UIScreen *UI::getUserStatsScreenBase() const { return this->userStatsScreen; }
UIScreen *UI::getSpectatorScreenBase() const { return this->spectatorScreen; }
UIScreen *UI::getPauseOverlayBase() const { return this->pauseOverlay; }
UIScreen *UI::getHUDBase() const { return this->hud; }
UIScreen *UI::getSongBrowserBase() const { return this->songBrowser; }
UIScreen *UI::getOsuDirectScreenBase() const { return this->osuDirectScreen; }
UIScreen *UI::getLobbyBase() const { return this->lobby; }
UIScreen *UI::getChangelogBase() const { return this->changelog; }
UIScreen *UI::getMainMenuBase() const { return this->mainMenu; }
UIScreen *UI::getTooltipOverlayBase() const { return this->tooltipOverlay; }

UI *ui{nullptr};

UI::UI() {
    ui = this;
    this->screens[0] = this->active_screen = this->dummy = new NullScreen();
    this->screens[1] = this->notificationOverlay = new NotificationOverlay();
}

UI::~UI() {
    cv::set_active_ui_screen.removeAllCallbacks();

    for(auto *overlay : this->extra_overlays) {
        SAFE_DELETE(overlay);
    }
    this->extra_overlays.clear();

    // destroy screens in reverse order
    for(auto &screen : this->screens | std::views::reverse) {
        SAFE_DELETE(screen);
    }
    this->screens = {};
    // ui = nullptr in ~Osu
}

bool UI::init() {
    int screenit = EARLY_SCREENS;
    this->screens[screenit++] = this->volumeOverlay = new VolumeOverlay();
    this->screens[screenit++] = this->promptOverlay = new PromptOverlay();
    this->screens[screenit++] = this->modSelector = new ModSelector();
    this->screens[screenit++] = this->userActionsOverlay = new UIUserContextMenuScreen();
    this->screens[screenit++] = this->room = new RoomScreen();
    this->screens[screenit++] = this->chatOverlay = new Chat();
    this->screens[screenit++] = this->optionsOverlay = new OptionsOverlay();
    this->screens[screenit++] = this->rankingScreen = new RankingScreen();
    this->screens[screenit++] = this->userStatsScreen = new UserStatsScreen();
    this->screens[screenit++] = this->spectatorScreen = new SpectatorScreen();
    this->screens[screenit++] = this->pauseOverlay = new PauseOverlay();
    this->screens[screenit++] = this->hud = new HUD();
    this->screens[screenit++] = this->songBrowser = new SongBrowser();
    this->screens[screenit++] = this->osuDirectScreen = new OsuDirectScreen();
    this->screens[screenit++] = this->lobby = new Lobby();
    this->screens[screenit++] = this->changelog = new Changelog();
    this->screens[screenit++] = this->mainMenu = new MainMenu();
    this->screens[screenit++] = this->tooltipOverlay = new TooltipOverlay();
    assert(screenit == NUM_SCREENS);

    this->notificationOverlay->addKeyListener(this->optionsOverlay);

    this->active_screen = Osu::isKioskMode() ? static_cast<UIScreen *>(this->dummy) : this->mainMenu;

    // debug
    // this->windowManager = std::make_unique<CWindowManager>();
    cv::set_active_ui_screen.setCallback(SA::MakeDelegate<&UI::setScreenByName>(this));

    return true;
}

void UI::update() {
    CBaseUIEventCtx c;

    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->update(c);
    }

    bool updated_active_screen = false;
    for(auto *screen : this->screens) {
        screen->update(c);
        if(screen == this->active_screen) updated_active_screen = true;
        if(c.mouse_consumed()) break;  // TODO: update() does more than only mouse event handling, should be decoupled
    }

    if(!updated_active_screen && !c.mouse_consumed()) {
        this->active_screen->update(c);
    }
}

void UI::draw() {
    // if we are not using the native window resolution, draw into the buffer
    const bool isBufferedDraw = (g->getResolution() != osu->getVirtScreenSize());
    if(isBufferedDraw) {
        osu->backBuffer->enable();
    }

    // draw active screen first, any "overlays" after
    this->active_screen->draw();

    // draw any extra overlays (TODO: draw order, this shouldn't be hardcoded at the start)
    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->draw();
    }

    f32 cursorAlpha = 1.f;
    if(f32 cursorIdleFadeTime = cv::cursor_idle_time_before_fade.getFloat(); cursorIdleFadeTime >= 0.f) {
        // fade out cursor if it hasn't been moved for over 15 seconds
        if(mouse->getPos() != this->lastCursorPos) {
            this->lastCursorMoveTime = engine->getTime();
            this->lastCursorPos = mouse->getPos();
        } else {
            const f32 fadeDuration = 1.f;
            const f32 idleTime = (f32)(engine->getTime() - this->lastCursorMoveTime);
            cursorAlpha *=
                std::clamp<f32>(cv::cursor_idle_time_before_fade.getFloat() - idleTime + fadeDuration, 0.f, 1.f);
        }
    }

    const bool isFPoSu = (cv::mod_fposu.getBool());

    // draw everything in the correct order
    if(osu->isInPlayMode()) {  // if we are playing a beatmap
        if(isFPoSu) osu->playfieldBuffer->enable();

        // draw playfield (incl. flashlight/smoke etc.)
        osu->map_iface->draw();

        if(!isFPoSu) this->hud->draw();

        // quick retry fadeout overlay
        if(osu->fQuickRetryTime != 0.0f && osu->bQuickRetryDown) {
            float alphaPercent = 1.0f - (osu->fQuickRetryTime - engine->getTime()) / cv::quick_retry_delay.getFloat();
            if(engine->getTime() > osu->fQuickRetryTime) alphaPercent = 1.0f;

            g->setColor(argb((int)(255 * alphaPercent), 0, 0, 0));
            g->fillRect(0, 0, osu->getVirtScreenWidth(), osu->getVirtScreenHeight());
        }

        this->pauseOverlay->draw();
        this->modSelector->draw();
        this->chatOverlay->draw();
        this->userActionsOverlay->draw();
        this->optionsOverlay->draw();

        if(!isFPoSu) {
            this->hud->drawFps();
        }

        // this->windowManager->draw();

        if(isFPoSu && cv::draw_cursor_ripples.getBool()) {
            this->hud->drawCursorRipples();
        }

        // apply fading cursor alpha multiplier
        if(!(this->pauseOverlay->isVisible() || osu->map_iface->isContinueScheduled() ||
             !cv::mod_fadingcursor.getBool())) {
            cursorAlpha *=
                1.0f -
                std::clamp<float>((float)osu->score->getCombo() / cv::mod_fadingcursor_combo.getFloat(), 0.0f, 1.0f);
        }

        // draw FPoSu cursor trail
        if(isFPoSu && cv::fposu_draw_cursor_trail.getBool()) {
            const vec2 trailpos = osu->map_iface->isPaused() ? mouse->getPos() : osu->map_iface->getCursorPos();
            this->hud->drawCursorTrail(trailpos, cursorAlpha);
        }

        if(isFPoSu) {
            osu->playfieldBuffer->disable();
            osu->fposu->draw();
            this->hud->draw();
            this->hud->drawFps();
        }

        // draw debug info on top of everything else
        if(cv::debug_draw_timingpoints.getBool()) osu->map_iface->drawDebug();

    } else {  // if we are not playing

        this->chatOverlay->draw();
        this->userActionsOverlay->draw();
        this->optionsOverlay->draw();
        this->promptOverlay->draw();

        this->hud->drawFps();

        // this->windowManager->draw();
    }

    this->tooltipOverlay->draw();
    this->notificationOverlay->draw();
    this->volumeOverlay->draw();

    // loading spinner for some async tasks
    if((osu->bSkinLoadScheduled && osu->getSkin() != osu->skinScheduledToLoad)) {
        this->hud->drawLoadingSmall("");
    }

    if(osu->isInPlayMode()) {
        // draw cursor (gameplay)
        const bool paused = osu->map_iface->isPaused();
        const vec2 cursorPos =
            isFPoSu ? (osu->getVirtScreenSize() / 2.0f) : (paused ? mouse->getPos() : osu->map_iface->getCursorPos());
        const bool drawSecondTrail = !paused && (cv::mod_autoplay.getBool() || cv::mod_autopilot.getBool() ||
                                                 osu->map_iface->is_watching || BanchoState::spectating);
        const bool updateAndDrawTrail = !isFPoSu;
        this->hud->drawCursor(cursorPos, cursorAlpha, drawSecondTrail, updateAndDrawTrail);
    } else {
        // draw cursor (menus)
        this->hud->drawCursor(mouse->getPos(), cursorAlpha);
    }

    // draw build info on top of everything else
    HUD::drawRuntimeInfo();

    // if we are not using the native window resolution
    if(isBufferedDraw) {
        // draw a scaled version from the buffer to the screen
        osu->backBuffer->disable();

        const vec2 offset{vec2{g->getResolution() - osu->getVirtScreenSize()} * 0.5f};
        g->setBlending(false);
        if(cv::letterboxing.getBool()) {
            osu->backBuffer->draw(offset.x * (1.0f + cv::letterboxing_offset_x.getFloat()),
                                  offset.y * (1.0f + cv::letterboxing_offset_y.getFloat()), osu->getVirtScreenWidth(),
                                  osu->getVirtScreenHeight());
        } else {
            if(cv::resolution_keep_aspect_ratio.getBool()) {
                const float scale = Osu::getRectScaleToFitResolution(osu->backBuffer->getSize(), g->getResolution());
                const float scaledWidth = osu->backBuffer->getWidth() * scale;
                const float scaledHeight = osu->backBuffer->getHeight() * scale;
                osu->backBuffer->draw(std::max(0.0f, g->getResolution().x / 2.0f - scaledWidth / 2.0f) *
                                          (1.0f + cv::letterboxing_offset_x.getFloat()),
                                      std::max(0.0f, g->getResolution().y / 2.0f - scaledHeight / 2.0f) *
                                          (1.0f + cv::letterboxing_offset_y.getFloat()),
                                      scaledWidth, scaledHeight);
            } else {
                osu->backBuffer->draw(0, 0, g->getResolution().x, g->getResolution().y);
            }
        }
        g->setBlending(true);
    }
}

void UI::onKeyDown(KeyboardEvent &key) {
    if(key.isConsumed()) return;

    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->onKeyDown(key);
        if(key.isConsumed()) return;
    }

    for(auto *screen : this->screens) {
        screen->onKeyDown(key);
        if(key.isConsumed()) return;
    }
}

void UI::onKeyUp(KeyboardEvent &key) {
    if(key.isConsumed()) return;

    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->onKeyUp(key);
        if(key.isConsumed()) return;
    }

    for(auto *screen : this->screens) {
        screen->onKeyUp(key);
        if(key.isConsumed()) return;
    }
}

void UI::onChar(KeyboardEvent &e) {
    if(e.isConsumed()) return;

    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->onChar(e);
        if(e.isConsumed()) return;
    }

    for(auto *screen : this->screens) {
        screen->onChar(e);
        if(e.isConsumed()) return;
    }
}

void UI::onResolutionChange(vec2 newResolution) {
    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->onResolutionChange(newResolution);
    }

    for(auto *screen : this->screens) {
        screen->onResolutionChange(newResolution);
    }
}

void UI::stealFocus() {
    // NOLINTNEXTLINE(modernize-loop-convert)
    for(uSz i = 0; i < this->extra_overlays.size(); ++i) {
        this->extra_overlays[i]->stealFocus();
    }

    for(auto *screen : this->screens) {
        screen->stealFocus();
    }
}

void UI::hide() {
    this->active_screen->setVisible(false);
    // Close any "temporary" overlays
    this->promptOverlay->setVisible(false);
    this->optionsOverlay->setVisible(false);
    this->pauseOverlay->setVisible(false);
}

void UI::show() { this->active_screen->setVisible(true); }

void UI::setScreen(UIScreen *screen) {
    assert(screen);

    if(screen != this->active_screen && this->active_screen->isVisible()) {
        this->hide();
    } else {
        // HACK: close "temporary" overlays unconditionally (we do not set pauseOverlay as the active screen ever)
        if(screen != this->pauseOverlay && this->pauseOverlay->isVisible()) {
            this->pauseOverlay->setVisible(false);
        }
        if(screen != this->optionsOverlay && this->optionsOverlay->isVisible()) {
            this->optionsOverlay->setVisible(false);
        }
        if(screen != this->promptOverlay && this->promptOverlay->isVisible()) {
            this->promptOverlay->setVisible(false);
        }
    }

    this->active_screen = screen;
    this->show();
}

UIOverlay *UI::pushOverlay(std::unique_ptr<UIOverlay> overlay) {
    assert(overlay);

    UIOverlay *raw = overlay.release();
    assert(!std::ranges::contains(this->extra_overlays, raw));
    this->extra_overlays.push_back(raw);

    // set the overlay visible immediately
    raw->setVisible(true);
    return raw;
}

bool UI::peekOverlay(UIOverlay *overlay) const {
    if(auto it = std::ranges::find(this->extra_overlays, overlay); it != this->extra_overlays.end()) {
        return true;
    }
    return false;
}

std::unique_ptr<UIOverlay> UI::popOverlay(UIOverlay *overlay) {
    if(auto it = std::ranges::find(this->extra_overlays, overlay); it != this->extra_overlays.end()) {
        std::unique_ptr<UIOverlay> overlay_out;
        overlay_out.reset(*it);

        // remove it
        this->extra_overlays.erase(it);
        UIScreen *parent = overlay->getParent();
        this->setScreen(parent);

        return overlay_out;
    } else {
        assert(false && "UI::popOverlay: double-popped overlay");
    }
    std::unreachable();
}

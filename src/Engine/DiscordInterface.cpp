// Copyright (c) 2018, PG, All rights reserved.
#include "DiscordInterface.h"

#ifndef MCENGINE_FEATURE_DISCORD
namespace DiscRPC {
void init() {}
void deinit() {}
void tick() {}
void destroy() {}
void clear_activity() {}
void set_activity(struct DiscordActivity* /*activity*/) {}
struct DiscordActivity create_base_activity() { return DiscordActivity{}; }
}  // namespace DiscRPC

#else

#include <discord_rpc.h>

#include "ConVar.h"
#include "Engine.h"
#include "Logging.h"

#define DISCORD_CLIENT_ID "1474141308183380181"

namespace cv {
static ConVar debug_discord_rpc("debug_discord_rpc", false, CLIENT, "print verbose discord rpc activity details");
}

namespace DiscRPC {
namespace  // static
{

static bool initialized{false};

static void on_ready(const DiscordUser* user) {
    if(user && user->username) {
        logRaw("[Discord] connected as {:s}", user->username);
    } else {
        logRaw("[Discord] connected");
    }
}

static void on_disconnected(int errorCode, const char* message) {
    logRaw("[Discord] disconnected ({:d}): {:s}", errorCode, message ? message : "");
}

static void on_errored(int errorCode, const char* message) {
    logRaw("[Discord] ERROR ({:d}): {:s}", errorCode, message ? message : "");
}

}  // namespace

void init() {
    if(initialized) return;

    // TODOs:
    // - set up more event handlers
    // - allow spectate/join lobby/invite etc. handlers
    DiscordEventHandlers handlers{};
    handlers.ready = on_ready;
    handlers.disconnected = on_disconnected;
    handlers.errored = on_errored;
    Discord_Initialize(DISCORD_CLIENT_ID, &handlers, /*autoRegister=*/0, /*optionalSteamId=*/nullptr);
    initialized = true;

    // there's an issue where if the game starts with discord closed, then the SDK fails to initialize, and there's
    // no way to try reinitializing it (without restarting the game)
    // so allow "turning it off and on again" to try reinitializing
    // (DiscRPC::init() does nothing if already initialized)
    if(!cv::rich_presence.hasSingleArgCallback()) {
        cv::rich_presence.setCallback(
            [](float newValue) -> void { return !!static_cast<int>(newValue) ? DiscRPC::init() : DiscRPC::deinit(); });
    }
}

void tick() {
    if(!initialized) return;
    Discord_RunCallbacks();
}

void deinit() {
    if(initialized) {
        Discord_Shutdown();
        initialized = false;
    }
}

void destroy() { deinit(); }

void clear_activity() {
    if(!initialized) return;
    Discord_ClearPresence();
}

struct DiscordActivity create_base_activity() {
    struct DiscordActivity activity{};
    strcpy(&activity.assets.large_image[0], PACKAGE_NAME "_icon");
    strcpy(&activity.assets.small_image[0], "None");
    return activity;
}

void set_activity(struct DiscordActivity* activity) {
    if(!initialized) return;
    if(!cv::rich_presence.getBool()) return;

    if(cv::debug_discord_rpc.getBool()) {
        std::string dbgstr{fmt::format("DISCORD PRESENCE\n")};
        dbgstr.append(fmt::format("state: {:s}\n", std::string_view{&activity->state[0]}));
        dbgstr.append(fmt::format("details: {:s}\n", std::string_view{&activity->details[0]}));
        dbgstr.append(fmt::format("timestamps: {{ start: {:d}, end: {:d} }}\n", activity->timestamps.start,
                                  activity->timestamps.end));
        dbgstr.append(fmt::format(
            "assets: {{ large_image: {:s}, large_text: {:s}, small_image: {:s}, small_text: {:s} }}",
            std::string_view{&activity->assets.large_image[0]}, std::string_view{&activity->assets.large_text[0]},
            std::string_view{&activity->assets.small_image[0]}, std::string_view{&activity->assets.small_text[0]}));
        dbgstr.append(fmt::format("\nparty: {{ id: {:s}, size: {{ current_size: {:d}, max_size: {:d} }} }}",
                                  std::string_view{&activity->party.id[0]}, activity->party.size.current_size,
                                  activity->party.size.max_size));
        logRaw(dbgstr);
    }

    DiscordRichPresence presence{};
    presence.state = &activity->state[0];
    presence.details = &activity->details[0];
    presence.startTimestamp = activity->timestamps.start;
    presence.endTimestamp = activity->timestamps.end;
    presence.largeImageKey = &activity->assets.large_image[0];
    presence.largeImageText = &activity->assets.large_text[0];
    presence.smallImageKey = &activity->assets.small_image[0];
    presence.smallImageText = &activity->assets.small_text[0];
    presence.partyId = &activity->party.id[0];
    presence.partySize = activity->party.size.current_size;
    presence.partyMax = activity->party.size.max_size;

    Discord_UpdatePresence(&presence);
}

}  // namespace DiscRPC

#endif

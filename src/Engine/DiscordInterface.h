#pragma once
// Copyright (c) 2018, PG, All rights reserved.

#include <cstdint>

#include "config.h"

// shim activity struct: we own the buffers, the impl translates to the underlying lib's struct.
// kept independent of any external header so this compiles whether or not Discord is enabled.
struct DiscordActivity {
    char state[128]{};
    char details[128]{};
    struct {
        int64_t start{0};
        int64_t end{0};
    } timestamps{};
    struct {
        char large_image[256]{};
        char large_text[128]{};
        char small_image[256]{};
        char small_text[128]{};
    } assets{};
    struct {
        char id[128]{};
        struct {
            int current_size{0};
            int max_size{0};
        } size{};
    } party{};
};

namespace DiscRPC {
void init();
void deinit();
void tick();
void destroy();
void clear_activity();
void set_activity(struct DiscordActivity *activity);
struct DiscordActivity create_base_activity();  // base init
}  // namespace DiscRPC

#pragma once
// Copyright (c) 2018, PG, All rights reserved.

#include "config.h"

#include <cstdint>
#include <array>

// shim activity struct: we own the buffers, the impl translates to the underlying lib's struct.
// kept independent of any external header so this compiles whether or not Discord is enabled.
struct DiscordActivity {
    std::array<char, 128> state{};
    std::array<char, 128> details{};
    struct {
        int64_t start{0};
        int64_t end{0};
    } timestamps{};
    struct {
        std::array<char, 256> large_image{};
        std::array<char, 128> large_text{};
        std::array<char, 256> small_image{};
        std::array<char, 128> small_text{};
    } assets{};
    struct {
        std::array<char, 128> id{};
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
void set_activity(const DiscordActivity &activity);
DiscordActivity create_base_activity();  // base init
}  // namespace DiscRPC

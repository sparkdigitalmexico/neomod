// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "types.h"

#include <string_view>

namespace UniString {
[[nodiscard]] uSz num_codepoints(std::string_view utf8);
[[nodiscard]] uSz num_codepoints(std::u16string_view utf16);
[[nodiscard]] uSz num_codepoints(std::u32string_view utf32);

[[nodiscard]] std::string to_utf8(const char *arbitrarily_encoded_data, uSz size);
[[nodiscard]] std::string to_utf8(std::string_view maybe_utf8);
[[nodiscard]] std::string to_utf8(std::u16string_view utf16);
[[nodiscard]] std::string to_utf8(std::u32string_view utf32);

[[nodiscard]] std::u16string to_utf16(std::string_view utf8);
[[nodiscard]] std::u16string to_utf16(std::u32string_view utf32);

[[nodiscard]] std::u32string to_utf32(std::string_view utf8);
[[nodiscard]] std::u32string to_utf32(std::u16string_view utf16);

[[nodiscard]] std::string to_utf8(std::wstring_view wide);
[[nodiscard]] std::wstring to_wide(std::string_view utf8);

// codepoint iteration (input must be valid)
class u8codepoint_view {
    std::string_view m_sv;
   public:
    explicit u8codepoint_view(std::string_view sv);
    struct iterator {
        const u8 *pos;
        char32_t operator*() const;
        iterator &operator++();
        [[nodiscard]] bool operator==(const iterator &o) const;
    };
    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;
};

class u16codepoint_view {
    std::u16string_view m_sv;
   public:
    explicit u16codepoint_view(std::u16string_view sv);
    struct iterator {
        const char16_t *pos;
        char32_t operator*() const;
        iterator &operator++();
        [[nodiscard]] bool operator==(const iterator &o) const;
    };
    [[nodiscard]] iterator begin() const;
    [[nodiscard]] iterator end() const;
};

// assumes valid utf-8
[[nodiscard]] u8codepoint_view codepoints(std::string_view sv);
// assumes valid utf-16
[[nodiscard]] u16codepoint_view codepoints(std::u16string_view sv);

// byte offset of the start of the previous codepoint (assumes valid utf-8)
[[nodiscard]] uSz prev(std::string_view sv, uSz pos);

// byte offset past the end of the current codepoint (assumes valid utf-8)
[[nodiscard]] uSz next(std::string_view sv, uSz pos);

}  // namespace UniString

// Copyright (c) 2026, WH, All rights reserved.
#pragma once

#include "types.h"

#include <string_view>

namespace UniString {
[[nodiscard]] uSz num_codepoints(std::string_view utf8);
[[nodiscard]] uSz num_codepoints(std::u16string_view utf16);
[[nodiscard]] uSz num_codepoints(std::u32string_view utf32);

[[nodiscard]] std::string to_utf8(const u8* arbitrarily_encoded_data, uSz size);
[[nodiscard]] std::string to_utf8(std::string_view maybe_utf8);
[[nodiscard]] std::string to_utf8(std::u16string_view utf16);
[[nodiscard]] std::string to_utf8(std::u32string_view utf32);

[[nodiscard]] std::u16string to_utf16(std::string_view utf8);
[[nodiscard]] std::u16string to_utf16(std::u32string_view utf32);

[[nodiscard]] std::u32string to_utf32(std::string_view utf8);
[[nodiscard]] std::u32string to_utf32(std::u16string_view utf16);

}  // namespace UniString

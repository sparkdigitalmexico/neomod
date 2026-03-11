// Copyright (c) 2026, WH, All rights reserved.
#include "UniString.h"
#include "noinclude.h"

#include "simdutf.h"

#include <string>
#include <cstring>
#include <bit>
#include <cassert>
#include <utility>
#include <new>
#include <type_traits>

namespace UniString {

uSz num_codepoints(std::string_view utf8) {
    if(utf8.empty()) return 0;
    return simdutf::count_utf8(utf8);
}

uSz num_codepoints(std::u16string_view utf16) {
    if(utf16.empty()) return 0;
    return simdutf::count_utf16le(utf16);
}

uSz num_codepoints(std::u32string_view utf32) { return utf32.size(); }

namespace {
struct AlignedBuffer {
    NOCOPY_NOMOVE(AlignedBuffer)
   private:
    static constexpr std::align_val_t alignment{16};

   public:
    AlignedBuffer() = delete;
    // NOLINTNEXTLINE
    AlignedBuffer(const u8 *src, uSz size, bool force_copy) {
        if(force_copy || reinterpret_cast<uintptr_t>(src) % static_cast<uSz>(alignment) != 0) {
            if(size <= sizeof(small)) {
                data = &small[0];
            } else {
                data = static_cast<u8 *>(::operator new(size, alignment));
                should_delete = true;
            }
            std::memcpy(static_cast<void *>(data), static_cast<const void *>(src), size);
        } else {
            data = const_cast<u8 *>(src);  // NOLINT
        }
    }
    ~AlignedBuffer() {
        if(should_delete) {
            ::operator delete(static_cast<void *>(data), alignment);
        }
    }

    template <typename T>
    [[nodiscard]] const T *get() const {
        return reinterpret_cast<const T *>(data);
    }

   private:
    alignas(static_cast<uSz>(alignment)) u8 small[128];
    u8 *data;
    bool should_delete{false};
};

inline std::pair<uSz, simdutf::encoding_type> get_bom_and_encoding(const u8 *data, uSz size) {
    uSz bom_size = 0;

    // check up to 4 bytes, since a UTF-32 BOM is 4 bytes
    simdutf::encoding_type detected = simdutf::BOM::check_bom(data, std::min<uSz>(4, size));

    if(detected != simdutf::encoding_type::unspecified) {
        // remove BOM from conversion
        bom_size = simdutf::BOM::bom_byte_size(detected);
        // sanity
        assert(bom_size <= size);
    } else {
        // if there was no BOM, autodetect encoding
        detected = simdutf::autodetect_encoding(data, size);
    }
    return std::pair{bom_size, detected};
}

}  // namespace

std::string to_utf8(const char *arbitrarily_encoded_data, uSz size) {
    if(unlikely(!arbitrarily_encoded_data || size == 0)) return {};

    // detect encoding with BOM support
    const auto [bom_pfx_bytes, detected] =
        get_bom_and_encoding(reinterpret_cast<const u8 *>(arbitrarily_encoded_data), size);

    const uSz in_bytes = size - bom_pfx_bytes;
    if(in_bytes == 0) {
        return {};
    }

    const u8 *src_start = &(reinterpret_cast<const u8 *>(arbitrarily_encoded_data)[bom_pfx_bytes]);

    std::string ret;
    if(detected == simdutf::encoding_type::unspecified || detected == simdutf::encoding_type::UTF8) {
        ret.assign(reinterpret_cast<const char *>(src_start), in_bytes);
        return ret;
    }

    // data must be aligned (because of simd)
    const AlignedBuffer buf{src_start, in_bytes, /*force_copy=*/detected == simdutf::encoding_type::UTF32_BE};

    switch(detected) {
        case simdutf::encoding_type::UTF16_LE: {
            const uSz in_u16_len = in_bytes / 2;
            const uSz out_u8_len = simdutf::utf8_length_from_utf16le(buf.get<char16_t>(), in_u16_len);
            ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
                return simdutf::convert_utf16le_to_utf8(buf.get<char16_t>(), in_u16_len, data);
            });
        } break;

        case simdutf::encoding_type::UTF16_BE: {
            const uSz in_u16_len = in_bytes / 2;
            const uSz out_u8_len = simdutf::utf8_length_from_utf16be(buf.get<char16_t>(), in_u16_len);
            ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
                return simdutf::convert_utf16be_to_utf8(buf.get<char16_t>(), in_u16_len, data);
            });
        } break;

        case simdutf::encoding_type::UTF32_LE: {
            const uSz in_u32_len = in_bytes / 4;
            const uSz out_u8_len = simdutf::utf8_length_from_utf32(buf.get<char32_t>(), in_u32_len);
            ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
                return simdutf::convert_utf32_to_utf8(buf.get<char32_t>(), in_u32_len, data);
            });
        } break;

        case simdutf::encoding_type::UTF32_BE: {
            // simdutf has no UTF-32 endianness swap or direct BE->UTF-8 conversion,
            // so byte-swap to native order first. should be rare anyways
            const uSz in_u32_len = in_bytes / 4;
            // NOLINTNEXTLINE
            auto *native = const_cast<char32_t *>(buf.get<char32_t>());
            for(uSz i = 0; i < in_u32_len; i++) native[i] = std::byteswap(native[i]);
            const uSz out_u8_len = simdutf::utf8_length_from_utf32(native, in_u32_len);
            ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
                return simdutf::convert_utf32_to_utf8(native, in_u32_len, data);
            });
        } break;

        case simdutf::encoding_type::unspecified:
        case simdutf::encoding_type::UTF8:
            // already handled above
        case simdutf::encoding_type::Latin1:
            /* ... the function might return simdutf::encoding_type::UTF8,
            * simdutf::encoding_type::UTF16_LE, simdutf::encoding_type::UTF16_BE, or
            * simdutf::encoding_type::UTF32_LE.
            */
            std::unreachable();
            break;
    }

    return ret;
}

std::string to_utf8(std::string_view maybe_utf8) { return to_utf8(maybe_utf8.data(), maybe_utf8.size()); }

std::string to_utf8(std::u16string_view utf16) {
    if(utf16.empty()) return {};
    std::string ret;
    const uSz utf8Length = simdutf::utf8_length_from_utf16le(utf16.data(), utf16.size());
    ret.resize_and_overwrite(utf8Length, [&](char *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf16le_to_utf8(utf16.data(), utf16.size(), data);
    });
    return ret;
}

std::string to_utf8(std::u32string_view utf32) {
    if(utf32.empty()) return {};
    std::string ret;
    const uSz utf8Length = simdutf::utf8_length_from_utf32(utf32.data(), utf32.size());
    ret.resize_and_overwrite(utf8Length, [&](char *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf32_to_utf8(utf32.data(), utf32.size(), data);
    });
    return ret;
}

std::u16string to_utf16(std::string_view utf8) {
    if(utf8.empty()) return {};
    std::u16string ret;
    const uSz out_u16_len = simdutf::utf16_length_from_utf8(utf8.data(), utf8.size());
    ret.resize_and_overwrite(out_u16_len, [&](char16_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf8_to_utf16le(utf8.data(), utf8.size(), data);
    });
    return ret;
}

std::u16string to_utf16(std::u32string_view utf32) {
    if(utf32.empty()) return {};
    std::u16string ret;
    const uSz out_u16_len = simdutf::utf16_length_from_utf32(utf32.data(), utf32.size());
    ret.resize_and_overwrite(out_u16_len, [&](char16_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf32_to_utf16le(utf32.data(), utf32.size(), data);
    });
    return ret;
}

std::u32string to_utf32(std::string_view utf8) {
    if(utf8.empty()) return {};
    std::u32string ret;
    const uSz out_u32_len = simdutf::utf32_length_from_utf8(utf8.data(), utf8.size());
    ret.resize_and_overwrite(out_u32_len, [&](char32_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf8_to_utf32(utf8.data(), utf8.size(), data);
    });
    return ret;
}

std::u32string to_utf32(std::u16string_view utf16) {
    if(utf16.empty()) return {};
    std::u32string ret;
    const uSz out_u32_len = simdutf::utf32_length_from_utf16le(utf16.data(), utf16.size());
    ret.resize_and_overwrite(out_u32_len, [&](char32_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf16le_to_utf32(utf16.data(), utf16.size(), data);
    });
    return ret;
}

std::string to_utf8(std::wstring_view wide) {
    if(wide.empty()) return {};
    std::string ret;
#if WCHAR_MAX <= 0xFFFF
    const uSz out_u8_len =
        simdutf::utf8_length_from_utf16(reinterpret_cast<const char16_t *>(wide.data()), wide.size());
    ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf16_to_utf8(reinterpret_cast<const char16_t *>(wide.data()), wide.size(), data);
    });
#else
    const uSz out_u8_len =
        simdutf::utf8_length_from_utf32(reinterpret_cast<const char32_t *>(wide.data()), wide.size());
    ret.resize_and_overwrite(out_u8_len, [&](char *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf32_to_utf8(reinterpret_cast<const char32_t *>(wide.data()), wide.size(), data);
    });
#endif
    return ret;
}

std::wstring to_wide(std::string_view utf8) {
    if(utf8.empty()) return {};
    std::wstring ret;
#if WCHAR_MAX <= 0xFFFF
    const uSz out_u16_len = simdutf::utf16_length_from_utf8(utf8.data(), utf8.size());
    ret.resize_and_overwrite(out_u16_len, [&](wchar_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf8_to_utf16(utf8.data(), utf8.size(), reinterpret_cast<char16_t *>(data));
    });
#else
    const uSz out_u32_len = simdutf::utf32_length_from_utf8(utf8.data(), utf8.size());
    ret.resize_and_overwrite(out_u32_len, [&](wchar_t *data, uSz /* size */) -> uSz {
        return simdutf::convert_utf8_to_utf32(utf8.data(), utf8.size(), reinterpret_cast<char32_t *>(data));
    });
#endif
    return ret;
}

u8codepoint_view::u8codepoint_view(std::string_view sv) : m_sv(sv) {}

char32_t u8codepoint_view::iterator::operator*() const {
    char32_t c = pos[0];
    if(c < 0x80) return c;
    if(c < 0xE0) return (c & 0x1F) << 6 | (pos[1] & 0x3F);
    if(c < 0xF0) return (c & 0x0F) << 12 | (pos[1] & 0x3F) << 6 | (pos[2] & 0x3F);
    return (c & 0x07) << 18 | (pos[1] & 0x3F) << 12 | (pos[2] & 0x3F) << 6 | (pos[3] & 0x3F);
}

u8codepoint_view::iterator &u8codepoint_view::iterator::operator++() {
    if(pos[0] < 0x80)
        pos += 1;
    else if(pos[0] < 0xE0)
        pos += 2;
    else if(pos[0] < 0xF0)
        pos += 3;
    else
        pos += 4;

    return *this;
}

bool u8codepoint_view::iterator::operator==(const iterator &o) const { return pos == o.pos; }

u8codepoint_view::iterator u8codepoint_view::begin() const { return {reinterpret_cast<const u8 *>(m_sv.data())}; }

u8codepoint_view::iterator u8codepoint_view::end() const {
    return {reinterpret_cast<const u8 *>(m_sv.data() + m_sv.size())};
}

u16codepoint_view::u16codepoint_view(std::u16string_view sv) : m_sv(sv) {}

char32_t u16codepoint_view::iterator::operator*() const {
    if(*pos >= 0xD800 && *pos <= 0xDBFF) return 0x10000 + (char32_t(*pos - 0xD800) << 10) + (pos[1] - 0xDC00);
    return *pos;
}

u16codepoint_view::iterator &u16codepoint_view::iterator::operator++() {
    pos += (*pos >= 0xD800 && *pos <= 0xDBFF) ? 2 : 1;
    return *this;
}

bool u16codepoint_view::iterator::operator==(const iterator &o) const { return pos == o.pos; }

u16codepoint_view::iterator u16codepoint_view::begin() const {
    return {m_sv.data() /* NOLINT(bugprone-suspicious-stringview-data-usage) */};
}

u16codepoint_view::iterator u16codepoint_view::end() const { return {m_sv.data() + m_sv.size()}; }

u8codepoint_view codepoints(std::string_view sv) { return u8codepoint_view{sv}; }

u16codepoint_view codepoints(std::u16string_view sv) { return u16codepoint_view{sv}; }

uSz prev(std::string_view sv, uSz pos) {
    if(pos == 0) return 0;
    do {
        pos--;
    } while(pos > 0 && (static_cast<u8>(sv[pos]) & 0xC0) == 0x80);
    return pos;
}

uSz next(std::string_view sv, uSz pos) {
    if(pos >= sv.size()) return sv.size();
    do {
        pos++;
    } while(pos < sv.size() && (static_cast<u8>(sv[pos]) & 0xC0) == 0x80);
    return pos;
}

}  // namespace UniString

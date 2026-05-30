// Copyright (c) 2026, WH, All rights reserved.
#pragma once

/* usage:
 * #define ENUM_VALUES(X) \
 *      X(LABEL0, = 1) X(LABEL1) X(LABEL2) X(LABEL3)
 *
 * enum MC_DEFINE_ENUM(EnumName, unsigned char, ENUM_VALUES, EnumName_to_string)
 * or
 * enum class MC_DEFINE_ENUM(EnumName, unsigned char, ENUM_VALUES, EnumName_to_string)
 */

#define MC_ENUM_XMACRO_VALUE(name, ...) name __VA_ARGS__,

#define MC_ENUM_XMACRO_CASE(name, ...) \
    case EnumType__::name:             \
        return #name;

#define MC_DEFINE_ENUM(x_enum_name__, size__, x_list__, to_string_funcname__) \
    x_enum_name__:                                                            \
    size__{x_list__(MC_ENUM_XMACRO_VALUE)};                                   \
    inline constexpr const char *to_string_funcname__(x_enum_name__ e) {      \
        using EnumType__ = x_enum_name__;                                     \
        switch(e) { x_list__(MC_ENUM_XMACRO_CASE) }                           \
        return "<unknown>";                                                   \
    }
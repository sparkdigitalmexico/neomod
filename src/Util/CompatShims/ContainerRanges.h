// Copyright (c) 2026, WH, All rights reserved.
// compatibility wrappers for some C++23+ ranges features
#pragma once

#if __has_include("config.h")
#include "config.h"
#endif

#include "noinclude.h"

#include <vector>
#include <ranges>

namespace Mc::ranges {

template <class R, class T>
concept ContainerCompatibleRange =
    std::ranges::input_range<R> && std::convertible_to<std::ranges::range_reference_t<R>, T>;

// NOLINTBEGIN(cppcoreguidelines-missing-std-forward)

template <typename Container, typename R>
    requires ContainerCompatibleRange<R, typename Container::value_type>
constexpr void assign(Container& c, R&& rg) {
    c.assign(std::ranges::begin(rg), std::ranges::end(rg));
}

template <typename Container, typename R>
    requires ContainerCompatibleRange<R, typename Container::value_type>
constexpr void append(Container& c, R&& rg) {
    if(c.empty()) {
        assign(c, std::forward<R>(rg));
    } else {
        c.insert(c.end(), std::ranges::begin(rg), std::ranges::end(rg));
    }
}

template <typename Container, typename R>
    requires ContainerCompatibleRange<R, typename Container::value_type>
constexpr typename Container::iterator insert(Container& c, typename Container::const_iterator pos, R&& rg) {
    if(pos == c.end()) {
        const auto offset = c.size();
        append(c, std::forward<R>(rg));
        auto it = c.begin();
        std::advance(it, static_cast<typename Container::difference_type>(offset));
        return it;
    } else {
        return c.insert(pos, std::ranges::begin(rg), std::ranges::end(rg));
    }
}

template <typename Container>
constexpr forceinline bool contains(const Container& c, const typename Container::value_type& val) {
    return std::find(c.begin(), c.end(), val) != c.end();
}

template <typename Container>
constexpr forceinline auto find(const Container& c, const typename Container::value_type& val) {
    return std::find(c.begin(), c.end(), val);
}

// NOLINTEND(cppcoreguidelines-missing-std-forward)

}  // namespace Mc::ranges

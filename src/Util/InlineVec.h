#pragma once
// Copyright (c) 2026, WH, All rights reserved.
// vector with small-size optimization
#if __has_include("config.h")
#include "config.h"
#endif

#include "types.h"

#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace Mc {
template <typename T, uSz N>
struct InlineVec {
    static_assert(std::is_trivially_copyable_v<T>, "InlineVec relies on trivial relocation/destruction");
    static_assert(N > 0);

    using value_type = T;
    using size_type = uSz;
    using difference_type = sSz;
    using reference = T &;
    using const_reference = const T &;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = T *;
    using const_iterator = const T *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
    InlineVec() = default;
    ~InlineVec() = default;

    InlineVec(uSz count, const T &value) {
        reserve(count);
        for(uSz i = 0; i < count; ++i) m_data[i] = value;
        m_size = count;
    }

    explicit InlineVec(uSz count) {
        reserve(count);
        std::memset(m_data, 0, count * sizeof(T));
        m_size = count;
    }

    InlineVec(std::initializer_list<T> init) {
        reserve(init.size());
        std::memcpy(m_data, init.begin(), init.size() * sizeof(T));
        m_size = init.size();
    }

    template <std::input_iterator It, std::sentinel_for<It> Sent>
    InlineVec(It first, Sent last) {
        if constexpr(std::sized_sentinel_for<Sent, It>) reserve(static_cast<uSz>(last - first));
        for(; first != last; ++first) push_back(*first);
    }

    InlineVec(const InlineVec &other) {
        reserve(other.m_size);
        std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
        m_size = other.m_size;
    }

    InlineVec &operator=(const InlineVec &other) {
        if(this != &other) {
            reserve(other.m_size);
            std::memcpy(m_data, other.m_data, other.m_size * sizeof(T));
            m_size = other.m_size;
        }
        return *this;
    }

    InlineVec(InlineVec &&other) noexcept {
        if(other.m_heap) {
            m_heap = std::move(other.m_heap);
            m_data = m_heap.get();
        } else {
            std::memcpy(m_inline, other.m_inline, other.m_size * sizeof(T));
            m_data = m_inline;
        }
        m_size = other.m_size;
        m_cap = other.m_cap;
        other.m_data = other.m_inline;
        other.m_size = 0;
        other.m_cap = N;
    }

    InlineVec &operator=(InlineVec &&other) noexcept {
        if(this != &other) {
            if(other.m_heap) {
                m_heap = std::move(other.m_heap);
                m_data = m_heap.get();
            } else {
                m_heap.reset();
                std::memcpy(m_inline, other.m_inline, other.m_size * sizeof(T));
                m_data = m_inline;
            }
            m_size = other.m_size;
            m_cap = other.m_cap;
            other.m_data = other.m_inline;
            other.m_size = 0;
            other.m_cap = N;
        }
        return *this;
    }

    // iterators
    [[nodiscard]] iterator begin() noexcept { return m_data; }
    [[nodiscard]] const_iterator begin() const noexcept { return m_data; }
    [[nodiscard]] iterator end() noexcept { return m_data + m_size; }
    [[nodiscard]] const_iterator end() const noexcept { return m_data + m_size; }
    [[nodiscard]] const_iterator cbegin() const noexcept { return m_data; }
    [[nodiscard]] const_iterator cend() const noexcept { return m_data + m_size; }
    [[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    [[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    // capacity
    [[nodiscard]] uSz size() const noexcept { return m_size; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] uSz capacity() const noexcept { return m_cap; }

    void reserve(uSz new_cap) {
        if(new_cap > m_cap) grow_to(new_cap);
    }

    // element access
    [[nodiscard]] T &operator[](uSz i) noexcept { return m_data[i]; }
    [[nodiscard]] const T &operator[](uSz i) const noexcept { return m_data[i]; }
    [[nodiscard]] T &front() noexcept { return m_data[0]; }
    [[nodiscard]] const T &front() const noexcept { return m_data[0]; }
    [[nodiscard]] T &back() noexcept { return m_data[m_size - 1]; }
    [[nodiscard]] const T &back() const noexcept { return m_data[m_size - 1]; }
    [[nodiscard]] T *data() noexcept { return m_data; }
    [[nodiscard]] const T *data() const noexcept { return m_data; }

    // modifiers
    void assign(uSz count, const T &value) {
        reserve(count);
        for(uSz i = 0; i < count; ++i) m_data[i] = value;
        m_size = count;
    }

    void assign(std::initializer_list<T> init) {
        reserve(init.size());
        std::memcpy(m_data, init.begin(), init.size() * sizeof(T));
        m_size = init.size();
    }

    template <std::input_iterator It, std::sentinel_for<It> Sent>
    void assign(It first, Sent last) {
        m_size = 0;
        if constexpr(std::sized_sentinel_for<Sent, It>) reserve(static_cast<uSz>(last - first));
        for(; first != last; ++first) push_back(*first);
    }

    void clear() noexcept { m_size = 0; }

    void push_back(const T &value) {
        if(m_size == m_cap) grow();
        m_data[m_size++] = value;
    }

    template <typename... Args>
    T &emplace_back(Args &&...args) {
        if(m_size == m_cap) grow();
        m_data[m_size] = T(std::forward<Args>(args)...);
        return m_data[m_size++];
    }

    void pop_back() noexcept { --m_size; }

    void resize(uSz count) {
        reserve(count);
        if(count > m_size) std::memset(m_data + m_size, 0, (count - m_size) * sizeof(T));
        m_size = count;
    }

    void resize(uSz count, const T &value) {
        reserve(count);
        for(uSz i = m_size; i < count; ++i) m_data[i] = value;
        m_size = count;
    }

    iterator insert(const_iterator pos, const T &value) {
        auto idx = static_cast<uSz>(pos - m_data);
        if(m_size == m_cap) grow();
        std::memmove(m_data + idx + 1, m_data + idx, (m_size - idx) * sizeof(T));
        m_data[idx] = value;
        ++m_size;
        return m_data + idx;
    }

    iterator erase(const_iterator pos) noexcept {
        auto idx = static_cast<uSz>(pos - m_data);
        --m_size;
        std::memmove(m_data + idx, m_data + idx + 1, (m_size - idx) * sizeof(T));
        return m_data + idx;
    }

    iterator erase(const_iterator first, const_iterator last) noexcept {
        auto idx = static_cast<uSz>(first - m_data);
        auto count = static_cast<uSz>(last - first);
        if(count == 0) return m_data + idx;
        m_size -= count;
        std::memmove(m_data + idx, m_data + idx + count, (m_size - idx) * sizeof(T));
        return m_data + idx;
    }

    void swap(InlineVec &other) noexcept {
        InlineVec tmp(std::move(other));
        other = std::move(*this);
        *this = std::move(tmp);
    }

    friend void swap(InlineVec &a, InlineVec &b) noexcept { a.swap(b); }

    friend bool operator==(const InlineVec &a, const InlineVec &b) {
        if(a.m_size != b.m_size) return false;
        for(uSz i = 0; i < a.m_size; ++i)
            if(!(a.m_data[i] == b.m_data[i])) return false;
        return true;
    }

   private:
    void grow() { grow_to(m_cap * 2); }

    void grow_to(uSz new_cap) {
        auto bigger = std::make_unique_for_overwrite<T[]>(new_cap);
        std::memcpy(bigger.get(), m_data, m_size * sizeof(T));
        m_heap = std::move(bigger);
        m_data = m_heap.get();
        m_cap = new_cap;
    }

    T m_inline[N];
    T *m_data{m_inline};
    uSz m_size{0};
    uSz m_cap{N};
    std::unique_ptr<T[]> m_heap{};
};
}  // namespace Mc

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d
{
    class buffer final {
    public:
        using value_type = u8;

        using pointer = u8*;
        using const_pointer = const u8*;
        using reference = u8&;
        using const_reference = const u8&;

        using iterator = pointer;
        using const_iterator = const_pointer;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    public:
        buffer() = default;

        buffer(buffer&& other) noexcept;
        buffer& operator=(buffer&& other) noexcept;

        buffer(const buffer& other);
        buffer& operator=(const buffer& other);

        explicit buffer(std::size_t size);
        buffer(const void* src, std::size_t size);

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

        reverse_iterator rbegin() noexcept;
        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator crbegin() const noexcept;

        reverse_iterator rend() noexcept;
        const_reverse_iterator rend() const noexcept;
        const_reverse_iterator crend() const noexcept;

        buffer& fill(u8 ch) noexcept;
        buffer& resize(std::size_t nsize);

        buffer& assign(buffer&& other) noexcept;
        buffer& assign(const buffer& other);
        buffer& assign(const void* src, std::size_t nsize);

        void swap(buffer& other) noexcept;
        void clear() noexcept;
        bool empty() const noexcept;

        u8* data() noexcept;
        const u8* data() const noexcept;
        std::size_t size() const noexcept;
    private:
        struct deleter {
            void operator()(u8*) const noexcept;
        };
        using data_t = std::unique_ptr<u8, deleter>;
        data_t data_;
        std::size_t size_ = 0;
    };

    void swap(buffer& l, buffer& r) noexcept;
    bool operator<(const buffer& l, const buffer& r) noexcept;
    bool operator==(const buffer& l, const buffer& r) noexcept;
    bool operator!=(const buffer& l, const buffer& r) noexcept;
}

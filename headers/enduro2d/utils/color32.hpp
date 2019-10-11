/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d
{
    class color32 final {
    public:
        u8 r = 255;
        u8 g = 255;
        u8 b = 255;
        u8 a = 255;
    public:
        static const color32& clear() noexcept;
        static const color32& black() noexcept;
        static const color32& white() noexcept;
        static const color32& red() noexcept;
        static const color32& green() noexcept;
        static const color32& blue() noexcept;
        static const color32& yellow() noexcept;
        static const color32& magenta() noexcept;
        static const color32& cyan() noexcept;
    public:
        color32() noexcept = default;

        color32(const color32& other) noexcept = default;
        color32& operator=(const color32& other) noexcept = default;

        color32(u8 r, u8 g, u8 b, u8 a = 255) noexcept;
        explicit color32(const color& other) noexcept;

        u8* data() noexcept;
        const u8* data() const noexcept;

        u8& operator[](std::size_t index) noexcept;
        u8 operator[](std::size_t index) const noexcept;

        color32& operator+=(u8 v) noexcept;
        color32& operator-=(u8 v) noexcept;
        color32& operator*=(u8 v) noexcept;
        color32& operator/=(u8 v) noexcept;

        color32& operator+=(const color32& other) noexcept;
        color32& operator-=(const color32& other) noexcept;
        color32& operator*=(const color32& other) noexcept;
        color32& operator/=(const color32& other) noexcept;
    };
}

namespace e2d
{
    vec3<u8> make_vec3(const color32& c) noexcept;
    vec4<u8> make_vec4(const color32& c) noexcept;

    bool operator<(const color32& l, const color32& r) noexcept;
    bool operator==(const color32& l, const color32& r) noexcept;
    bool operator!=(const color32& l, const color32& r) noexcept;

    color32 operator+(color32 l, u8 v) noexcept;
    color32 operator-(color32 l, u8 v) noexcept;
    color32 operator*(color32 l, u8 v) noexcept;
    color32 operator/(color32 l, u8 v) noexcept;

    color32 operator+(u8 v, const color32& r) noexcept;
    color32 operator-(u8 v, const color32& r) noexcept;
    color32 operator*(u8 v, const color32& r) noexcept;
    color32 operator/(u8 v, const color32& r) noexcept;

    color32 operator+(color32 l, const color32& r) noexcept;
    color32 operator-(color32 l, const color32& r) noexcept;
    color32 operator*(color32 l, const color32& r) noexcept;
    color32 operator/(color32 l, const color32& r) noexcept;
}

namespace e2d::math
{
    bool approximately(const color32& l, const color32& r) noexcept;
    bool approximately(const color32& l, const color32& r, u8 precision) noexcept;

    u8 minimum(const color32& c) noexcept;
    u8 maximum(const color32& c) noexcept;

    color32 minimized(const color32& c, const color32& cmin) noexcept;
    color32 maximized(const color32& c, const color32& cmax) noexcept;
    color32 clamped(const color32& c, const color32& cmin, const color32& cmax) noexcept;

    bool contains_nan(const color32& c) noexcept;

    color32 lerp(const color32& l, const color32& r, f32 v) noexcept;
}

namespace e2d::colors
{
    u32 pack_color32(const color32& c) noexcept;
    color32 unpack_color32(u32 argb) noexcept;
}

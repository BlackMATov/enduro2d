/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/utils/color32.hpp>
#include <enduro2d/utils/color.hpp>

namespace e2d
{
    const color32& color32::clear() noexcept {
        static const color32 c(0, 0, 0, 0);
        return c;
    }

    const color32& color32::black() noexcept {
        static const color32 c(0, 0, 0, 255);
        return c;
    }

    const color32& color32::white() noexcept {
        static const color32 c(255, 255, 255, 255);
        return c;
    }

    const color32& color32::red() noexcept {
        static const color32 c(255, 0, 0, 255);
        return c;
    }

    const color32& color32::green() noexcept {
        static const color32 c(0, 255, 0, 255);
        return c;
    }

    const color32& color32::blue() noexcept {
        static const color32 c(0, 0, 255, 255);
        return c;
    }

    const color32& color32::yellow() noexcept {
        static const color32 c(255, 255, 0, 255);
        return c;
    }

    const color32& color32::magenta() noexcept {
        static const color32 c(255, 0, 255, 255);
        return c;
    }

    const color32& color32::cyan() noexcept {
        static const color32 c(0, 255, 255, 255);
        return c;
    }

    color32::color32(const color& other) noexcept
    : r(static_cast<u8>(math::saturate(other.r) * 255.f + 0.5f))
    , g(static_cast<u8>(math::saturate(other.g) * 255.f + 0.5f))
    , b(static_cast<u8>(math::saturate(other.b) * 255.f + 0.5f))
    , a(static_cast<u8>(math::saturate(other.a) * 255.f + 0.5f)) {}

    color32::color32(u8 nr, u8 ng, u8 nb, u8 na) noexcept
    : r(nr)
    , g(ng)
    , b(nb)
    , a(na) {}

    u8* color32::data() noexcept {
        return &r;
    }

    const u8* color32::data() const noexcept {
        return &r;
    }

    u8& color32::operator[](std::size_t index) noexcept {
        E2D_ASSERT(index < 4);
        return data()[index];
    }

    u8 color32::operator[](std::size_t index) const noexcept {
        E2D_ASSERT(index < 4);
        return data()[index];
    }

    color32& color32::operator+=(u8 v) noexcept {
        return *this += color32(v,v,v,v);
    }

    color32& color32::operator-=(u8 v) noexcept {
        return *this -= color32(v,v,v,v);
    }

    color32& color32::operator*=(u8 v) noexcept {
        return *this *= color32(v,v,v,v);
    }

    color32& color32::operator/=(u8 v) noexcept {
        return *this /= color32(v,v,v,v);
    }

    color32& color32::operator+=(const color32& other) noexcept {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;
        return *this;
    }

    color32& color32::operator-=(const color32& other) noexcept {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        a -= other.a;
        return *this;
    }

    color32& color32::operator*=(const color32& other) noexcept {
        r *= other.r;
        g *= other.g;
        b *= other.b;
        a *= other.a;
        return *this;
    }

    color32& color32::operator/=(const color32& other) noexcept {
        E2D_ASSERT(!math::is_near_zero(other.r, u8(0)));
        E2D_ASSERT(!math::is_near_zero(other.g, u8(0)));
        E2D_ASSERT(!math::is_near_zero(other.b, u8(0)));
        E2D_ASSERT(!math::is_near_zero(other.a, u8(0)));
        r /= other.r;
        g /= other.g;
        b /= other.b;
        a /= other.a;
        return *this;
    }
}

namespace e2d
{
    vec3<u8> make_vec3(const color32& c) noexcept {
        return make_vec3(c.r, c.g, c.b);
    }

    vec4<u8> make_vec4(const color32& c) noexcept {
        return make_vec4(c.r, c.g, c.b, c.a);
    }

    //
    // color32 (<,==,!=) color32
    //

    bool operator<(const color32& l, const color32& r) noexcept {
        return (l.r < r.r)
            || (l.r == r.r && l.g < r.g)
            || (l.r == r.r && l.g == r.g && l.b < r.b)
            || (l.r == r.r && l.g == r.g && l.b == r.b && l.a < r.a);
    }

    bool operator==(const color32& l, const color32& r) noexcept {
        return math::approximately(l.r, r.r)
            && math::approximately(l.g, r.g)
            && math::approximately(l.b, r.b)
            && math::approximately(l.a, r.a);
    }

    bool operator!=(const color32& l, const color32& r) noexcept {
        return !(l == r);
    }

    //
    // color32 (+,-,*,/) value
    //

    color32 operator+(color32 l, u8 v) noexcept {
        l += v;
        return l;
    }

    color32 operator-(color32 l, u8 v) noexcept {
        l -= v;
        return l;
    }

    color32 operator*(color32 l, u8 v) noexcept {
        l *= v;
        return l;
    }

    color32 operator/(color32 l, u8 v) noexcept {
        l /= v;
        return l;
    }

    //
    // value (+,-,*,/) color32
    //

    color32 operator+(u8 v, const color32& r) noexcept {
        color32 l(v,v,v,v);
        l += r;
        return l;
    }

    color32 operator-(u8 v, const color32& r) noexcept {
        color32 l(v,v,v,v);
        l -= r;
        return l;
    }

    color32 operator*(u8 v, const color32& r) noexcept {
        color32 l(v,v,v,v);
        l *= r;
        return l;
    }

    color32 operator/(u8 v, const color32& r) noexcept {
        color32 l(v,v,v,v);
        l /= r;
        return l;
    }

    //
    // color32 (+,-,*,/) color32
    //

    color32 operator+(color32 l, const color32& r) noexcept {
        l += r;
        return l;
    }

    color32 operator-(color32 l, const color32& r) noexcept {
        l -= r;
        return l;
    }

    color32 operator*(color32 l, const color32& r) noexcept {
        l *= r;
        return l;
    }

    color32 operator/(color32 l, const color32& r) noexcept {
        l /= r;
        return l;
    }
}

namespace e2d::math
{
    //
    // approximately
    //

    bool approximately(const color32& l, const color32& r) noexcept {
        return math::approximately(l.r, r.r)
            && math::approximately(l.g, r.g)
            && math::approximately(l.b, r.b)
            && math::approximately(l.a, r.a);
    }

    bool approximately(const color32& l, const color32& r, u8 precision) noexcept {
        return math::approximately(l.r, r.r, precision)
            && math::approximately(l.g, r.g, precision)
            && math::approximately(l.b, r.b, precision)
            && math::approximately(l.a, r.a, precision);
    }

    //
    // minimum/maximum
    //

    u8 minimum(const color32& c) noexcept {
        return math::min(c.r, c.g, c.b, c.a);
    }

    u8 maximum(const color32& c) noexcept {
        return math::max(c.r, c.g, c.b, c.a);
    }

    //
    // minimized/maximized/clamped
    //

    color32 minimized(const color32& c, const color32& cmin) noexcept {
        return color32(
            math::min(c.r, cmin.r),
            math::min(c.g, cmin.g),
            math::min(c.b, cmin.b),
            math::min(c.a, cmin.a));
    }

    color32 maximized(const color32& c, const color32& cmax) noexcept {
        return color32(
            math::max(c.r, cmax.r),
            math::max(c.g, cmax.g),
            math::max(c.b, cmax.b),
            math::max(c.a, cmax.a));
    }

    color32 clamped(const color32& c, const color32& cmin, const color32& cmax) noexcept {
        return color32(
            math::clamp(c.r, cmin.r, cmax.r),
            math::clamp(c.g, cmin.g, cmax.g),
            math::clamp(c.b, cmin.b, cmax.b),
            math::clamp(c.a, cmin.a, cmax.a));
    }

    //
    // contains_nan
    //

    bool contains_nan(const color32& c) noexcept {
        return !math::is_finite(c.r)
            || !math::is_finite(c.g)
            || !math::is_finite(c.b)
            || !math::is_finite(c.a);
    }
}

namespace e2d::colors
{
    u32 pack_color32(const color32& c) noexcept {
        return
            static_cast<u32>(c.a) << 24 |
            static_cast<u32>(c.r) << 16 |
            static_cast<u32>(c.g) <<  8 |
            static_cast<u32>(c.b) <<  0;
    }

    color32 unpack_color32(u32 argb) noexcept {
        return color32(
            static_cast<u8>((argb >> 16) & 0xFF),
            static_cast<u8>((argb >>  8) & 0xFF),
            static_cast<u8>((argb >>  0) & 0xFF),
            static_cast<u8>((argb >> 24) & 0xFF));
    }
}

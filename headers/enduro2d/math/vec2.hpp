/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

namespace e2d
{
    template < typename T >
    class vec2 final {
        static_assert(
            std::is_arithmetic_v<T>,
            "type of 'vec2' must be arithmetic");
    public:
        using self_type = vec2;
        using value_type = T;
    public:
        T x = 0;
        T y = 0;
    public:
        static const vec2& zero() noexcept;
        static const vec2& unit() noexcept;
        static const vec2& unit_x() noexcept;
        static const vec2& unit_y() noexcept;
    public:
        vec2() noexcept = default;
        vec2(const vec2& other) noexcept = default;
        vec2& operator=(const vec2& other) noexcept = default;

        explicit vec2(T v) noexcept;
        vec2(T x, T y) noexcept;
        explicit vec2(const vec3<T>& other) noexcept;
        explicit vec2(const vec4<T>& other) noexcept;

        template < typename To >
        vec2<To> cast_to() const noexcept;

        T* data() noexcept;
        const T* data() const noexcept;

        T& operator[](std::size_t index) noexcept;
        T operator[](std::size_t index) const noexcept;

        vec2& operator+=(T v) noexcept;
        vec2& operator-=(T v) noexcept;
        vec2& operator*=(T v) noexcept;
        vec2& operator/=(T v) noexcept;

        vec2& operator+=(const vec2& other) noexcept;
        vec2& operator-=(const vec2& other) noexcept;
        vec2& operator*=(const vec2& other) noexcept;
        vec2& operator/=(const vec2& other) noexcept;
    };
}

namespace e2d
{
    template < typename T >
    const vec2<T>& vec2<T>::zero() noexcept {
        static const vec2<T> zero{0, 0};
        return zero;
    }

    template < typename T >
    const vec2<T>& vec2<T>::unit() noexcept {
        static const vec2<T> unit{1, 1};
        return unit;
    }

    template < typename T >
    const vec2<T>& vec2<T>::unit_x() noexcept {
        static const vec2<T> unit_x{1, 0};
        return unit_x;
    }

    template < typename T >
    const vec2<T>& vec2<T>::unit_y() noexcept {
        static const vec2<T> unit_y{0, 1};
        return unit_y;
    }

    template < typename T >
    vec2<T>::vec2(T v) noexcept
    : x(v)
    , y(v) {}

    template < typename T >
    vec2<T>::vec2(T nx, T ny) noexcept
    : x(nx)
    , y(ny) {}

    template < typename T >
    vec2<T>::vec2(const vec3<T>& other) noexcept
    : x(other.x)
    , y(other.y) {}

    template < typename T >
    vec2<T>::vec2(const vec4<T>& other) noexcept
    : x(other.x)
    , y(other.y) {}

    template < typename T >
    template < typename To >
    vec2<To> vec2<T>::cast_to() const noexcept {
        return {
            math::numeric_cast<To>(x),
            math::numeric_cast<To>(y)};
    }

    template < typename T >
    T* vec2<T>::data() noexcept {
        return &x;
    }

    template < typename T >
    const T* vec2<T>::data() const noexcept {
        return &x;
    }

    template < typename T >
    T& vec2<T>::operator[](std::size_t index) noexcept {
        E2D_ASSERT(index < 2);
        return data()[index];
    }

    template < typename T >
    T vec2<T>::operator[](std::size_t index) const noexcept {
        E2D_ASSERT(index < 2);
        return data()[index];
    }

    template < typename T >
    vec2<T>& vec2<T>::operator+=(T v) noexcept {
        x += v;
        y += v;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator-=(T v) noexcept {
        x -= v;
        y -= v;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator*=(T v) noexcept {
        x *= v;
        y *= v;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator/=(T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        x /= v;
        y /= v;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator+=(const vec2& other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator-=(const vec2& other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator*=(const vec2& other) noexcept {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    template < typename T >
    vec2<T>& vec2<T>::operator/=(const vec2& other) noexcept {
        E2D_ASSERT(!math::is_near_zero(other.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.y, T(0)));
        x /= other.x;
        y /= other.y;
        return *this;
    }
}

namespace e2d
{
    //
    // make_vec2
    //

    template < typename T >
    vec2<T> make_vec2(T v) noexcept {
        return vec2<T>(v);
    }

    template < typename T >
    vec2<T> make_vec2(T x, T y) noexcept {
        return vec2<T>(x, y);
    }

    template < typename T >
    vec2<T> make_vec2(const vec3<T>& other) noexcept {
        return vec2<T>(other);
    }

    template < typename T >
    vec2<T> make_vec2(const vec4<T>& other) noexcept {
        return vec2<T>(other);
    }

    //
    // vec2 (==,!=) vec2
    //

    template < typename T >
    bool operator==(const vec2<T>& l, const vec2<T>& r) noexcept {
        return
            math::approximately(l.x, r.x) &&
            math::approximately(l.y, r.y);
    }

    template < typename T >
    bool operator!=(const vec2<T>& l, const vec2<T>& r) noexcept {
        return !(l == r);
    }

    //
    // vec2 (<,>,<=,>=) vec2
    //

    template < typename T >
    bool operator<(const vec2<T>& l, const vec2<T>& r) noexcept {
        return
            (l.x < r.x) ||
            (l.x == r.x && l.y < r.y);
    }

    template < typename T >
    bool operator>(const vec2<T>& l, const vec2<T>& r) noexcept {
        return r < l;
    }

    template < typename T >
    bool operator<=(const vec2<T>& l, const vec2<T>& r) noexcept {
        return !(r < l);
    }

    template < typename T >
    bool operator>=(const vec2<T>& l, const vec2<T>& r) noexcept {
        return !(l < r);
    }

    //
    // (-) vec2
    //

    template < typename T >
    vec2<T> operator-(const vec2<T>& v) noexcept {
        return {
            -v.x,
            -v.y};
    }

    //
    // vec2 (+,-,*,/) value
    //

    template < typename T >
    vec2<T> operator+(const vec2<T>& l, T v) noexcept {
        return {
            l.x + v,
            l.y + v};
    }

    template < typename T >
    vec2<T> operator-(const vec2<T>& l, T v) noexcept {
        return {
            l.x - v,
            l.y - v};
    }

    template < typename T >
    vec2<T> operator*(const vec2<T>& l, T v) noexcept {
        return {
            l.x * v,
            l.y * v};
    }

    template < typename T >
    vec2<T> operator/(const vec2<T>& l, T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        return {
            l.x / v,
            l.y / v};
    }

    //
    // value (+,-,*,/) vec2
    //

    template < typename T >
    vec2<T> operator+(T v, const vec2<T>& r) noexcept {
        return {
            v + r.x,
            v + r.y};
    }

    template < typename T >
    vec2<T> operator-(T v, const vec2<T>& r) noexcept {
        return {
            v - r.x,
            v - r.y};
    }

    template < typename T >
    vec2<T> operator*(T v, const vec2<T>& r) noexcept {
        return {
            v * r.x,
            v * r.y};
    }

    template < typename T >
    vec2<T> operator/(T v, const vec2<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        return {
            v / r.x,
            v / r.y};
    }

    //
    // vec2 (+,-,*,/) vec2
    //

    template < typename T >
    vec2<T> operator+(const vec2<T>& l, const vec2<T>& r) noexcept {
        return {
            l.x + r.x,
            l.y + r.y};
    }

    template < typename T >
    vec2<T> operator-(const vec2<T>& l, const vec2<T>& r) noexcept {
        return {
            l.x - r.x,
            l.y - r.y};
    }

    template < typename T >
    vec2<T> operator*(const vec2<T>& l, const vec2<T>& r) noexcept {
        return {
            l.x * r.x,
            l.y * r.y};
    }

    template < typename T >
    vec2<T> operator/(const vec2<T>& l, const vec2<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        return {
            l.x / r.x,
            l.y / r.y};
    }
}

namespace e2d::math
{
    //
    // approximately
    //

    template < typename T >
    bool approximately(
        const vec2<T>& l,
        const vec2<T>& r,
        T precision = math::default_precision<T>()) noexcept
    {
        return
            math::approximately(l.x, r.x, precision) &&
            math::approximately(l.y, r.y, precision);
    }

    //
    // dot
    //

    template < typename T >
    T dot(const vec2<T>& l, const vec2<T>& r) noexcept {
        return
            l.x * r.x +
            l.y * r.y;
    }

    template < typename T >
    T abs_dot(const vec2<T>& l, const vec2<T>& r) noexcept {
        return
            math::abs(l.x * r.x) +
            math::abs(l.y * r.y);
    }

    //
    // length
    //

    template < typename T >
    T length_squared(const vec2<T>& v) noexcept {
        return dot(v, v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    length(const vec2<T>& v) noexcept {
        return math::sqrt(length_squared(v));
    }

    //
    // distance
    //

    template < typename T >
    math::make_distance_t<T>
    distance_squared(const vec2<T>& l, const vec2<T>& r) noexcept {
        return length_squared(make_vec2(
            math::distance(l.x, r.x),
            math::distance(l.y, r.y)));
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    distance(const vec2<T>& l, const vec2<T>& r) noexcept {
        return math::sqrt(distance_squared(l, r));
    }

    //
    // normalized
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec2<T>>
    normalized(
        const vec2<T>& v,
        T precision = math::default_precision<T>()) noexcept
    {
        const T l = length(v);
        const T inv_l = math::is_near_zero(l, precision)
            ? T(0)
            : T(1) / l;
        return v * inv_l;
    }

    //
    // minimum/maximum
    //

    template < typename T >
    T minimum(const vec2<T>& v) noexcept {
        return math::min(v.x, v.y);
    }

    template < typename T >
    T maximum(const vec2<T>& v) noexcept {
        return math::max(v.x, v.y);
    }

    //
    // minimized/maximized/clamped
    //

    template < typename T >
    vec2<T> minimized(const vec2<T>& v, const vec2<T>& vmin) noexcept {
        return {
            math::min(v.x, vmin.x),
            math::min(v.y, vmin.y)};
    }

    template < typename T >
    vec2<T> maximized(const vec2<T>& v, const vec2<T>& vmax) noexcept {
        return {
            math::max(v.x, vmax.x),
            math::max(v.y, vmax.y)};
    }

    template < typename T >
    vec2<T> clamped(const vec2<T>& v, const vec2<T>& vmin, const vec2<T>& vmax) noexcept {
        return {
            math::clamp(v.x, vmin.x, vmax.x),
            math::clamp(v.y, vmin.y, vmax.y)};
    }

    template < typename T >
    vec2<T> saturated(const vec2<T>& v) noexcept {
        return clamped(v, vec2<T>::zero(), vec2<T>::unit());
    }

    //
    // lerp/inverse_lerp
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec2<T>>
    lerp(const vec2<T>& l, const vec2<T>& r, T v) noexcept {
        return {
            math::lerp(l.x, r.x, v),
            math::lerp(l.y, r.y, v)};
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec2<T>>
    lerp(const vec2<T>& l, const vec2<T>& r, const vec2<T>& v) noexcept {
        return {
            math::lerp(l.x, r.x, v.x),
            math::lerp(l.y, r.y, v.y)};
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec2<T>>
    inverse_lerp(const vec2<T>& l, const vec2<T>& r, const vec2<T>& v) noexcept {
        return {
            math::inverse_lerp(l.x, r.x, v.x),
            math::inverse_lerp(l.y, r.y, v.y)};
    }

    //
    // contains_nan
    //

    template < typename T >
    bool contains_nan(const vec2<T>& v) noexcept {
        return
            !math::is_finite(v.x) ||
            !math::is_finite(v.y);
    }

    //
    // any, all
    //

    inline bool any(const vec2<bool>& v) noexcept {
        return v.x | v.y;
    }

    inline bool all(const vec2<bool>& v) noexcept {
        return v.x & v.y;
    }
}

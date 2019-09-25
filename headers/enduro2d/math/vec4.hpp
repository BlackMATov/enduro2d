/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "vec2.hpp"
#include "vec3.hpp"

namespace e2d
{
    template < typename T >
    class vec4 final {
        static_assert(
            std::is_arithmetic_v<T>,
            "type of 'vec4' must be arithmetic");
    public:
        using self_type = vec4;
        using value_type = T;
    public:
        T x = 0;
        T y = 0;
        T z = 0;
        T w = 0;
    public:
        static const vec4& zero() noexcept;
        static const vec4& unit() noexcept;
        static const vec4& unit_x() noexcept;
        static const vec4& unit_y() noexcept;
        static const vec4& unit_z() noexcept;
        static const vec4& unit_w() noexcept;
    public:
        vec4() noexcept = default;
        vec4(const vec4& other) noexcept = default;
        vec4& operator=(const vec4& other) noexcept = default;

        explicit vec4(T v) noexcept;
        vec4(T x, T y, T z, T w) noexcept;
        explicit vec4(const vec2<T>& other, T z, T w) noexcept;
        explicit vec4(const vec3<T>& other, T w) noexcept;

        template < typename To >
        vec4<To> cast_to() const noexcept;

        T* data() noexcept;
        const T* data() const noexcept;

        T& operator[](std::size_t index) noexcept;
        T operator[](std::size_t index) const noexcept;

        vec4& operator+=(T v) noexcept;
        vec4& operator-=(T v) noexcept;
        vec4& operator*=(T v) noexcept;
        vec4& operator/=(T v) noexcept;

        vec4& operator+=(const vec4& other) noexcept;
        vec4& operator-=(const vec4& other) noexcept;
        vec4& operator*=(const vec4& other) noexcept;
        vec4& operator/=(const vec4& other) noexcept;
    };
}

namespace e2d
{
    template < typename T >
    const vec4<T>& vec4<T>::zero() noexcept {
        static const vec4<T> zero{0, 0, 0, 0};
        return zero;
    }

    template < typename T >
    const vec4<T>& vec4<T>::unit() noexcept {
        static const vec4<T> unit{1, 1, 1, 1};
        return unit;
    }

    template < typename T >
    const vec4<T>& vec4<T>::unit_x() noexcept {
        static const vec4<T> unit_x{1, 0, 0, 0};
        return unit_x;
    }

    template < typename T >
    const vec4<T>& vec4<T>::unit_y() noexcept {
        static const vec4<T> unit_y{0, 1, 0, 0};
        return unit_y;
    }

    template < typename T >
    const vec4<T>& vec4<T>::unit_z() noexcept {
        static const vec4<T> unit_z{0, 0, 1, 0};
        return unit_z;
    }

    template < typename T >
    const vec4<T>& vec4<T>::unit_w() noexcept {
        static const vec4<T> unit_w{0, 0, 0, 1};
        return unit_w;
    }

    template < typename T >
    vec4<T>::vec4(T v) noexcept
    : x(v)
    , y(v)
    , z(v)
    , w(v) {}

    template < typename T >
    vec4<T>::vec4(T nx, T ny, T nz, T nw) noexcept
    : x(nx)
    , y(ny)
    , z(nz)
    , w(nw) {}

    template < typename T >
    vec4<T>::vec4(const vec2<T>& other, T nz, T nw) noexcept
    : x(other.x)
    , y(other.y)
    , z(nz)
    , w(nw) {}

    template < typename T >
    vec4<T>::vec4(const vec3<T>& other, T nw) noexcept
    : x(other.x)
    , y(other.y)
    , z(other.z)
    , w(nw) {}

    template < typename T >
    template < typename To >
    vec4<To> vec4<T>::cast_to() const noexcept {
        return {
            math::numeric_cast<To>(x),
            math::numeric_cast<To>(y),
            math::numeric_cast<To>(z),
            math::numeric_cast<To>(w)};
    }

    template < typename T >
    T* vec4<T>::data() noexcept {
        return &x;
    }

    template < typename T >
    const T* vec4<T>::data() const noexcept {
        return &x;
    }

    template < typename T >
    T& vec4<T>::operator[](std::size_t index) noexcept {
        E2D_ASSERT(index < 4);
        return data()[index];
    }

    template < typename T >
    T vec4<T>::operator[](std::size_t index) const noexcept {
        E2D_ASSERT(index < 4);
        return data()[index];
    }

    template < typename T >
    vec4<T>& vec4<T>::operator+=(T v) noexcept {
        x += v;
        y += v;
        z += v;
        w += v;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator-=(T v) noexcept {
        x -= v;
        y -= v;
        z -= v;
        w -= v;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator*=(T v) noexcept {
        x *= v;
        y *= v;
        z *= v;
        w *= v;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator/=(T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        x /= v;
        y /= v;
        z /= v;
        w /= v;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator+=(const vec4& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator-=(const vec4& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator*=(const vec4& other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }

    template < typename T >
    vec4<T>& vec4<T>::operator/=(const vec4& other) noexcept {
        E2D_ASSERT(!math::is_near_zero(other.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.z, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.w, T(0)));
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }
}

namespace e2d
{
    //
    // make_vec4
    //

    template < typename T >
    vec4<T> make_vec4(T v) noexcept {
        return vec4<T>(v);
    }

    template < typename T >
    vec4<T> make_vec4(T x, T y, T z, T w) noexcept {
        return vec4<T>(x, y, z, w);
    }

    template < typename T >
    vec4<T> make_vec4(const vec2<T>& other, T z, T w) noexcept {
        return vec4<T>(other, z, w);
    }

    template < typename T >
    vec4<T> make_vec4(const vec3<T>& other, T w) noexcept {
        return vec4<T>(other, w);
    }

    //
    // vec4 (==,!=) vec4
    //

    template < typename T >
    bool operator==(const vec4<T>& l, const vec4<T>& r) noexcept {
        return
            math::approximately(l.x, r.x) &&
            math::approximately(l.y, r.y) &&
            math::approximately(l.z, r.z) &&
            math::approximately(l.w, r.w);
    }

    template < typename T >
    bool operator!=(const vec4<T>& l, const vec4<T>& r) noexcept {
        return !(l == r);
    }

    //
    // vec4 (<,>,<=,>=) vec4
    //

    template < typename T >
    bool operator<(const vec4<T>& l, const vec4<T>& r) noexcept {
        return
            (l.x < r.x) ||
            (l.x == r.x && l.y < r.y) ||
            (l.x == r.x && l.y == r.y && l.z < r.z) ||
            (l.x == r.x && l.y == r.y && l.z == r.z && l.w < r.w);
    }

    template < typename T >
    bool operator>(const vec4<T>& l, const vec4<T>& r) noexcept {
        return r < l;
    }

    template < typename T >
    bool operator<=(const vec4<T>& l, const vec4<T>& r) noexcept {
        return !(r < l);
    }

    template < typename T >
    bool operator>=(const vec4<T>& l, const vec4<T>& r) noexcept {
        return !(l < r);
    }

    //
    // (-) vec4
    //

    template < typename T >
    vec4<T> operator-(const vec4<T>& v) noexcept {
        return {
            -v.x,
            -v.y,
            -v.z,
            -v.w};
    }

    //
    // vec4 (+,-,*,/) value
    //

    template < typename T >
    vec4<T> operator+(const vec4<T>& l, T v) noexcept {
        return {
            l.x + v,
            l.y + v,
            l.z + v,
            l.w + v};
    }

    template < typename T >
    vec4<T> operator-(const vec4<T>& l, T v) noexcept {
        return {
            l.x - v,
            l.y - v,
            l.z - v,
            l.w - v};
    }

    template < typename T >
    vec4<T> operator*(const vec4<T>& l, T v) noexcept {
        return {
            l.x * v,
            l.y * v,
            l.z * v,
            l.w * v};
    }

    template < typename T >
    vec4<T> operator/(const vec4<T>& l, T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        return {
            l.x / v,
            l.y / v,
            l.z / v,
            l.w / v};
    }

    //
    // value (+,-,*,/) vec4
    //

    template < typename T >
    vec4<T> operator+(T v, const vec4<T>& r) noexcept {
        return {
            v + r.x,
            v + r.y,
            v + r.z,
            v + r.w};
    }

    template < typename T >
    vec4<T> operator-(T v, const vec4<T>& r) noexcept {
        return {
            v - r.x,
            v - r.y,
            v - r.z,
            v - r.w};
    }

    template < typename T >
    vec4<T> operator*(T v, const vec4<T>& r) noexcept {
        return {
            v * r.x,
            v * r.y,
            v * r.z,
            v * r.w};
    }

    template < typename T >
    vec4<T> operator/(T v, const vec4<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.z, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.w, T(0)));
        return {
            v / r.x,
            v / r.y,
            v / r.z,
            v / r.w};
    }

    //
    // vec4 (+,-,*,/) vec4
    //

    template < typename T >
    vec4<T> operator+(const vec4<T>& l, const vec4<T>& r) noexcept {
        return {
            l.x + r.x,
            l.y + r.y,
            l.z + r.z,
            l.w + r.w};
    }

    template < typename T >
    vec4<T> operator-(const vec4<T>& l, const vec4<T>& r) noexcept {
        return {
            l.x - r.x,
            l.y - r.y,
            l.z - r.z,
            l.w - r.w};
    }

    template < typename T >
    vec4<T> operator*(const vec4<T>& l, const vec4<T>& r) noexcept {
        return {
            l.x * r.x,
            l.y * r.y,
            l.z * r.z,
            l.w * r.w};
    }

    template < typename T >
    vec4<T> operator/(const vec4<T>& l, const vec4<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.z, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.w, T(0)));
        return {
            l.x / r.x,
            l.y / r.y,
            l.z / r.z,
            l.w / r.w};
    }
}

namespace e2d::math
{
    //
    // approximately
    //

    template < typename T >
    bool approximately(
        const vec4<T>& l,
        const vec4<T>& r,
        T precision = math::default_precision<T>()) noexcept
    {
        return
            math::approximately(l.x, r.x, precision) &&
            math::approximately(l.y, r.y, precision) &&
            math::approximately(l.z, r.z, precision) &&
            math::approximately(l.w, r.w, precision);
    }

    //
    // dot
    //

    template < typename T >
    T dot(const vec4<T>& l, const vec4<T>& r) noexcept {
        return
            l.x * r.x +
            l.y * r.y +
            l.z * r.z +
            l.w * r.w;
    }

    template < typename T >
    T abs_dot(const vec4<T>& l, const vec4<T>& r) noexcept {
        return
            math::abs(l.x * r.x) +
            math::abs(l.y * r.y) +
            math::abs(l.z * r.z) +
            math::abs(l.w * r.w);
    }

    //
    // length
    //

    template < typename T >
    T length_squared(const vec4<T>& v) noexcept {
        return dot(v, v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    length(const vec4<T>& v) noexcept {
        return math::sqrt(length_squared(v));
    }

    //
    // distance
    //

    template < typename T >
    math::make_distance_t<T>
    distance_squared(const vec4<T>& l, const vec4<T>& r) noexcept {
        return length_squared(make_vec4(
            math::distance(l.x, r.x),
            math::distance(l.y, r.y),
            math::distance(l.z, r.z),
            math::distance(l.w, r.w)));
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    distance(const vec4<T>& l, const vec4<T>& r) noexcept {
        return math::sqrt(distance_squared(l, r));
    }

    //
    // normalized
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec4<T>>
    normalized(
        const vec4<T>& v,
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
    T minimum(const vec4<T>& v) noexcept {
        return math::min(v.x, v.y, v.z, v.w);
    }

    template < typename T >
    T maximum(const vec4<T>& v) noexcept {
        return math::max(v.x, v.y, v.z, v.w);
    }

    //
    // minimized/maximized/clamped
    //

    template < typename T >
    vec4<T> minimized(const vec4<T>& v, const vec4<T>& vmin) noexcept {
        return {
            math::min(v.x, vmin.x),
            math::min(v.y, vmin.y),
            math::min(v.z, vmin.z),
            math::min(v.w, vmin.w)};
    }

    template < typename T >
    vec4<T> maximized(const vec4<T>& v, const vec4<T>& vmax) noexcept {
        return {
            math::max(v.x, vmax.x),
            math::max(v.y, vmax.y),
            math::max(v.z, vmax.z),
            math::max(v.w, vmax.w)};
    }

    template < typename T >
    vec4<T> clamped(const vec4<T>& v, const vec4<T>& vmin, const vec4<T>& vmax) noexcept {
        return {
            math::clamp(v.x, vmin.x, vmax.x),
            math::clamp(v.y, vmin.y, vmax.y),
            math::clamp(v.z, vmin.z, vmax.z),
            math::clamp(v.w, vmin.w, vmax.w)};
    }

    template < typename T >
    vec4<T> saturated(const vec4<T>& v) noexcept {
        return clamped(v, vec4<T>::zero(), vec4<T>::unit());
    }

    //
    // lerp/inverse_lerp
    //

    template < typename T >
    vec4<T> lerp(const vec4<T>& l, const vec4<T>& r, T v) noexcept {
        return {
            math::lerp(l.x, r.x, v),
            math::lerp(l.y, r.y, v),
            math::lerp(l.z, r.z, v),
            math::lerp(l.w, r.w, v)};
    }

    template < typename T >
    vec4<T> lerp(const vec4<T>& l, const vec4<T>& r, const vec4<T>& v) noexcept {
        return {
            math::lerp(l.x, r.x, v.x),
            math::lerp(l.y, r.y, v.y),
            math::lerp(l.z, r.z, v.z),
            math::lerp(l.w, r.w, v.w)};
    }

    template < typename T >
    vec4<T> inverse_lerp(const vec4<T>& l, const vec4<T>& r, const vec4<T>& v) noexcept {
        return {
            math::inverse_lerp(l.x, r.x, v.x),
            math::inverse_lerp(l.y, r.y, v.y),
            math::inverse_lerp(l.z, r.z, v.z),
            math::inverse_lerp(l.w, r.w, v.w)};
    }

    //
    // contains_nan
    //

    template < typename T >
    bool contains_nan(const vec4<T>& v) noexcept {
        return
            !math::is_finite(v.x) ||
            !math::is_finite(v.y) ||
            !math::is_finite(v.z) ||
            !math::is_finite(v.w);
    }
}

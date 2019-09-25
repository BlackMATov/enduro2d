/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "vec2.hpp"
#include "vec4.hpp"

namespace e2d
{
    template < typename T >
    class vec3 final {
        static_assert(
            std::is_arithmetic_v<T>,
            "type of 'vec3' must be arithmetic");
    public:
        using self_type = vec3;
        using value_type = T;
    public:
        T x = 0;
        T y = 0;
        T z = 0;
    public:
        static const vec3& zero() noexcept;
        static const vec3& unit() noexcept;
        static const vec3& unit_x() noexcept;
        static const vec3& unit_y() noexcept;
        static const vec3& unit_z() noexcept;
    public:
        vec3() noexcept = default;
        vec3(const vec3& other) noexcept = default;
        vec3& operator=(const vec3& other) noexcept = default;

        explicit vec3(T v) noexcept;
        vec3(T x, T y, T z) noexcept;
        explicit vec3(const vec2<T>& other, T z) noexcept;
        explicit vec3(const vec4<T>& other) noexcept;

        template < typename To >
        vec3<To> cast_to() const noexcept;

        T* data() noexcept;
        const T* data() const noexcept;

        T& operator[](std::size_t index) noexcept;
        T operator[](std::size_t index) const noexcept;

        vec3& operator+=(T v) noexcept;
        vec3& operator-=(T v) noexcept;
        vec3& operator*=(T v) noexcept;
        vec3& operator/=(T v) noexcept;

        vec3& operator+=(const vec3& other) noexcept;
        vec3& operator-=(const vec3& other) noexcept;
        vec3& operator*=(const vec3& other) noexcept;
        vec3& operator/=(const vec3& other) noexcept;
    };
}

namespace e2d
{
    template < typename T >
    const vec3<T>& vec3<T>::zero() noexcept {
        static const vec3<T> zero{0, 0, 0};
        return zero;
    }

    template < typename T >
    const vec3<T>& vec3<T>::unit() noexcept {
        static const vec3<T> unit{1, 1, 1};
        return unit;
    }

    template < typename T >
    const vec3<T>& vec3<T>::unit_x() noexcept {
        static const vec3<T> unit_x{1, 0, 0};
        return unit_x;
    }

    template < typename T >
    const vec3<T>& vec3<T>::unit_y() noexcept {
        static const vec3<T> unit_y{0, 1, 0};
        return unit_y;
    }

    template < typename T >
    const vec3<T>& vec3<T>::unit_z() noexcept {
        static const vec3<T> unit_z{0, 0, 1};
        return unit_z;
    }

    template < typename T >
    vec3<T>::vec3(T v) noexcept
    : x(v)
    , y(v)
    , z(v) {}

    template < typename T >
    vec3<T>::vec3(T nx, T ny, T nz) noexcept
    : x(nx)
    , y(ny)
    , z(nz) {}

    template < typename T >
    vec3<T>::vec3(const vec2<T>& other, T nz) noexcept
    : x(other.x)
    , y(other.y)
    , z(nz) {}

    template < typename T >
    vec3<T>::vec3(const vec4<T>& other) noexcept
    : x(other.x)
    , y(other.y)
    , z(other.z) {}

    template < typename T >
    template < typename To >
    vec3<To> vec3<T>::cast_to() const noexcept {
        return {
            math::numeric_cast<To>(x),
            math::numeric_cast<To>(y),
            math::numeric_cast<To>(z)};
    }

    template < typename T >
    T* vec3<T>::data() noexcept {
        return &x;
    }

    template < typename T >
    const T* vec3<T>::data() const noexcept {
        return &x;
    }

    template < typename T >
    T& vec3<T>::operator[](std::size_t index) noexcept {
        E2D_ASSERT(index < 3);
        return data()[index];
    }

    template < typename T >
    T vec3<T>::operator[](std::size_t index) const noexcept {
        E2D_ASSERT(index < 3);
        return data()[index];
    }

    template < typename T >
    vec3<T>& vec3<T>::operator+=(T v) noexcept {
        x += v;
        y += v;
        z += v;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator-=(T v) noexcept {
        x -= v;
        y -= v;
        z -= v;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator*=(T v) noexcept {
        x *= v;
        y *= v;
        z *= v;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator/=(T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        x /= v;
        y /= v;
        z /= v;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator+=(const vec3& other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator-=(const vec3& other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator*=(const vec3& other) noexcept {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    template < typename T >
    vec3<T>& vec3<T>::operator/=(const vec3& other) noexcept {
        E2D_ASSERT(!math::is_near_zero(other.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(other.z, T(0)));
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }
}

namespace e2d
{
    //
    // make_vec3
    //

    template < typename T >
    vec3<T> make_vec3(T v) noexcept {
        return vec3<T>(v);
    }

    template < typename T >
    vec3<T> make_vec3(T x, T y, T z) noexcept {
        return vec3<T>(x, y, z);
    }

    template < typename T >
    vec3<T> make_vec3(const vec2<T>& other, T z) noexcept {
        return vec3<T>(other, z);
    }

    template < typename T >
    vec3<T> make_vec3(const vec4<T>& other) noexcept {
        return vec3<T>(other);
    }

    //
    // vec3 (==,!=) vec3
    //

    template < typename T >
    bool operator==(const vec3<T>& l, const vec3<T>& r) noexcept {
        return
            math::approximately(l.x, r.x) &&
            math::approximately(l.y, r.y) &&
            math::approximately(l.z, r.z);
    }

    template < typename T >
    bool operator!=(const vec3<T>& l, const vec3<T>& r) noexcept {
        return !(l == r);
    }

    //
    // vec3 (<,>,<=,>=) vec3
    //

    template < typename T >
    bool operator<(const vec3<T>& l, const vec3<T>& r) noexcept {
        return
            (l.x < r.x) ||
            (l.x == r.x && l.y < r.y) ||
            (l.x == r.x && l.y == r.y && l.z < r.z);
    }

    template < typename T >
    bool operator>(const vec3<T>& l, const vec3<T>& r) noexcept {
        return r < l;
    }

    template < typename T >
    bool operator<=(const vec3<T>& l, const vec3<T>& r) noexcept {
        return !(r < l);
    }

    template < typename T >
    bool operator>=(const vec3<T>& l, const vec3<T>& r) noexcept {
        return !(l < r);
    }

    //
    // (-) vec3
    //

    template < typename T >
    vec3<T> operator-(const vec3<T>& v) noexcept {
        return {
            -v.x,
            -v.y,
            -v.z};
    }

    //
    // vec3 (+,-,*,/) value
    //

    template < typename T >
    vec3<T> operator+(const vec3<T>& l, T v) noexcept {
        return {
            l.x + v,
            l.y + v,
            l.z + v};
    }

    template < typename T >
    vec3<T> operator-(const vec3<T>& l, T v) noexcept {
        return {
            l.x - v,
            l.y - v,
            l.z - v};
    }

    template < typename T >
    vec3<T> operator*(const vec3<T>& l, T v) noexcept {
        return {
            l.x * v,
            l.y * v,
            l.z * v};
    }

    template < typename T >
    vec3<T> operator/(const vec3<T>& l, T v) noexcept {
        E2D_ASSERT(!math::is_near_zero(v, T(0)));
        return {
            l.x / v,
            l.y / v,
            l.z / v};
    }

    //
    // value (+,-,*,/) vec3
    //

    template < typename T >
    vec3<T> operator+(T v, const vec3<T>& r) noexcept {
        return {
            v + r.x,
            v + r.y,
            v + r.z};
    }

    template < typename T >
    vec3<T> operator-(T v, const vec3<T>& r) noexcept {
        return {
            v - r.x,
            v - r.y,
            v - r.z};
    }

    template < typename T >
    vec3<T> operator*(T v, const vec3<T>& r) noexcept {
        return {
            v * r.x,
            v * r.y,
            v * r.z};
    }

    template < typename T >
    vec3<T> operator/(T v, const vec3<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.z, T(0)));
        return {
            v / r.x,
            v / r.y,
            v / r.z};
    }

    //
    // vec3 (+,-,*,/) vec3
    //

    template < typename T >
    vec3<T> operator+(const vec3<T>& l, const vec3<T>& r) noexcept {
        return {
            l.x + r.x,
            l.y + r.y,
            l.z + r.z};
    }

    template < typename T >
    vec3<T> operator-(const vec3<T>& l, const vec3<T>& r) noexcept {
        return {
            l.x - r.x,
            l.y - r.y,
            l.z - r.z};
    }

    template < typename T >
    vec3<T> operator*(const vec3<T>& l, const vec3<T>& r) noexcept {
        return {
            l.x * r.x,
            l.y * r.y,
            l.z * r.z};
    }

    template < typename T >
    vec3<T> operator/(const vec3<T>& l, const vec3<T>& r) noexcept {
        E2D_ASSERT(!math::is_near_zero(r.x, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.y, T(0)));
        E2D_ASSERT(!math::is_near_zero(r.z, T(0)));
        return {
            l.x / r.x,
            l.y / r.y,
            l.z / r.z};
    }
}

namespace e2d::math
{
    //
    // approximately
    //

    template < typename T >
    bool approximately(
        const vec3<T>& l,
        const vec3<T>& r,
        T precision = math::default_precision<T>()) noexcept
    {
        return
            math::approximately(l.x, r.x, precision) &&
            math::approximately(l.y, r.y, precision) &&
            math::approximately(l.z, r.z, precision);
    }

    //
    // dot
    //

    template < typename T >
    T dot(const vec3<T>& l, const vec3<T>& r) noexcept {
        return
            l.x * r.x +
            l.y * r.y +
            l.z * r.z;
    }

    template < typename T >
    T abs_dot(const vec3<T>& l, const vec3<T>& r) noexcept {
        return
            math::abs(l.x * r.x) +
            math::abs(l.y * r.y) +
            math::abs(l.z * r.z);
    }

    //
    // cross
    //

    template < typename T >
    vec3<T> cross(const vec3<T>& l, const vec3<T>& r) noexcept {
        return {
            l.y * r.z - l.z * r.y,
            l.z * r.x - l.x * r.z,
            l.x * r.y - l.y * r.x};
    }

    //
    // length
    //

    template < typename T >
    T length_squared(const vec3<T>& v) noexcept {
        return dot(v, v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    length(const vec3<T>& v) noexcept {
        return math::sqrt(length_squared(v));
    }

    //
    // distance
    //

    template < typename T >
    math::make_distance_t<T>
    distance_squared(const vec3<T>& l, const vec3<T>& r) noexcept {
        return length_squared(make_vec3(
            math::distance(l.x, r.x),
            math::distance(l.y, r.y),
            math::distance(l.z, r.z)));
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    distance(const vec3<T>& l, const vec3<T>& r) noexcept {
        return math::sqrt(distance_squared(l, r));
    }

    //
    // normalized
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, vec3<T>>
    normalized(
        const vec3<T>& v,
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
    T minimum(const vec3<T>& v) noexcept {
        return math::min(v.x, v.y, v.z);
    }

    template < typename T >
    T maximum(const vec3<T>& v) noexcept {
        return math::max(v.x, v.y, v.z);
    }

    //
    // minimized/maximized/clamped
    //

    template < typename T >
    vec3<T> minimized(const vec3<T>& v, const vec3<T>& vmin) noexcept {
        return {
            math::min(v.x, vmin.x),
            math::min(v.y, vmin.y),
            math::min(v.z, vmin.z)};
    }

    template < typename T >
    vec3<T> maximized(const vec3<T>& v, const vec3<T>& vmax) noexcept {
        return {
            math::max(v.x, vmax.x),
            math::max(v.y, vmax.y),
            math::max(v.z, vmax.z)};
    }

    template < typename T >
    vec3<T> clamped(const vec3<T>& v, const vec3<T>& vmin, const vec3<T>& vmax) noexcept {
        return {
            math::clamp(v.x, vmin.x, vmax.x),
            math::clamp(v.y, vmin.y, vmax.y),
            math::clamp(v.z, vmin.z, vmax.z)};
    }

    template < typename T >
    vec3<T> saturated(const vec3<T>& v) noexcept {
        return clamped(v, vec3<T>::zero(), vec3<T>::unit());
    }

    //
    // lerp/inverse_lerp
    //

    template < typename T >
    vec3<T> lerp(const vec3<T>& l, const vec3<T>& r, T v) noexcept {
        return {
            math::lerp(l.x, r.x, v),
            math::lerp(l.y, r.y, v),
            math::lerp(l.z, r.z, v)};
    }

    template < typename T >
    vec3<T> lerp(const vec3<T>& l, const vec3<T>& r, const vec3<T>& v) noexcept {
        return {
            math::lerp(l.x, r.x, v.x),
            math::lerp(l.y, r.y, v.y),
            math::lerp(l.z, r.z, v.z)};
    }

    template < typename T >
    vec3<T> inverse_lerp(const vec3<T>& l, const vec3<T>& r, const vec3<T>& v) noexcept {
        return {
            math::inverse_lerp(l.x, r.x, v.x),
            math::inverse_lerp(l.y, r.y, v.y),
            math::inverse_lerp(l.z, r.z, v.z)};
    }

    //
    // contains_nan
    //

    template < typename T >
    bool contains_nan(const vec3<T>& v) noexcept {
        return
            !math::is_finite(v.x) ||
            !math::is_finite(v.y) ||
            !math::is_finite(v.z);
    }
}

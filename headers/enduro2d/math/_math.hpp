/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../base/_all.hpp"

namespace e2d
{
    template < typename T >
    class vec2;

    template < typename T >
    class vec3;

    template < typename T >
    class vec4;

    template < typename T >
    class mat2;

    template < typename T >
    class mat3;

    template < typename T >
    class mat4;

    template < typename T >
    class quat;

    template < typename T >
    class rect;

    template < typename T >
    class aabb;

    template < typename T >
    class trs2;

    template < typename T >
    class trs3;

    template < typename T, typename Tag >
    class unit;

    template < typename FromTag, typename ToTag >
    struct unit_converter;
}

namespace e2d
{
    using v2d = vec2<f64>;
    using v2f = vec2<f32>;
    using v2i = vec2<i32>;
    using v2u = vec2<u32>;
    using v2hi = vec2<i16>;
    using v2hu = vec2<u16>;

    using v3d = vec3<f64>;
    using v3f = vec3<f32>;
    using v3i = vec3<i32>;
    using v3u = vec3<u32>;
    using v3hi = vec3<i16>;
    using v3hu = vec3<u16>;

    using v4d = vec4<f64>;
    using v4f = vec4<f32>;
    using v4i = vec4<i32>;
    using v4u = vec4<u32>;
    using v4hi = vec4<i16>;
    using v4hu = vec4<u16>;

    using m2d = mat2<f64>;
    using m2f = mat2<f32>;
    using m2i = mat2<i32>;
    using m2u = mat2<u32>;
    using m2hi = mat2<i16>;
    using m2hu = mat2<u16>;

    using m3d = mat3<f64>;
    using m3f = mat3<f32>;
    using m3i = mat3<i32>;
    using m3u = mat3<u32>;
    using m3hi = mat3<i16>;
    using m3hu = mat3<u16>;

    using m4d = mat4<f64>;
    using m4f = mat4<f32>;
    using m4i = mat4<i32>;
    using m4u = mat4<u32>;
    using m4hi = mat4<i16>;
    using m4hu = mat4<u16>;

    using q4d = quat<f64>;
    using q4f = quat<f32>;
    using q4i = quat<i32>;
    using q4u = quat<u32>;
    using q4hi = quat<i16>;
    using q4hu = quat<u16>;

    using b2d = rect<f64>;
    using b2f = rect<f32>;
    using b2i = rect<i32>;
    using b2u = rect<u32>;
    using b2hi = rect<i16>;
    using b2hu = rect<u16>;

    using b3d = aabb<f64>;
    using b3f = aabb<f32>;
    using b3i = aabb<i32>;
    using b3u = aabb<u32>;
    using b3hi = aabb<i16>;
    using b3hu = aabb<u16>;

    using t2d = trs2<f64>;
    using t2f = trs2<f32>;
    using t2i = trs2<i32>;
    using t2u = trs2<u32>;
    using t2hi = trs2<i16>;
    using t2hu = trs2<u16>;

    using t3d = trs3<f64>;
    using t3f = trs3<f32>;
    using t3i = trs3<i32>;
    using t3u = trs3<u32>;
    using t3hi = trs3<i16>;
    using t3hu = trs3<u16>;

    struct deg_tag {};
    struct rad_tag {};

    template < typename T >
    using deg = unit<T, deg_tag>;
    template < typename T >
    using rad = unit<T, rad_tag>;
}

namespace e2d::math
{
    //
    // ceil/floor/round/trunc
    //

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    ceil(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    floor(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    round(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    trunc(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    ceil(T v) noexcept {
        return std::ceil(v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    floor(T v) noexcept {
        return std::floor(v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    round(T v) noexcept {
        return std::round(v);
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    trunc(T v) noexcept {
        return std::trunc(v);
    }

    //
    // numeric cast
    //

    namespace impl
    {
        // i -> i, u -> u, f -> f
        template < typename To, typename From >
        std::enable_if_t<
            std::is_signed_v<From> == std::is_signed_v<To> &&
            std::is_integral_v<From> == std::is_integral_v<To> &&
            std::is_arithmetic_v<From> && std::is_arithmetic_v<To>,
            bool>
        check_numeric_cast(From v) noexcept {
            const To l = std::numeric_limits<To>::lowest();
            const To m = std::numeric_limits<To>::max();
            using U = std::common_type_t<To, From>;
            return
                static_cast<U>(v) >= static_cast<U>(l) &&
                static_cast<U>(v) <= static_cast<U>(m);
        }

        // i/u -> f
        template < typename To, typename From >
        std::enable_if_t<
            std::is_integral_v<From> && std::is_floating_point_v<To>,
            bool>
        check_numeric_cast(From v) noexcept {
            E2D_UNUSED(v);
            return true;
        }

        // f -> i/u
        template < typename To, typename From >
        std::enable_if_t<
            std::is_floating_point_v<From> && std::is_integral_v<To>,
            bool>
        check_numeric_cast(From v) noexcept {
            const From t = trunc(v);
            const To l = std::numeric_limits<To>::lowest();
            const To m = std::numeric_limits<To>::max();
            return t >= static_cast<From>(l) && t <= static_cast<From>(m);
        }

        // i -> u
        template < typename To, typename From >
        std::enable_if_t<
            std::is_signed_v<From> && std::is_unsigned_v<To> &&
            std::is_integral_v<From> && std::is_integral_v<To>,
            bool>
        check_numeric_cast(From v) noexcept {
            const To m = std::numeric_limits<To>::max();
            return v >= 0 && static_cast<std::make_unsigned_t<From>>(v) <= m;
        }

        // u -> i
        template < typename To, typename From >
        std::enable_if_t<
            std::is_unsigned_v<From> && std::is_signed_v<To> &&
            std::is_integral_v<From> && std::is_integral_v<To>,
            bool>
        check_numeric_cast(From v) noexcept {
            const To m = std::numeric_limits<To>::max();
            return v <= static_cast<std::make_unsigned_t<To>>(m);
        }
    }

    template < typename To, typename From >
    std::enable_if_t<
        std::is_arithmetic_v<From> && std::is_arithmetic_v<To>,
        To>
    numeric_cast(From v) noexcept {
        E2D_ASSERT(impl::check_numeric_cast<To>(v));
        return static_cast<To>(v);
    }

    //
    // default precision
    //

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    default_precision() noexcept {
        return 0;
    }

    template < typename T >
    std::enable_if_t<std::is_same_v<T, f32>, T>
    default_precision() noexcept {
        return 0.00001f;
    }

    template < typename T >
    std::enable_if_t<std::is_same_v<T, f64>, T>
    default_precision() noexcept {
        return 0.0000001;
    }

    //
    // bit flags
    //

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        T>
    set_flags(T flags, U flag_mask) noexcept {
        return flags | flag_mask;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        void>
    set_flags_inplace(T& flags, U flag_mask) noexcept {
        flags = set_flags(flags, flag_mask);
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        T>
    flip_flags(T flags, U flag_mask) noexcept {
        return flags ^ flag_mask;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        void>
    flip_flags_inplace(T& flags, U flag_mask) noexcept {
        flags = flip_flags(flags, flag_mask);
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        T>
    clear_flags(T flags, U flag_mask) noexcept {
        return flags & ~flag_mask;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        void>
    clear_flags_inplace(T& flags, U flag_mask) noexcept {
        flags = clear_flags(flags, flag_mask);
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_any_flags(T flags, U flag_mask) noexcept {
        return !!(flags & flag_mask);
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_all_flags(T flags, U flag_mask) noexcept {
        return flag_mask == (flags & flag_mask);
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_and_set_any_flags(T& flags, U flag_mask) noexcept {
        if ( flag_mask == (flags & flag_mask) ) {
            return false;
        }
        flags |= flag_mask;
        return true;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_and_set_all_flags(T& flags, U flag_mask) noexcept {
        if ( !!(flags & flag_mask) ) {
            return false;
        }
        flags |= flag_mask;
        return true;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_and_clear_any_flags(T& flags, U flag_mask) noexcept {
        if ( !(flags & flag_mask) ) {
            return false;
        }
        flags &= ~flag_mask;
        return true;
    }

    template < typename T, typename U >
    std::enable_if_t<
        std::is_unsigned_v<T> &&
        std::is_convertible_v<U,T>,
        bool>
    check_and_clear_all_flags(T& flags, U flag_mask) noexcept {
        if ( flag_mask != (flags & flag_mask) ) {
            return false;
        }
        flags &= ~flag_mask;
        return true;
    }

    //
    // power of two
    //

    template < typename T >
    std::enable_if_t<std::is_unsigned_v<T>, bool>
    is_power_of_2(T v) noexcept {
        return v && !(v & (v - 1));
    }

    template < typename T >
    std::enable_if_t<std::is_unsigned_v<T>, T>
    max_power_of_2() noexcept {
        return T(1) << (sizeof(T) * 8 - 1);
    }

    template < typename T >
    std::enable_if_t<std::is_unsigned_v<T>, T>
    next_power_of_2(T v) noexcept {
        E2D_ASSERT(v <= max_power_of_2<T>());
        if ( v == 0 ) {
            return 1;
        }
        --v;
        u32 i = 1;
        do {
            v |= v >> i;
            i <<= 1;
        } while (v & (v + 1));
        return v + 1;
    }

    //
    // is_finite
    //

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, bool>
    is_finite(T v) noexcept {
        E2D_UNUSED(v);
        return true;
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, bool>
    is_finite(T v) noexcept {
        return std::isfinite(v);
    }

    //
    // abs/abs_to_unsigned
    //

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_signed_v<T>,
        T>
    abs(T v) noexcept {
        E2D_ASSERT(v > std::numeric_limits<T>::lowest());
        return v < 0 ? -v : v;
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_unsigned_v<T>,
        T>
    abs(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    abs(T v) noexcept {
        return std::abs(v);
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_signed_v<T>,
        std::make_unsigned_t<T>>
    abs_to_unsigned(T v) noexcept {
        return v >= 0
            ? static_cast<std::make_unsigned_t<T>>(v)
            : static_cast<std::make_unsigned_t<T>>(-(v + 1)) + 1;
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_unsigned_v<T>,
        T>
    abs_to_unsigned(T v) noexcept {
        return v;
    }

    //
    // mod
    //

    template < typename T >
    std::enable_if_t<std::is_integral_v<T>, T>
    mod(T x, T y) noexcept {
        E2D_ASSERT(y != T(0));
        return x % y;
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    mod(T x, T y) noexcept {
        E2D_ASSERT(y != T(0));
        return std::fmod(x, y);
    }

    //
    // sign
    //

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_signed_v<T>,
        bool>
    sign(T v) noexcept {
        return v < 0;
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_unsigned_v<T>,
        bool>
    sign(T v) noexcept {
        E2D_UNUSED(v);
        return false;
    }

    template < typename T >
    std::enable_if_t<
        std::is_floating_point_v<T>,
        bool>
    sign(T v) noexcept {
        return std::signbit(v);
    }

    //
    // sqrt
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, T>
    sqrt(T v) noexcept {
        return std::sqrt(v);
    }

    //
    // min
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    min(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    min(T l, T r) noexcept {
        return l < r ? l : r;
    }

    template < typename T, typename... Ts >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    min(T a, T b, T c, Ts... ts) noexcept {
        return min(a < b ? a : b, c, ts...);
    }

    //
    // max
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    max(T v) noexcept {
        return v;
    }

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    max(T l, T r) noexcept {
        return l < r ? r : l;
    }

    template < typename T, typename... Ts >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    max(T a, T b, T c, Ts... ts) noexcept {
        return max(a < b ? b : a, c, ts...);
    }

    //
    // minmax
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, std::pair<T,T>>
    minmax(T v) noexcept {
        return std::make_pair(v, v);
    }

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, std::pair<T,T>>
    minmax(T l, T r) noexcept {
        return l < r
            ? std::make_pair(l, r)
            : std::make_pair(r, l);
    }

    template < typename T, typename... Ts >
    std::enable_if_t<std::is_arithmetic_v<T>, std::pair<T,T>>
    minmax(T a, T b, T c, Ts... ts) noexcept {
        return std::make_pair(
            min(a, b, c, ts...),
            max(a, b, c, ts...));
    }

    //
    // clamp/saturate
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    clamp(T v, T vmin, T vmax) noexcept {
        std::tie(vmin, vmax) = minmax(vmin, vmax);
        return min(max(v, vmin), vmax);
    }

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, T>
    saturate(T v) noexcept {
        return clamp(v, T(0), T(1));
    }

    //
    // make_distance/distance
    //

    namespace impl
    {
        template < typename T, typename = void >
        struct make_distance_impl;

        template < typename T >
        struct make_distance_impl<
            T,
            std::enable_if_t<std::is_integral_v<T>>>
        {
            using type = std::make_unsigned_t<T>;
        };

        template < typename T >
        struct make_distance_impl<
            T,
            std::enable_if_t<std::is_floating_point_v<T>>>
        {
            using type = T;
        };
    }

    template < typename T >
    using make_distance = impl::make_distance_impl<T>;

    template < typename T >
    using make_distance_t = typename make_distance<T>::type;

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> && std::is_signed_v<T>,
        make_distance_t<T>>
    distance(T l, T r) noexcept {
        std::tie(l, r) = minmax(l, r);
        return r < 0 || l >= 0
            ? abs_to_unsigned<T>(r - l)
            : abs_to_unsigned(l) + abs_to_unsigned(r);
    }

    template < typename T >
    std::enable_if_t<
        std::is_unsigned_v<T> || std::is_floating_point_v<T>,
        make_distance_t<T>>
    distance(T l, T r) noexcept {
        std::tie(l, r) = minmax(l, r);
        return r - l;
    }

    //
    // approximately
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, bool>
    approximately(T l, T r, T precision = default_precision<T>()) noexcept {
        return distance(l, r) <= numeric_cast<make_distance_t<T>>(abs(precision));
    }

    //
    // is_near_zero
    //

    template < typename T >
    std::enable_if_t<std::is_arithmetic_v<T>, bool>
    is_near_zero(T v, T precision = default_precision<T>()) noexcept {
        return approximately(v, T(0), precision);
    }

    //
    // lerp/inverse_lerp
    //

    template < typename T >
    T lerp(T l, T r, T v) noexcept {
        return l + (r - l) * v;
    }

    template < typename T >
    T inverse_lerp(T l, T r, T v) noexcept {
        E2D_ASSERT(!approximately(l, r, T(0)));
        return (v - l) / (r - l);
    }
}

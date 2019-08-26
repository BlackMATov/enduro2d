/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "quat.hpp"
#include "trig.hpp"
#include "trs2.hpp"
#include "trs3.hpp"
#include "unit.hpp"
#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

namespace e2d
{
    template < typename T >
    class mat4 final {
        static_assert(
            std::is_arithmetic_v<T>,
            "type of 'mat4' must be arithmetic");
    public:
        using self_type = mat4;
        using value_type = T;
    public:
        vec4<T> rows[4] = {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}};
    public:
        static const mat4& zero() noexcept;
        static const mat4& identity() noexcept;
    public:
        mat4() noexcept = default;
        mat4(const mat4& other) noexcept = default;
        mat4& operator=(const mat4& other) noexcept = default;

        mat4(const vec4<T>& row0,
             const vec4<T>& row1,
             const vec4<T>& row2,
             const vec4<T>& row3) noexcept;

        mat4(T m11, T m12, T m13, T m14,
             T m21, T m22, T m23, T m24,
             T m31, T m32, T m33, T m34,
             T m41, T m42, T m43, T m44) noexcept;

        template < typename To >
        mat4<To> cast_to() const noexcept;

        T* data() noexcept;
        const T* data() const noexcept;

        vec4<T>& operator[](std::size_t row) noexcept;
        const vec4<T>& operator[](std::size_t row) const noexcept;

        mat4& operator+=(T v) noexcept;
        mat4& operator*=(T v) noexcept;

        mat4& operator+=(const mat4& other) noexcept;
        mat4& operator*=(const mat4& other) noexcept;
    };
}

namespace e2d
{
    template < typename T >
    const mat4<T>& mat4<T>::zero() noexcept {
        static const mat4<T> zero{
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0};
        return zero;
    }

    template < typename T >
    const mat4<T>& mat4<T>::identity() noexcept {
        static const mat4<T> identity{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
        return identity;
    }

    template < typename T >
    mat4<T>::mat4(
        const vec4<T>& row0,
        const vec4<T>& row1,
        const vec4<T>& row2,
        const vec4<T>& row3) noexcept
    : rows{row0, row1, row2, row3} {}

    template < typename T >
    mat4<T>::mat4(
        T m11, T m12, T m13, T m14,
        T m21, T m22, T m23, T m24,
        T m31, T m32, T m33, T m34,
        T m41, T m42, T m43, T m44) noexcept
    : rows{{m11, m12, m13, m14},
           {m21, m22, m23, m24},
           {m31, m32, m33, m34},
           {m41, m42, m43, m44}} {}

    template < typename T >
    template < typename To >
    mat4<To> mat4<T>::cast_to() const noexcept {
        return {
            rows[0].template cast_to<To>(),
            rows[1].template cast_to<To>(),
            rows[2].template cast_to<To>(),
            rows[3].template cast_to<To>()};
    }

    template < typename T >
    T* mat4<T>::data() noexcept {
        return rows[0].data();
    }

    template < typename T >
    const T* mat4<T>::data() const noexcept {
        return rows[0].data();
    }

    template < typename T >
    vec4<T>& mat4<T>::operator[](std::size_t row) noexcept {
        E2D_ASSERT(row < 4);
        return rows[row];
    }

    template < typename T >
    const vec4<T>& mat4<T>::operator[](std::size_t row) const noexcept {
        E2D_ASSERT(row < 4);
        return rows[row];
    }

    template < typename T >
    mat4<T>& mat4<T>::operator+=(T v) noexcept {
        return *this = *this + v;
    }

    template < typename T >
    mat4<T>& mat4<T>::operator*=(T v) noexcept {
        return *this = *this * v;
    }

    template < typename T >
    mat4<T>& mat4<T>::operator+=(const mat4& other) noexcept {
        return *this = *this + other;
    }

    template < typename T >
    mat4<T>& mat4<T>::operator*=(const mat4& other) noexcept {
        return *this = *this * other;
    }
}

namespace e2d
{
    //
    // make_mat4
    //

    template < typename T >
    mat4<T> make_mat4(
        const vec4<T>& row0,
        const vec4<T>& row1,
        const vec4<T>& row2,
        const vec4<T>& row3) noexcept
    {
        return mat4<T>(row0, row1, row2, row3);
    }

    template < typename T >
    mat4<T> make_mat4(
        T m11, T m12, T m13, T m14,
        T m21, T m22, T m23, T m24,
        T m31, T m32, T m33, T m34,
        T m41, T m42, T m43, T m44) noexcept
    {
        return mat4<T>(
            m11, m12, m13, m14,
            m21, m22, m23, m24,
            m31, m32, m33, m34,
            m41, m42, m43, m44);
    }

    //
    // mat4 (==,!=) mat4
    //

    template < typename T >
    bool operator==(const mat4<T>& l, const mat4<T>& r) noexcept {
        return
            l.rows[0] == r.rows[0] &&
            l.rows[1] == r.rows[1] &&
            l.rows[2] == r.rows[2] &&
            l.rows[3] == r.rows[3];
    }

    template < typename T >
    bool operator!=(const mat4<T>& l, const mat4<T>& r) noexcept {
        return !(l == r);
    }

    //
    // (-) mat4
    //

    template < typename T >
    mat4<T> operator-(const mat4<T>& m) noexcept {
        return {
            -m.rows[0],
            -m.rows[1],
            -m.rows[2],
            -m.rows[3]};
    }

    //
    // mat4 (+,*) value
    //

    template < typename T >
    mat4<T> operator+(const mat4<T>& l, T v) noexcept {
        return {
            l.rows[0] + v,
            l.rows[1] + v,
            l.rows[2] + v,
            l.rows[3] + v};
    }

    template < typename T >
    mat4<T> operator*(const mat4<T>& l, T v) noexcept {
        return {
            l.rows[0] * v,
            l.rows[1] * v,
            l.rows[2] * v,
            l.rows[3] * v};
    }

    //
    // value (+,*) mat4
    //

    template < typename T >
    mat4<T> operator+(T v, const mat4<T>& r) noexcept {
        return {
            v + r.rows[0],
            v + r.rows[1],
            v + r.rows[2],
            v + r.rows[3]};
    }

    template < typename T >
    mat4<T> operator*(T v, const mat4<T>& r) noexcept {
        return {
            v * r.rows[0],
            v * r.rows[1],
            v * r.rows[2],
            v * r.rows[3]};
    }

    //
    // mat4 (+,*) mat4
    //

    template < typename T >
    mat4<T> operator+(const mat4<T>& l, const mat4<T>& r) noexcept {
        return {
            l.rows[0] + r.rows[0],
            l.rows[1] + r.rows[1],
            l.rows[2] + r.rows[2],
            l.rows[3] + r.rows[3]};
    }

    template < typename T >
    mat4<T> operator*(const mat4<T>& l, const mat4<T>& r) noexcept {
        const T* const lm = l.data();
        const T* const rm = r.data();
        return {
            lm[ 0] * rm[0] + lm[ 1] * rm[4] + lm[ 2] * rm[ 8] + lm[ 3] * rm[12],
            lm[ 0] * rm[1] + lm[ 1] * rm[5] + lm[ 2] * rm[ 9] + lm[ 3] * rm[13],
            lm[ 0] * rm[2] + lm[ 1] * rm[6] + lm[ 2] * rm[10] + lm[ 3] * rm[14],
            lm[ 0] * rm[3] + lm[ 1] * rm[7] + lm[ 2] * rm[11] + lm[ 3] * rm[15],

            lm[ 4] * rm[0] + lm[ 5] * rm[4] + lm[ 6] * rm[ 8] + lm[ 7] * rm[12],
            lm[ 4] * rm[1] + lm[ 5] * rm[5] + lm[ 6] * rm[ 9] + lm[ 7] * rm[13],
            lm[ 4] * rm[2] + lm[ 5] * rm[6] + lm[ 6] * rm[10] + lm[ 7] * rm[14],
            lm[ 4] * rm[3] + lm[ 5] * rm[7] + lm[ 6] * rm[11] + lm[ 7] * rm[15],

            lm[ 8] * rm[0] + lm[ 9] * rm[4] + lm[10] * rm[ 8] + lm[11] * rm[12],
            lm[ 8] * rm[1] + lm[ 9] * rm[5] + lm[10] * rm[ 9] + lm[11] * rm[13],
            lm[ 8] * rm[2] + lm[ 9] * rm[6] + lm[10] * rm[10] + lm[11] * rm[14],
            lm[ 8] * rm[3] + lm[ 9] * rm[7] + lm[10] * rm[11] + lm[11] * rm[15],

            lm[12] * rm[0] + lm[13] * rm[4] + lm[14] * rm[ 8] + lm[15] * rm[12],
            lm[12] * rm[1] + lm[13] * rm[5] + lm[14] * rm[ 9] + lm[15] * rm[13],
            lm[12] * rm[2] + lm[13] * rm[6] + lm[14] * rm[10] + lm[15] * rm[14],
            lm[12] * rm[3] + lm[13] * rm[7] + lm[14] * rm[11] + lm[15] * rm[15]};
    }

    //
    // vec4 (*) mat4
    //

    template < typename T >
    vec4<T> operator*(const vec4<T>& l, const mat4<T>& r) noexcept {
        const T* const rm = r.data();
        return {
            l.x * rm[0] + l.y * rm[4] + l.z * rm[8]  + l.w * rm[12],
            l.x * rm[1] + l.y * rm[5] + l.z * rm[9]  + l.w * rm[13],
            l.x * rm[2] + l.y * rm[6] + l.z * rm[10] + l.w * rm[14],
            l.x * rm[3] + l.y * rm[7] + l.z * rm[11] + l.w * rm[15]};
    }
}

namespace e2d::math
{
    //
    // make_scale_matrix
    //

    template < typename T >
    mat4<T> make_scale_matrix4(T x, T y, T z = T(1)) noexcept {
        return {
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1};
    }

    template < typename T >
    mat4<T> make_scale_matrix4(const vec4<T>& xyz) noexcept {
        return make_scale_matrix4(
            xyz.x,
            xyz.y,
            xyz.z);
    }

    template < typename T >
    mat4<T> make_scale_matrix4(const vec3<T>& xyz) noexcept {
        return make_scale_matrix4(
            xyz.x,
            xyz.y,
            xyz.z);
    }

    template < typename T >
    mat4<T> make_scale_matrix4(const vec2<T>& xy, T z = T(1)) noexcept {
        return make_scale_matrix4(
            xy.x,
            xy.y,
            z);
    }

    //
    // make_translation_matrix
    //

    template < typename T >
    mat4<T> make_translation_matrix4(T x, T y, T z = T(0)) noexcept {
        return {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            x, y, z, 1};
    }

    template < typename T >
    mat4<T> make_translation_matrix4(const vec4<T>& xyz) noexcept {
        return make_translation_matrix4(
            xyz.x,
            xyz.y,
            xyz.z);
    }

    template < typename T >
    mat4<T> make_translation_matrix4(const vec3<T>& xyz) noexcept {
        return make_translation_matrix4(
            xyz.x,
            xyz.y,
            xyz.z);
    }

    template < typename T >
    mat4<T> make_translation_matrix4(const vec2<T>& xy, T z = T(0)) noexcept {
        return make_translation_matrix4(
            xy.x,
            xy.y,
            z);
    }

    //
    // make_rotation_matrix
    //

    template < typename T, typename AngleTag >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_rotation_matrix4(
        const unit<T, AngleTag>& angle,
        T axis_x,
        T axis_y,
        T axis_z) noexcept
    {
        const T x = axis_x;
        const T y = axis_y;
        const T z = axis_z;
        const T px = x * x;
        const T py = y * y;
        const T pz = z * z;
        const T cs = math::cos(angle);
        const T sn = math::sin(angle);
        const T ics = T(1) - cs;
        const T xym = x * y * ics;
        const T xzm = x * z * ics;
        const T yzm = y * z * ics;
        const T xsn = x * sn;
        const T ysn = y * sn;
        const T zsn = z * sn;
        return {
            px * ics + cs, xym + zsn,     xzm - ysn,     T(0),
            xym - zsn,     py * ics + cs, yzm + xsn,     T(0),
            xzm + ysn,     yzm - xsn,     pz * ics + cs, T(0),
            T(0),          T(0),          T(0),          T(1)};
    }

    template < typename T, typename AngleTag >
    mat4<T> make_rotation_matrix4(
        const unit<T, AngleTag>& angle,
        const vec4<T>& axis_xyz) noexcept
    {
        return make_rotation_matrix4(
            angle,
            axis_xyz.x,
            axis_xyz.y,
            axis_xyz.z);
    }

    template < typename T, typename AngleTag >
    mat4<T> make_rotation_matrix4(
        const unit<T, AngleTag>& angle,
        const vec3<T>& axis_xyz) noexcept
    {
        return make_rotation_matrix4(
            angle,
            axis_xyz.x,
            axis_xyz.y,
            axis_xyz.z);
    }

    template < typename T, typename AngleTag >
    mat4<T> make_rotation_matrix4(
        const unit<T, AngleTag>& angle,
        const vec2<T>& axis_xy,
        T axis_z) noexcept
    {
        return make_rotation_matrix4(
            angle,
            axis_xy.x,
            axis_xy.y,
            axis_z);
    }

    template < typename T >
    mat4<T> make_rotation_matrix4(const quat<T>& q) noexcept {
        const T x = q.x;
        const T y = q.y;
        const T z = q.z;
        const T w = q.w;

        const T xx = x * x;
        const T xy = x * y;
        const T xz = x * z;
        const T xw = x * w;

        const T yy = y * y;
        const T yz = y * z;
        const T yw = y * w;

        const T zz = z * z;
        const T zw = z * w;

        return {
            T(1) - T(2) * (yy + zz), T(2) * (xy + zw),        T(2) * (xz - yw),        T(0),
            T(2) * (xy - zw),        T(1) - T(2) * (xx + zz), T(2) * (yz + xw),        T(0),
            T(2) * (xz + yw),        T(2) * (yz - xw),        T(1) - T(2) * (xx + yy), T(0),
            T(0),                    T(0),                    T(0),                    T(1)};
    }

    //
    // make_trs_matrix4
    //

    template < typename T >
    mat4<T> make_trs_matrix4(const trs2<T>& trs) noexcept {
        return make_scale_matrix4(trs.scale)
             * make_rotation_matrix4(trs.rotation, v4f::unit_z())
             * make_translation_matrix4(trs.translation);
    }

    template < typename T >
    mat4<T> make_trs_matrix4(const trs3<T>& trs) noexcept {
        return make_scale_matrix4(trs.scale)
             * make_rotation_matrix4(trs.rotation)
             * make_translation_matrix4(trs.translation);
    }

    //
    // make_look_at_lh_matrix4
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_look_at_lh_matrix4(const vec3<T>& eye, const vec3<T>& at, const vec3<T>& up) noexcept {
        const vec3<T> az = normalized(at - eye);
        const vec3<T> ax = normalized(math::cross(up, az));
        const vec3<T> ay = math::cross(az, ax);
        const T dx = math::dot(ax, eye);
        const T dy = math::dot(ay, eye);
        const T dz = math::dot(az, eye);
        return {
            ax.x, ay.x, az.x, T(0),
            ax.y, ay.y, az.y, T(0),
            ax.z, ay.z, az.z, T(0),
            -dx,  -dy,  -dz,  T(1)};
    }

    //
    // make_look_at_rh_matrix4
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_look_at_rh_matrix4(const vec3<T>& eye, const vec3<T>& at, const vec3<T>& up) noexcept {
        const vec3<T> az = normalized(eye - at);
        const vec3<T> ax = normalized(math::cross(up, az));
        const vec3<T> ay = math::cross(az, ax);
        const T dx = math::dot(ax, eye);
        const T dy = math::dot(ay, eye);
        const T dz = math::dot(az, eye);
        return {
            ax.x, ay.x, az.x, T(0),
            ax.y, ay.y, az.y, T(0),
            ax.z, ay.z, az.z, T(0),
            dx,   dy,   dz,   T(1)};
    }

    //
    // make_orthogonal_lh_matrix4
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_orthogonal_lh_matrix4(T width, T height, T znear, T zfar) noexcept {
        E2D_ASSERT(!math::is_near_zero(width, T(0)));
        E2D_ASSERT(!math::is_near_zero(height, T(0)));
        E2D_ASSERT(!math::approximately(znear, zfar, T(0)));
        const T sx = T(2) / width;
        const T sy = T(2) / height;
        const T sz = T(1) / (zfar - znear);
        const T tz = -znear * sz;
        return {
            sx,   T(0), T(0), T(0),
            T(0), sy,   T(0), T(0),
            T(0), T(0), sz,   T(0),
            T(0), T(0), tz,   T(1)};
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_orthogonal_lh_matrix4(const vec2<T>& size, T znear, T zfar) {
        return make_orthogonal_lh_matrix4(size.x, size.y, znear, zfar);
    }

    //
    // make_orthogonal_rh_matrix4
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_orthogonal_rh_matrix4(T width, T height, T znear, T zfar) noexcept {
        E2D_ASSERT(!math::is_near_zero(width, T(0)));
        E2D_ASSERT(!math::is_near_zero(height, T(0)));
        E2D_ASSERT(!math::approximately(znear, zfar, T(0)));
        const T sx = T(2) / width;
        const T sy = T(2) / height;
        const T sz = T(1) / (znear - zfar);
        const T tz = znear * sz;
        return {
            sx,   T(0), T(0), T(0),
            T(0), sy,   T(0), T(0),
            T(0), T(0), sz,   T(0),
            T(0), T(0), tz,   T(1)};
    }

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_orthogonal_rh_matrix4(const vec2<T>& size, T znear, T zfar) {
        return make_orthogonal_rh_matrix4(size.x, size.y, znear, zfar);
    }

    //
    // make_perspective_lh_matrix4
    //

    template < typename T, typename AngleTag >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_perspective_lh_matrix4(const unit<T, AngleTag>& fov, T aspect, T znear, T zfar) noexcept {
        E2D_ASSERT(!math::is_near_zero(aspect, T(0)));
        E2D_ASSERT(!math::approximately(znear, zfar, T(0)));
        E2D_ASSERT(!math::approximately(to_rad(fov), make_rad<T>(0), T(0)));
        const T sy = T(1) / math::tan(fov * T(0.5));
        const T sx = sy / aspect;
        const T sz = zfar / (zfar - znear);
        const T tz = -znear * zfar / (zfar - znear);
        return {
            sx,   T(0), T(0), T(0),
            T(0), sy,   T(0), T(0),
            T(0), T(0), sz,   T(1),
            T(0), T(0), tz,   T(0)};
    }

    //
    // make_perspective_rh_matrix4
    //

    template < typename T, typename AngleTag >
    std::enable_if_t<std::is_floating_point_v<T>, mat4<T>>
    make_perspective_rh_matrix4(const unit<T, AngleTag>& fov, T aspect, T znear, T zfar) noexcept {
        E2D_ASSERT(!math::is_near_zero(aspect, T(0)));
        E2D_ASSERT(!math::approximately(znear, zfar, T(0)));
        E2D_ASSERT(!math::approximately(to_rad(fov), make_rad<T>(0), T(0)));
        const T sy = T(1) / math::tan(fov * T(0.5));
        const T sx = sy / aspect;
        const T sz = zfar / (znear - zfar);
        const T tz = znear * zfar / (znear - zfar);
        return  {
            sx,   T(0), T(0), T(0),
            T(0), sy,   T(0), T(0),
            T(0), T(0), sz,   T(-1),
            T(0), T(0), tz,   T(0)};
    }

    //
    // inversed
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>, std::pair<mat4<T>, bool>>
    inversed(
        const mat4<T>& m,
        T precision = math::default_precision<T>()) noexcept
    {
        const T* const mm = m.data();
        const T det = (mm[0] * mm[5] - mm[1] * mm[4]) * (mm[10] * mm[15] - mm[11] * mm[14]) -
                      (mm[0] * mm[6] - mm[2] * mm[4]) * (mm[ 9] * mm[15] - mm[11] * mm[13]) +
                      (mm[0] * mm[7] - mm[3] * mm[4]) * (mm[ 9] * mm[14] - mm[10] * mm[13]) +
                      (mm[1] * mm[6] - mm[2] * mm[5]) * (mm[ 8] * mm[15] - mm[11] * mm[12]) -
                      (mm[1] * mm[7] - mm[3] * mm[5]) * (mm[ 8] * mm[14] - mm[10] * mm[12]) +
                      (mm[2] * mm[7] - mm[3] * mm[6]) * (mm[ 8] * mm[13] - mm[ 9] * mm[12]);
        if ( math::is_near_zero(det, precision) ) {
            return std::make_pair(mat4<T>::identity(), false);
        }
        const mat4<T> inv_m(
            (mm[ 5] * (mm[10] * mm[15] - mm[11] * mm[14]) +
             mm[ 6] * (mm[11] * mm[13] - mm[ 9] * mm[15]) +
             mm[ 7] * (mm[ 9] * mm[14] - mm[10] * mm[13])),
            (mm[ 9] * (mm[ 2] * mm[15] - mm[ 3] * mm[14]) +
             mm[10] * (mm[ 3] * mm[13] - mm[ 1] * mm[15]) +
             mm[11] * (mm[ 1] * mm[14] - mm[ 2] * mm[13])),
            (mm[13] * (mm[ 2] * mm[ 7] - mm[ 3] * mm[ 6]) +
             mm[14] * (mm[ 3] * mm[ 5] - mm[ 1] * mm[ 7]) +
             mm[15] * (mm[ 1] * mm[ 6] - mm[ 2] * mm[ 5])),
            (mm[ 1] * (mm[ 7] * mm[10] - mm[ 6] * mm[11]) +
             mm[ 2] * (mm[ 5] * mm[11] - mm[ 7] * mm[ 9]) +
             mm[ 3] * (mm[ 6] * mm[ 9] - mm[ 5] * mm[10])),
            (mm[ 6] * (mm[ 8] * mm[15] - mm[11] * mm[12]) +
             mm[ 7] * (mm[10] * mm[12] - mm[ 8] * mm[14]) +
             mm[ 4] * (mm[11] * mm[14] - mm[10] * mm[15])),
            (mm[10] * (mm[ 0] * mm[15] - mm[ 3] * mm[12]) +
             mm[11] * (mm[ 2] * mm[12] - mm[ 0] * mm[14]) +
             mm[ 8] * (mm[ 3] * mm[14] - mm[ 2] * mm[15])),
            (mm[14] * (mm[ 0] * mm[ 7] - mm[ 3] * mm[ 4]) +
             mm[15] * (mm[ 2] * mm[ 4] - mm[ 0] * mm[ 6]) +
             mm[12] * (mm[ 3] * mm[ 6] - mm[ 2] * mm[ 7])),
            (mm[ 2] * (mm[ 7] * mm[ 8] - mm[ 4] * mm[11]) +
             mm[ 3] * (mm[ 4] * mm[10] - mm[ 6] * mm[ 8]) +
             mm[ 0] * (mm[ 6] * mm[11] - mm[ 7] * mm[10])),
            (mm[ 7] * (mm[ 8] * mm[13] - mm[ 9] * mm[12]) +
             mm[ 4] * (mm[ 9] * mm[15] - mm[11] * mm[13]) +
             mm[ 5] * (mm[11] * mm[12] - mm[ 8] * mm[15])),
            (mm[11] * (mm[ 0] * mm[13] - mm[ 1] * mm[12]) +
             mm[ 8] * (mm[ 1] * mm[15] - mm[ 3] * mm[13]) +
             mm[ 9] * (mm[ 3] * mm[12] - mm[ 0] * mm[15])),
            (mm[15] * (mm[ 0] * mm[ 5] - mm[ 1] * mm[ 4]) +
             mm[12] * (mm[ 1] * mm[ 7] - mm[ 3] * mm[ 5]) +
             mm[13] * (mm[ 3] * mm[ 4] - mm[ 0] * mm[ 7])),
            (mm[ 3] * (mm[ 5] * mm[ 8] - mm[ 4] * mm[ 9]) +
             mm[ 0] * (mm[ 7] * mm[ 9] - mm[ 5] * mm[11]) +
             mm[ 1] * (mm[ 4] * mm[11] - mm[ 7] * mm[ 8])),
            (mm[ 4] * (mm[10] * mm[13] - mm[ 9] * mm[14]) +
             mm[ 5] * (mm[ 8] * mm[14] - mm[10] * mm[12]) +
             mm[ 6] * (mm[ 9] * mm[12] - mm[ 8] * mm[13])),
            (mm[ 8] * (mm[ 2] * mm[13] - mm[ 1] * mm[14]) +
             mm[ 9] * (mm[ 0] * mm[14] - mm[ 2] * mm[12]) +
             mm[10] * (mm[ 1] * mm[12] - mm[ 0] * mm[13])),
            (mm[12] * (mm[ 2] * mm[ 5] - mm[ 1] * mm[ 6]) +
             mm[13] * (mm[ 0] * mm[ 6] - mm[ 2] * mm[ 4]) +
             mm[14] * (mm[ 1] * mm[ 4] - mm[ 0] * mm[ 5])),
            (mm[ 0] * (mm[ 5] * mm[10] - mm[ 6] * mm[ 9]) +
             mm[ 1] * (mm[ 6] * mm[ 8] - mm[ 4] * mm[10]) +
             mm[ 2] * (mm[ 4] * mm[ 9] - mm[ 5] * mm[ 8])));
        const T inv_det = T(1) / det;
        return std::make_pair(inv_m * inv_det, true);
    }

    //
    // transposed
    //

    template < typename T >
    mat4<T> transposed(const mat4<T>& m) noexcept {
        const T* const mm = m.data();
        return {
            mm[0], mm[4], mm[ 8], mm[12],
            mm[1], mm[5], mm[ 9], mm[13],
            mm[2], mm[6], mm[10], mm[14],
            mm[3], mm[7], mm[11], mm[15]};
    }
    
    //
    // project
    //

    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>,vec3<T>>
    project(const vec3<T>& world_space, const mat4<T>& proj, const rect<T>& viewport) noexcept {
        vec4<T> v = vec4<T>(world_space, T(1));
        v = v * proj;
        E2D_ASSERT(!math::is_near_zero(v.w, math::default_precision<T>()));
        v /= v.w;
        v.x = (v.x * T(0.5) + T(0.5)) * viewport.size.x + viewport.position.x;
        v.y = (v.y * T(0.5) + T(0.5)) * viewport.size.y + viewport.position.y;
        return vec3<T>(v);
    }

    //
    // unproject
    //
    
    template < typename T >
    std::enable_if_t<std::is_floating_point_v<T>,vec3<T>>
    unproject(const vec3<T>& window_space, const mat4<T>& inv_proj, const rect<T>& viewport) noexcept {
        vec4<T> v = vec4<T>(window_space, T(1));
        v.x = ((v.x - viewport.position.x) / viewport.size.x) * T(2) - T(1);
        v.y = ((v.y - viewport.position.y) / viewport.size.y) * T(2) - T(1);
        vec4<T> world_v = v * inv_proj;
        E2D_ASSERT(!math::is_near_zero(world_v.w, math::default_precision<T>()));
        world_v /= world_v.w;
        return vec3<T>(world_v);
    }

    //
    // approximately
    //

    template < typename T >
    bool approximately(
        const mat4<T>& l,
        const mat4<T>& r,
        T precision = math::default_precision<T>()) noexcept
    {
        return
            math::approximately(l.rows[0], r.rows[0], precision) &&
            math::approximately(l.rows[1], r.rows[1], precision) &&
            math::approximately(l.rows[2], r.rows[2], precision) &&
            math::approximately(l.rows[3], r.rows[3], precision);
    }
}

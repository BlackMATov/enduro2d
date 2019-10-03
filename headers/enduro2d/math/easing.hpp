/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"

namespace e2d::easing
{
    //
    // linear
    //

    inline f32 linear(f32 t) noexcept {
        return t;
    }

    //
    // quad
    //

    inline f32 in_quad(f32 t) noexcept {
        return t * t;
    }

    inline f32 out_quad(f32 t) noexcept {
        return 1.0f - in_quad(t - 1.0f);
    }

    inline f32 inout_quad(f32 t) noexcept {
        return t < 0.5
            ? in_quad(t * 2.0f) * 0.5f
            : out_quad(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_quad(f32 t) noexcept {
        return t < 0.5
            ? out_quad(t * 2.0f) * 0.5f
            : in_quad(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    //
    // cubic
    //

    inline f32 in_cubic(f32 t) noexcept {
        return t * t * t;
    }

    inline f32 out_cubic(f32 t) noexcept {
        return 1.0f + in_cubic(t - 1.0f);
    }

    inline f32 inout_cubic(f32 t) noexcept {
        return t < 0.5
            ? in_cubic(t * 2.0f) * 0.5f
            : out_cubic(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_cubic(f32 t) noexcept {
        return t < 0.5
            ? out_cubic(t * 2.0f) * 0.5f
            : in_cubic(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    //
    // quart
    //

    inline f32 in_quart(f32 t) noexcept {
        return t * t * t * t;
    }

    inline f32 out_quart(f32 t) noexcept {
        return 1.0f - in_quart(t - 1.0f);
    }

    inline f32 inout_quart(f32 t) noexcept {
        return t < 0.5
            ? in_quart(t * 2.0f) * 0.5f
            : out_quart(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_quart(f32 t) noexcept {
        return t < 0.5
            ? out_quart(t * 2.0f) * 0.5f
            : in_quart(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    //
    // quint
    //

    inline f32 in_quint(f32 t) noexcept {
        return t * t * t * t * t;
    }

    inline f32 out_quint(f32 t) noexcept {
        return 1.0f + in_quint(t - 1.0f);
    }

    inline f32 inout_quint(f32 t) noexcept {
        return t < 0.5
            ? in_quint(t * 2.0f) * 0.5f
            : out_quint(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_quint(f32 t) noexcept {
        return t < 0.5
            ? out_quint(t * 2.0f) * 0.5f
            : in_quint(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    //
    // expo
    //

    inline f32 in_expo(f32 t) noexcept {
        return t < 1.0f
            ? std::pow(2.0f, (t - 1.0f) * 10.0f)
            : t;
    }

    inline f32 out_expo(f32 t) noexcept {
        return t < 1.0f
            ? 1.0f - std::pow(2.0f, -t * 10.0f)
            : t;
    }

    inline f32 inout_expo(f32 t) noexcept {
        return t < 0.5
            ? in_expo(t * 2.0f) * 0.5f
            : out_expo(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_expo(f32 t) noexcept {
        return t < 0.5
            ? out_expo(t * 2.0f) * 0.5f
            : in_expo(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }
}

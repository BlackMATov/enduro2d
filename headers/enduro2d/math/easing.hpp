/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_math.hpp"
#include "trig.hpp"

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
    // sine
    //

    inline f32 in_sine(f32 t) noexcept {
        return 1.0f - math::cos(t * math::pi<f32>() * 0.5f);
    }

    inline f32 out_sine(f32 t) noexcept {
        return math::sin(t * math::pi<f32>() * 0.5f);
    }
    
    inline f32 inout_sine(f32 t) noexcept {
        return t < 0.5
            ? in_sine(t * 2.0f) * 0.5f
            : out_sine(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_sine(f32 t) noexcept {
        return t < 0.5
            ? out_sine(t * 2.0f) * 0.5f
            : in_sine(t * 2.0f - 1.0f) * 0.5f + 0.5f;
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

    //
    // circular
    //

    inline f32 in_circ(f32 t) noexcept {
        return 1.0f - math::sqrt(1.0f - t*t);
    }

    inline f32 out_circ(f32 t) noexcept {
        return 1.0f - in_circ(t - 1.0f);
    }

    inline f32 inout_circ(f32 t) noexcept {
        return t < 0.5
            ? in_circ(t * 2.0f) * 0.5f
            : out_circ(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    inline f32 outin_circ(f32 t) noexcept {
        return t < 0.5
            ? out_circ(t * 2.0f) * 0.5f
            : in_circ(t * 2.0f - 1.0f) * 0.5f + 0.5f;
    }

    //
    // back
    //

    inline f32 in_back(f32 t, f32 overshoot = 1.7f) noexcept {
        // overshoot = 1.7 -- ~10%
        // overshoot = 2.5 -- ~20%
        // overshoot = 3.3 -- ~30$
        return t * t * ((overshoot + 1.0f) * t - overshoot);
    }

    inline f32 out_back(f32 t, f32 overshoot = 1.7f) noexcept {
        t -= 1.0f;
        return 1.0f + t * t * ((overshoot + 1.0f) * t + overshoot);
    }

    inline f32 inout_back(f32 t, f32 overshoot = 1.7f) noexcept {
        return t < 0.5
            ? in_back(t * 2.0f, overshoot) * 0.5f
            : out_back(t * 2.0f - 1.0f, overshoot) * 0.5f + 0.5f;
    }

    inline f32 outin_back(f32 t, f32 overshoot = 1.7f) noexcept {
        return t < 0.5
            ? out_back(t * 2.0f, overshoot) * 0.5f
            : in_back(t * 2.0f - 1.0f, overshoot) * 0.5f + 0.5f;
    }
}

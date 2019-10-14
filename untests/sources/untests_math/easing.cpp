/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_math.hpp"
using namespace e2d;

namespace
{
    using easing_fn = f32 (*)(f32);

    bool is_accelerating(easing_fn ease, f32 from = 0.0f, f32 to = 1.0f, f32 step = 0.01f) {
        f32 prev = ease(from);
        f32 prev_a = 0.0f;
        for ( f32 t = from + step; t < to; t += step ) {
            f32 curr = ease(t);
            f32 curr_a = curr - prev;
            if ( curr_a < prev_a ) {
                return false;
            }
        }
        return true;
    }

    bool is_decelerating(easing_fn ease, f32 from = 0.0f, f32 to = 1.0f, f32 step = 0.01f) {
        f32 prev = ease(from);
        f32 prev_a = 1.0e+10f;
        for ( f32 t = from + step; t < to; t += step ) {
            f32 curr = ease(t);
            f32 curr_a = curr - prev;
            if ( curr_a > prev_a ) {
                return false;
            }
        }
        return true;
    }
}

TEST_CASE("easing") {
    {
        REQUIRE(is_accelerating(easing::linear));
        
        REQUIRE(is_accelerating(easing::in_quad));
        REQUIRE(is_decelerating(easing::out_quad));
        REQUIRE(is_accelerating(easing::inout_quad, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_quad, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_quad, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_quad, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_quad(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_quad(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_quad(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_quad(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_quad(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_quad(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_quad(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_quad(1.0f), 1.0f));

        REQUIRE(is_accelerating(easing::in_cubic));
        REQUIRE(is_decelerating(easing::out_cubic));
        REQUIRE(is_accelerating(easing::inout_cubic, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_cubic, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_cubic, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_cubic, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_cubic(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_cubic(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_cubic(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_cubic(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_cubic(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_cubic(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_cubic(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_cubic(1.0f), 1.0f));

        REQUIRE(is_accelerating(easing::in_quart));
        REQUIRE(is_decelerating(easing::out_quart));
        REQUIRE(is_accelerating(easing::inout_quart, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_quart, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_quart, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_quart, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_quart(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_quart(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_quart(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_quart(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_quart(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_quart(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_quart(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_quart(1.0f), 1.0f));

        REQUIRE(is_accelerating(easing::in_quint));
        REQUIRE(is_decelerating(easing::out_quint));
        REQUIRE(is_accelerating(easing::inout_quint, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_quint, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_quint, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_quint, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_quint(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_quint(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_quint(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_quint(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_quint(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_quint(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_quint(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_quint(1.0f), 1.0f));
        
        REQUIRE(is_accelerating(easing::in_expo));
        REQUIRE(is_decelerating(easing::out_expo));
        REQUIRE(is_accelerating(easing::inout_expo, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_expo, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_expo, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_expo, 0.5f, 1.0f));
        f32 eps = 0.001f;
        REQUIRE(math::approximately(easing::in_expo(0.0f), 0.0f, eps));
        REQUIRE(math::approximately(easing::in_expo(1.0f), 1.0f, eps));
        REQUIRE(math::approximately(easing::out_expo(0.0f), 0.0f, eps));
        REQUIRE(math::approximately(easing::out_expo(1.0f), 1.0f, eps));
        REQUIRE(math::approximately(easing::inout_expo(0.0f), 0.0f, eps));
        REQUIRE(math::approximately(easing::inout_expo(1.0f), 1.0f, eps));
        REQUIRE(math::approximately(easing::outin_expo(0.0f), 0.0f, eps));
        REQUIRE(math::approximately(easing::outin_expo(1.0f), 1.0f, eps));
        
        REQUIRE(is_accelerating(easing::in_sine));
        REQUIRE(is_decelerating(easing::out_sine));
        REQUIRE(is_accelerating(easing::inout_sine, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_sine, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_sine, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_sine, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_sine(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_sine(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_sine(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_sine(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_sine(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_sine(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_sine(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_sine(1.0f), 1.0f));
        
        REQUIRE(is_accelerating(easing::in_circ));
        REQUIRE(is_decelerating(easing::out_circ));
        REQUIRE(is_accelerating(easing::inout_circ, 0.0f, 0.5f));
        REQUIRE(is_decelerating(easing::inout_circ, 0.5f, 1.0f));
        REQUIRE(is_decelerating(easing::outin_circ, 0.0f, 0.5f));
        REQUIRE(is_accelerating(easing::outin_circ, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_circ(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_circ(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_circ(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_circ(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_circ(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_circ(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_circ(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_circ(1.0f), 1.0f));
        
        REQUIRE(is_decelerating([](f32 t) { return easing::in_back(t); }, 0.0f, 0.5f));
        REQUIRE(is_accelerating([](f32 t) { return easing::in_back(t); }, 0.5f, 1.0f));
        REQUIRE(is_accelerating([](f32 t) { return easing::out_back(t); }, 0.0f, 0.5f));
        REQUIRE(is_decelerating([](f32 t) { return easing::out_back(t); }, 0.5f, 1.0f));
        REQUIRE(math::approximately(easing::in_back(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::in_back(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::out_back(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::out_back(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::inout_back(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::inout_back(1.0f), 1.0f));
        REQUIRE(math::approximately(easing::outin_back(0.0f), 0.0f));
        REQUIRE(math::approximately(easing::outin_back(1.0f), 1.0f));
    }
}

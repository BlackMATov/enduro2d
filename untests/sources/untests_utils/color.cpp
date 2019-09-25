/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_utils.hpp"
using namespace e2d;

TEST_CASE("color") {
    {
        REQUIRE(color() == color::white());
        REQUIRE(color(color::black()) == color::black());
        REQUIRE(color(color32::yellow()) == color::yellow());
        color c = color::white();
        c = color::red();
        REQUIRE(c == color::red());
        REQUIRE((c = color(0.5f, 0.6f, 0.7f, 0.8f)) == color(0.5f, 0.6f, 0.7f, 0.8f));
        REQUIRE(math::approximately(c[0], 0.5f));
        REQUIRE(math::approximately(c[1], 0.6f));
        REQUIRE(math::approximately(c[2], 0.7f));
        REQUIRE(math::approximately(c[3], 0.8f));
        c[0] = 0.1f;
        c[1] = 0.2f;
        c[2] = 0.3f;
        c[3] = 0.4f;
        REQUIRE(math::approximately(c[0], 0.1f));
        REQUIRE(math::approximately(c[1], 0.2f));
        REQUIRE(math::approximately(c[2], 0.3f));
        REQUIRE(math::approximately(c[3], 0.4f));
    }
    {
        color c = color(0.1f,0.2f,0.3f,0.4f);
        c += 0.1f;
        REQUIRE(c == color(0.2f,0.3f,0.4f,0.5f));
        c -= -0.3f;
        REQUIRE(c == color(0.5f,0.6f,0.7f,0.8f));
        c *= 2.f;
        REQUIRE(c == color(1.0f,1.2f,1.4f,1.6f));
        c /= 2.f;
        REQUIRE(c == color(0.5f,0.6f,0.7f,0.8f));
        c += color(0.1f,0.2f,0.3f,0.4f);
        REQUIRE(c == color(0.6f,0.8f,1.0f,1.2f));
        c -= color(0.1f,0.2f,0.3f,0.4f);
        REQUIRE(c == color(0.5f,0.6f,0.7f,0.8f));
        c *= color(0.1f,0.2f,0.3f,0.4f);
        REQUIRE(c == color(0.05f,0.12f,0.21f,0.32f));
        c /= color(0.1f,0.2f,0.3f,0.4f);
        REQUIRE(c == color(0.5f,0.6f,0.7f,0.8f));
    }
    {
        REQUIRE(make_vec3(color(0.1f,0.2f,0.3f,0.4f)) == v3f(0.1f,0.2f,0.3f));
        REQUIRE(make_vec4(color(0.1f,0.2f,0.3f,0.4f)) == v4f(0.1f,0.2f,0.3f, 0.4f));
    }
    {
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) + 0.1f == color(0.2f,0.3f,0.4f,0.5f));
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) - 0.1f == color(0.0f,0.1f,0.2f,0.3f));
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) * 2.f == color(0.2f,0.4f,0.6f,0.8f));
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) / 2.f == color(0.05f,0.1f,0.15f,0.2f));

        REQUIRE(0.1f + color(0.1f,0.2f,0.3f,0.4f) == color(0.2f,0.3f,0.4f,0.5f));
        REQUIRE(0.6f - color(0.1f,0.2f,0.3f,0.4f) == color(0.5f,0.4f,0.3f,0.2f));
        REQUIRE(2.0f * color(0.1f,0.2f,0.3f,0.4f) == color(0.2f,0.4f,0.6f,0.8f));
        REQUIRE(0.1f / color(0.1f,0.2f,0.4f,0.5f) == color(1.f,0.5f,0.25f,0.2f));

        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) + color(0.2f,0.3f,0.4f,0.5f) == color(0.3f,0.5f,0.7f,0.9f));
        REQUIRE(color(0.3f,0.5f,0.7f,0.9f) - color(0.2f,0.3f,0.4f,0.5f) == color(0.1f,0.2f,0.3f,0.4f));
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) * color(0.2f,0.3f,0.4f,0.5f) == color(0.02f,0.06f,0.12f,0.2f));
        REQUIRE(color(0.1f,0.2f,0.3f,0.4f) / color(0.2f,0.1f,0.6f,0.5f) == color(0.5f,2.0f,0.5f,0.8f));
    }
    {
        REQUIRE(math::approximately(color::white(), color(1.f,1.f,1.f,1.000001f)));
        REQUIRE(math::approximately(color::white(), color(1.f,0.999999f,1.f,1.000001f)));
        REQUIRE(math::approximately(color::white(), color(0.9f,1.f,1.1f,1.f), 0.15f));

        color c0(1,2,3,4);
        REQUIRE(math::clamped(c0, color(0,0,0,0), color(0,1,2,3)) == color(0,1,2,3));
        REQUIRE(c0 == color(1,2,3,4));
        REQUIRE(math::clamped(c0, color(2,3,4,5), color(9,9,9,9)) == color(2,3,4,5));
        REQUIRE(c0 == color(1,2,3,4));

        REQUIRE(math::clamped(c0, color(3,4,5,6), color(6,7,8,9)) == color(3,4,5,6));
        REQUIRE(c0 == color(1,2,3,4));
        REQUIRE(math::clamped(c0, color(2,3,4,5), color(6,7,8,9)) == color(2,3,4,5));
        REQUIRE(c0 == color(1,2,3,4));
    }
    {
        REQUIRE(math::approximately(math::minimum(color(1,2,3,4)), 1.f));
        REQUIRE(math::approximately(math::minimum(color(2,1,3,4)), 1.f));
        REQUIRE(math::approximately(math::minimum(color(4,3,2,1)), 1.f));

        REQUIRE(math::approximately(math::maximum(color(1,2,3,4)), 4.f));
        REQUIRE(math::approximately(math::maximum(color(2,1,3,4)), 4.f));
        REQUIRE(math::approximately(math::maximum(color(4,3,2,1)), 4.f));
    }
    {
        color c0(1,2,3,4);
        REQUIRE(math::minimized(c0, color(0,1,2,3)) == color(0,1,2,3));
        REQUIRE(c0 == color(1,2,3,4));
        REQUIRE(math::maximized(c0, color(2,3,4,5)) == color(2,3,4,5));
        REQUIRE(c0 == color(1,2,3,4));

        REQUIRE(math::maximized(c0, color(3,4,5,6)) == color(3,4,5,6));
        REQUIRE(c0 == color(1,2,3,4));
        REQUIRE(math::minimized(c0, color(2,3,4,5)) == color(1,2,3,4));
        REQUIRE(c0 == color(1,2,3,4));
    }
    {
        color v0(3,4,5,6);
        REQUIRE(math::clamped(v0, color(0,0,0,0), color(2,3,4,5)) == color(2,3,4,5));
        REQUIRE(v0 == color(3,4,5,6));
        REQUIRE(math::clamped(v0, color(3,4,5,6), color(3,4,5,6)) == color(3,4,5,6));
        REQUIRE(v0 == color(3,4,5,6));

        REQUIRE(math::clamped(v0, color(0,0,0,0), color(0,0,0,0)) == color(0,0,0,0));
        REQUIRE(math::clamped(v0, color(0,0,0,0), color(3,2,1,0)) == color(3,2,1,0));
        REQUIRE(math::clamped(v0, color(0,0,0,0), color(4,3,2,1)) == color(3,3,2,1));

        REQUIRE(math::clamped(v0, color(4,5,6,7), color(9,9,9,9)) == color(4,5,6,7));
        REQUIRE(math::clamped(v0, color(6,5,4,3), color(9,9,9,9)) == color(6,5,5,6));
        REQUIRE(math::clamped(v0, color(7,6,5,4), color(9,9,9,9)) == color(7,6,5,6));
    }
    {
        {
            REQUIRE_FALSE(math::contains_nan(color(0,1,2,3)));
            REQUIRE_FALSE(math::contains_nan(color(0.f,1.f,2.f,3.f)));
            REQUIRE(math::contains_nan(color(0.f,1.f,2.f,std::numeric_limits<f32>::quiet_NaN())));
            REQUIRE(math::contains_nan(color(0.f,1.f,std::numeric_limits<f32>::quiet_NaN(),2.f)));
            REQUIRE(math::contains_nan(color(std::numeric_limits<f32>::infinity(),1.f,2.f,3.f)));
            REQUIRE(math::contains_nan(color(1.f,std::numeric_limits<f32>::infinity(),2.f,3.f)));
        }
    }
    {
        REQUIRE(colors::pack_color(color(color32(1,2,3,4))) == 0x04010203);
        REQUIRE(colors::pack_color(color(color32(0x12,0x34,0x56,0x78))) == 0x78123456);
        REQUIRE(colors::unpack_color(0x04010203) == color(color32(1,2,3,4)));
        REQUIRE(colors::unpack_color(0x78123456) == color(color32(0x12,0x34,0x56,0x78)));

        REQUIRE(colors::pack_color(color(0.001f,0.001f,0.001f,0.001f)) == 0x00000000);
        REQUIRE(colors::pack_color(color(0.999f,0.999f,0.999f,0.999f)) == 0xFFFFFFFF);
    }
}

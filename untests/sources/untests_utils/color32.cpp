/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_utils.hpp"
using namespace e2d;

TEST_CASE("color32") {
    {
        REQUIRE(color32() == color32::white());
        REQUIRE(color32(color32::black()) == color32::black());
        REQUIRE(color32(color::yellow()) == color32::yellow());
        REQUIRE(color32(color(0.9999f,0.9999f,0.9999f,0.9999f)) == color32::white());
        REQUIRE(color32(color(0.0001f,0.0001f,0.0001f,0.0001f)) == color32::clear());
        color32 c = color32::white();
        c = color32::red();
        REQUIRE(c == color32::red());
        REQUIRE((c = color32(10,20,30,40)) == color32(10,20,30,40));
        REQUIRE(c[0] == 10);
        REQUIRE(c[1] == 20);
        REQUIRE(c[2] == 30);
        REQUIRE(c[3] == 40);
        c[0] = u8(20);
        c[1] = u8(30);
        c[2] = u8(40);
        c[3] = u8(50);
        REQUIRE(c[0] == 20);
        REQUIRE(c[1] == 30);
        REQUIRE(c[2] == 40);
        REQUIRE(c[3] == 50);
    }
    {
        color32 c = color32(10,20,30,40);
        c += 40;
        REQUIRE(c == color32(50,60,70,80));
        c -= 10;
        REQUIRE(c == color32(40,50,60,70));
        c *= 2;
        REQUIRE(c == color32(80,100,120,140));
        c /= 2;
        REQUIRE(c == color32(40,50,60,70));
        c += color32(10,20,30,40);
        REQUIRE(c == color32(50,70,90,110));
        c -= color32(10,20,30,40);
        REQUIRE(c == color32(40,50,60,70));
        c *= color32(2,3,4,2);
        REQUIRE(c == color32(80,150,240,140));
        c /= color32(2,3,6,7);
        REQUIRE(c == color32(40,50,40,20));
    }
    {
        REQUIRE(make_vec3(color32(10,20,30,40)) == vec3<u8>(10,20,30));
        REQUIRE(make_vec4(color32(10,20,30,40)) == vec4<u8>(10,20,30,40));
    }
    {
        REQUIRE(color32(10,20,30,40) + 10 == color32(20,30,40,50));
        REQUIRE(color32(10,20,30,40) - 10 == color32(0,10,20,30));
        REQUIRE(color32(10,20,30,40) * 2  == color32(20,40,60,80));
        REQUIRE(color32(10,20,30,40) / 2  == color32(5,10,15,20));

        REQUIRE(10  + color32(10,20,30,40) == color32(20,30,40,50));
        REQUIRE(60  - color32(10,20,30,40) == color32(50,40,30,20));
        REQUIRE(2   * color32(10,20,30,40) == color32(20,40,60,80));
        REQUIRE(100 / color32(10,20,25,50) == color32(10,5,4,2));

        REQUIRE(color32(10,20,30,40) + color32(20,30,40,50) == color32(30,50,70,90));
        REQUIRE(color32(10,20,30,40) - color32(1,2,3,4) == color32(9,18,27,36));
        REQUIRE(color32(10,20,30,40) * color32(3,4,2,5) == color32(30,80,60,200));
        REQUIRE(color32(80,150,180,140) / color32(2,3,6,7) == color32(40,50,30,20));
    }
    {
        REQUIRE(math::approximately(color32::white(), color32(255,255,255,255)));
        REQUIRE(math::approximately(color32::white(), color32(255,255,255,255)));
        REQUIRE(math::approximately(color32::white(), color32(253,255,255,254), 3));

        color32 c0(1,2,3,4);
        REQUIRE(math::clamped(c0, color32(0,0,0,0), color32(0,1,2,3)) == color32(0,1,2,3));
        REQUIRE(c0 == color32(1,2,3,4));
        REQUIRE(math::clamped(c0, color32(2,3,4,5), color32(9,9,9,9)) == color32(2,3,4,5));
        REQUIRE(c0 == color32(1,2,3,4));

        REQUIRE(math::clamped(c0, color32(3,4,5,6), color32(6,7,8,9)) == color32(3,4,5,6));
        REQUIRE(c0 == color32(1,2,3,4));
        REQUIRE(math::clamped(c0, color32(2,3,4,5), color32(6,7,8,9)) == color32(2,3,4,5));
        REQUIRE(c0 == color32(1,2,3,4));
    }
    {
        REQUIRE(math::minimum(color32(1,2,3,4)) == 1);
        REQUIRE(math::minimum(color32(2,1,3,4)) == 1);
        REQUIRE(math::minimum(color32(4,3,2,1)) == 1);

        REQUIRE(math::maximum(color32(1,2,3,4)) == 4);
        REQUIRE(math::maximum(color32(2,1,3,4)) == 4);
        REQUIRE(math::maximum(color32(4,3,2,1)) == 4);
    }
    {
        color32 v0(3,4,5,6);
        REQUIRE(math::clamped(v0, color32(0,0,0,0), color32(2,3,4,5)) == color32(2,3,4,5));
        REQUIRE(v0 == color32(3,4,5,6));
        REQUIRE(math::clamped(v0, color32(3,4,5,6), color32(3,4,5,6)) == color32(3,4,5,6));
        REQUIRE(v0 == color32(3,4,5,6));

        REQUIRE(math::clamped(v0, color32(0,0,0,0), color32(0,0,0,0)) == color32(0,0,0,0));
        REQUIRE(math::clamped(v0, color32(0,0,0,0), color32(3,2,1,0)) == color32(3,2,1,0));
        REQUIRE(math::clamped(v0, color32(0,0,0,0), color32(4,3,2,1)) == color32(3,3,2,1));

        REQUIRE(math::clamped(v0, color32(4,5,6,7), color32(9,9,9,9)) == color32(4,5,6,7));
        REQUIRE(math::clamped(v0, color32(6,5,4,3), color32(9,9,9,9)) == color32(6,5,5,6));
        REQUIRE(math::clamped(v0, color32(7,6,5,4), color32(9,9,9,9)) == color32(7,6,5,6));
    }
    {
        color32 c0(1,2,3,4);
        REQUIRE(math::minimized(c0, color32(0,1,2,3)) == color32(0,1,2,3));
        REQUIRE(c0 == color32(1,2,3,4));
        REQUIRE(math::maximized(c0, color32(2,3,4,5)) == color32(2,3,4,5));
        REQUIRE(c0 == color32(1,2,3,4));

        REQUIRE(math::maximized(c0, color32(3,4,5,6)) == color32(3,4,5,6));
        REQUIRE(c0 == color32(1,2,3,4));
        REQUIRE(math::minimized(c0, color32(2,3,4,5)) == color32(1,2,3,4));
        REQUIRE(c0 == color32(1,2,3,4));
    }
    {
        REQUIRE(colors::pack_color32(color32(1,2,3,4)) == 0x04010203);
        REQUIRE(colors::pack_color32(color32(0x12,0x34,0x56,0x78)) == 0x78123456);
        REQUIRE(colors::unpack_color32(0x04010203) == color32(1,2,3,4));
        REQUIRE(colors::unpack_color32(0x78123456) == color32(0x12,0x34,0x56,0x78));

        REQUIRE(colors::pack_color32(color32(0x01,0x01,0x01,0x01)) == 0x01010101);
        REQUIRE(colors::pack_color32(color32(0xFE,0xFE,0xFE,0xFE)) == 0xFEFEFEFE);
    }
}

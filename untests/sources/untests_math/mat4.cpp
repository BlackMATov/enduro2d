/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_math.hpp"
using namespace e2d;

TEST_CASE("mat4") {
    {
        {
            auto m = m4i::identity();
            REQUIRE(m.rows[0][0] == 1);
            REQUIRE(m[2][2] == 1);
            REQUIRE((m[0][0] == m[1][1] && m[1][1] == m[2][2] && m[2][2] == m[3][3]));
            REQUIRE((m.data()[0] == m.data()[5] && m.data()[5] == m.data()[10]));
            REQUIRE((m.data()[10] == m.data()[15] && m.data()[15] == 1));
            REQUIRE((m[0][1] == 0 && m[0][2] == 0 && m[0][3] == 0));
            REQUIRE((m[1][0] == 0 && m[1][2] == 0 && m[1][3] == 0));
            REQUIRE((m.data()[1] == 0 && m.data()[2] == 0 && m.data()[3] == 0 && m.data()[4] == 0));
            REQUIRE((m.data()[6] == 0 && m.data()[7] == 0 && m.data()[8] == 0 && m.data()[9] == 0));
            m[0] = v4i::unit(); m[1] = v4i::unit() * 2;
            REQUIRE((m[0] == v4i::unit() && m[1] == v4i::unit() * 2));
            m[1][2] = 3; m[2][1] = 4;
            REQUIRE((m.rows[1][2] == 3 && m.rows[2][1] == 4));
            REQUIRE((m.data()[1*4+2] == 3 && m.data()[2*4+1] == 4));
        }
        REQUIRE(math::make_scale_matrix4(1,1,1) == math::make_scale_matrix4(v3i::unit()));
        REQUIRE(math::make_scale_matrix4(v2i(1,1),1) == math::make_scale_matrix4(v3i::unit()));

        REQUIRE(math::make_translation_matrix4(1,1,1) == math::make_translation_matrix4(v3i::unit()));
        REQUIRE(math::make_translation_matrix4(v2i(1,1),1) == math::make_translation_matrix4(v3i::unit()));

        REQUIRE(math::make_rotation_matrix4(make_rad(1.f),1.f,1.f,1.f) == math::make_rotation_matrix4(make_rad(1.f), v3f::unit()));
        REQUIRE(math::make_rotation_matrix4(make_rad(1.f),v2f(1.f,1.f),1.f) == math::make_rotation_matrix4(make_rad(1.f), v3f::unit()));

        REQUIRE(math::make_orthogonal_lh_matrix4(640.f, 480.f, 0.f, 1.f) == math::make_orthogonal_lh_matrix4(v2f(640,480), 0.f, 1.f));
        REQUIRE(math::make_orthogonal_lh_matrix4(640.0, 480.0, 0.0, 1.0) == math::make_orthogonal_lh_matrix4(v2d(640,480), 0.0, 1.0));
    }
    {
        REQUIRE(m4f(m4f::identity()) == m4f::identity());
        m4f m0(m4f::identity());
        m0 = m4f::zero();
        REQUIRE(m0 == m4f::zero());

        REQUIRE(-m4f::identity() == math::make_scale_matrix4(1.f, 1.f, 1.f) * -1.f);

        REQUIRE(math::make_scale_matrix4(1.1f, 2.2f, 3.3f).cast_to<i32>() ==
                math::make_scale_matrix4(1, 2, 3));
    }
    {
        REQUIRE(m4f() == m4f(1, 0, 0, 0,
                             0, 1, 0, 0,
                             0, 0, 1, 0,
                             0, 0, 0, 1));

        REQUIRE(math::make_scale_matrix4(2.f,3.f,4.f) + 2.f == m4f(4, 2, 2, 2,
                                                                   2, 5, 2, 2,
                                                                   2, 2, 6, 2,
                                                                   2, 2, 2, 3));

        REQUIRE(math::make_scale_matrix4(2, 3, 4) * 2 == m4i(4, 0, 0, 0,
                                                             0, 6, 0, 0,
                                                             0, 0, 8, 0,
                                                             0, 0, 0, 2));

        m4f m0 = math::make_scale_matrix4(2.f, 3.f, 4.f);
        m0 += 2.f;
        REQUIRE(m0 == math::make_scale_matrix4(2.f, 3.f, 4.f) + 2.f);

        m4f m2 = math::make_scale_matrix4(2.f, 3.f, 4.f);
        m2 *= 2.f;
        REQUIRE(m2 == math::make_scale_matrix4(2.f, 3.f, 4.f) * 2.f);
    }
    {
        REQUIRE(math::make_scale_matrix4(2, 3, 4) +
                math::make_scale_matrix4(2, 3, 4) ==
                math::make_scale_matrix4(2, 3, 4) * 2);

        REQUIRE(math::make_scale_matrix4(2, 3, 4) *
                math::make_scale_matrix4(2, 3, 4) ==
                math::make_scale_matrix4(4, 9, 16));

        m4f m0 = math::make_scale_matrix4(2.f, 3.f, 4.f);
        m0 += math::make_scale_matrix4(2.f, 3.f, 4.f);
        REQUIRE(m0 == math::make_scale_matrix4(2, 3, 4).cast_to<f32>() * 2.f);

        m4f m2 = math::make_scale_matrix4(2.f, 3.f, 4.f);
        m2 *= math::make_scale_matrix4(2.f, 3.f, 4.f);
        REQUIRE(m2 == math::make_scale_matrix4(4, 9, 16).cast_to<f32>());
    }
    {
        REQUIRE(v4f(2,3,4,5) * math::make_scale_matrix4(3.f,4.f,5.f) == v4f(6.f,12.f,20.f,5.f));
        REQUIRE(v4f(2,3,4,1) * math::make_translation_matrix4(3.f,4.f,5.f) == v4f(2+3,3+4,4+5,1));
        REQUIRE(v4d(2,3,4,2) * math::make_translation_matrix4(3.0,4.0,5.0) == v4d(2+3*2,3+4*2,4+5*2,2));
        REQUIRE(math::approximately(
            v4f(20,30,10,1) * math::make_rotation_matrix4(math::half_pi<f32>(), v3f::unit_z()),
            v4f(-30.f,20.f,10,1)));
    }
    {
        auto m = math::make_scale_matrix4(2.f,3.f,4.f);
        m[3][3] = 0.5f;
        REQUIRE(v4f(2,3,4,0) * m == v4f(4,9,16,0));
        REQUIRE(v4f(2,3,4,1) * m == v4f(4,9,16,0.5));
    }
    {
        REQUIRE(math::transposed(m4f::identity()) == m4f::identity());
        REQUIRE(math::transposed(m4i( 1, 2, 3, 4,
                                      5, 6, 7, 8,
                                      9,10,11,12,
                                     13,14,15,16)) == m4i(1,5, 9,13,
                                                          2,6,10,14,
                                                          3,7,11,15,
                                                          4,8,12,16));
    }
    {
        REQUIRE(
            math::inversed(math::make_scale_matrix4(2.f,4.f)).first ==
            math::make_scale_matrix4(0.5f, 0.25f));
        REQUIRE(
            math::inversed(math::make_translation_matrix4(10.f, 20.f)).first ==
            math::make_translation_matrix4(-10.f, -20.f));

        auto m0 = math::make_scale_matrix4(2.f,4.f,5.f) *
                  math::make_rotation_matrix4(make_rad(1.f), v3f::unit_z()) *
                  math::make_translation_matrix4(30.f, 40.f);
        REQUIRE(math::inversed(m0).first * m0 == m4f::identity());
        REQUIRE(m0 * math::inversed(m0).first == m4f::identity());

        REQUIRE(math::inversed(m4f::zero()).second == false);
        REQUIRE(math::inversed(m4f::identity()).second == true);
    }
    {
        REQUIRE(math::make_trs_matrix4(math::make_translation_trs2(v2f(1.f,2.f)))
            == math::make_translation_matrix4(1.f,2.f));
        REQUIRE(math::make_trs_matrix4(math::make_translation_trs3(v3f(1.f,2.f,3.f)))
            == math::make_translation_matrix4(1.f,2.f,3.f));

        REQUIRE(math::make_trs_matrix4(math::make_rotation_trs2(make_rad(3.f)))
            == math::make_rotation_matrix4(make_rad(3.f), v4f::unit_z()));
        REQUIRE(math::make_trs_matrix4(math::make_rotation_trs3(q4f(1,2,3,4)))
            == math::make_rotation_matrix4(q4f(1,2,3,4)));

        REQUIRE(math::make_trs_matrix4(math::make_scale_trs2(v2f(1.f,2.f)))
            == math::make_scale_matrix4(1.f,2.f));
        REQUIRE(math::make_trs_matrix4(math::make_scale_trs3(v3f(1.f,2.f,3.f)))
            == math::make_scale_matrix4(1.f,2.f,3.f));

        auto m0 = math::make_scale_matrix4(2.f,4.f,5.f) *
                  math::make_rotation_matrix4(make_rad(1.f), v3f::unit_z()) *
                  math::make_translation_matrix4(30.f, 40.f, 50.f);
        REQUIRE(m0 == math::make_trs_matrix4(make_trs3(
            v3f(30.f, 40.f, 50.f),
            math::make_quat_from_axis_angle(make_rad(1.f), v3f::unit_z()),
            v3f(2.f,4.f,5.f))));
    }
    {
        -m4i::identity();
        m4i::identity() + 2;
        m4i::identity() * 2;
        m4i::identity() + m4i::identity();
        m4i::identity() * m4i::identity();
        v4i::unit() * m4i::identity();
        REQUIRE(m4i::identity() == m4i::identity());
        REQUIRE_FALSE(m4i::identity() != m4i::identity());
    }
    {
        // z-axis: positive form zero to one
        b2f viewport(100.0f, 150.0f, 600.0f, 400.0f);
        m4f proj = math::make_perspective_lh_matrix4(deg<f32>(60.0f), viewport.size.x / viewport.size.y, 1.0f, 100.0f);
        m4f view = m4f::identity();
        m4f vp = view * proj;
        auto vp_inv_opt = math::inversed(vp);
        m4f vp_inv = vp_inv_opt.first;
        REQUIRE(vp_inv_opt.second);

        v3f world_p1 = v3f(0.0f, 0.0f, 10.0f);
        v3f wnd_p1 = math::project(world_p1, vp, viewport);
        v3f world_u1 = math::unproject(wnd_p1, vp_inv, viewport);
        REQUIRE(math::approximately(world_p1, world_u1, 0.01f));
        
        v3f world_p2 = v3f(-10.0f, 40.0f, 90.0f);
        v3f wnd_p2 = math::project(world_p2, vp, viewport);
        v3f world_u2 = math::unproject(wnd_p2, vp_inv, viewport);
        REQUIRE(math::approximately(world_p2, world_u2, 0.01f));
    }
}

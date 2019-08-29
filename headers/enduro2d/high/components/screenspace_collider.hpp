/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "shape2d.hpp"

#include "../factory.hpp"

namespace e2d
{
    template < u32 N >
    class convex_hull_screenspace_collider_base {
    public:
        struct plane2d {
            v2f norm;
            f32 dist;
        };
        std::array<plane2d, N> planes;
    };

    class rectangle_screenspace_collider final :
        public convex_hull_screenspace_collider_base<4>
    {};
    
    class circle_screenspace_collider final :
        public convex_hull_screenspace_collider_base<circle_shape::detail_level>
    {};

    class polygon_screenspace_collider final {
    public:
        struct plane2d {
            v2f norm;
            f32 dist;
        };
        struct triangle {
            std::array<plane2d, 3> planes;
        };
        std::vector<triangle> triangles;
    };
}

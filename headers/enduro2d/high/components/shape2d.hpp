/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class rectangle_shape final {
    public:
        rectangle_shape(const b2f& r);

        const b2f& rectangle() const noexcept;
    private:
        b2f rect_;
    };

    class circle_shape final {
    public:
        circle_shape(const v2f& c, f32 r);
        
        v2f center() const noexcept;
        f32 radius() const noexcept;

        static constexpr u32 detail_level = 16; // segments per circle
    private:
        v2f center_;
        f32 radius_ = 0.0f;
    };

    class polygon_shape final {
    public:
        struct triangle {
            v3f p0, p1, p2;
        };
        std::vector<triangle> triangles;
    };
}

namespace e2d
{
    inline rectangle_shape::rectangle_shape(const b2f& r)
    : rect_(r) {}

    inline const b2f& rectangle_shape::rectangle() const noexcept {
        return rect_;
    }

    inline circle_shape::circle_shape(const v2f& c, f32 r)
    : center_(c)
    , radius_(r) {}
        
    inline v2f circle_shape::center() const noexcept {
        return center_;
    }

    inline f32 circle_shape::radius() const noexcept {
        return radius_;
    }
}

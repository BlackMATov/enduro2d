/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class circle_shape final {
    public:
        circle_shape(const v2f& c, f32 r) : center_(c), radius_(r) {}
        
        v2f center() const noexcept { return center_; }
        f32 radius() const noexcept { return radius_; }

        static constexpr u32 detail_level = 16; // segments per circle
    private:
        v2f center_;
        f32 radius_ = 0.0f;
    };
}

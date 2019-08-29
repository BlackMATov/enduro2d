/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"

namespace e2d
{
    class rectangle_shape final {
    public:
        rectangle_shape(const b2f& r);

        const b2f& rectangle() const noexcept;
    private:
        b2f rect_;
    };
}

namespace e2d
{
    inline rectangle_shape::rectangle_shape(const b2f& r)
    : rect_(r) {}

    inline const b2f& rectangle_shape::rectangle() const noexcept {
        return rect_;
    }
}

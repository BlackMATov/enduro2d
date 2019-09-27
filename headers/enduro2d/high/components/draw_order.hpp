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
    class draw_order final {
    public:
        draw_order() = default;
        draw_order(u32 index);

        draw_order& set_index(u32 value) noexcept;
        u32 index() const noexcept;
    private:
        u32 index_ = 0;
    };
}

namespace e2d
{
    inline draw_order::draw_order(u32 index)
    : index_(index) {}

    inline draw_order& draw_order::set_index(u32 value) noexcept {
        index_ = value;
        return *this;
    }

    inline u32 draw_order::index() const noexcept {
        return index_;
    }
}

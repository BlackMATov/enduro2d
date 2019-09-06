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
    class scissor_rect final {
    public:
        scissor_rect() = default;
        scissor_rect(const b2u& r);

        scissor_rect& rect(const b2u& value) noexcept;
        const b2u& rect() const noexcept;
    private:
        b2u rect_;
    };
}

namespace e2d
{
    inline scissor_rect::scissor_rect(const b2u& r)
    : rect_(r) {}

    inline scissor_rect& scissor_rect::rect(const b2u& value) noexcept {
        rect_ = value;
        return *this;
    }

    inline const b2u& scissor_rect::rect() const noexcept {
        return rect_;
    }
}

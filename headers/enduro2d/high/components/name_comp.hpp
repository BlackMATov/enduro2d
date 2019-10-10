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
    class name_comp final {
    public:
        name_comp(str_hash h)
        : name_(h) {}

        const str_hash& name() const noexcept {
            return name_;
        }
    private:
        friend class name_comp_system;

        name_comp() = default;

        str_hash name_;
    };
}

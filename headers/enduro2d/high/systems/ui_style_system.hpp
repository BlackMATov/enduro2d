/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class ui_style_system final : public ecs::system {
    public:
        ui_style_system();
        ~ui_style_system() noexcept;
        void before_update(ecs::registry& owner);
        void process(ecs::registry& owner);
    };
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class ui_layout_system final : public ecs::system {
    public:
        ui_layout_system();
        ~ui_layout_system() noexcept;
        void process(ecs::registry& owner);
    };
}
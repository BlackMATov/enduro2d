/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class convex_hull_screenspace_raycast_system final : public ecs::system {
    public:
        convex_hull_screenspace_raycast_system();
        ~convex_hull_screenspace_raycast_system() noexcept final;
        void process(ecs::registry& owner) override;
    };
}

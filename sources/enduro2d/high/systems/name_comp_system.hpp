/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include <enduro2d/high/single_components/name_map_comp.hpp>

namespace e2d
{
    class name_comp_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };
}

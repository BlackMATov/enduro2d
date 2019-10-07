/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/spine_systems.hpp>
#include <enduro2d/high/world_ev.hpp>

namespace e2d
{
    void spine_system::add_systems(ecs::registry& owner) const {
        owner.assign_system<spine_pre_system, world_ev::pre_update>();
        owner.assign_system<spine_post_system, world_ev::post_update>();
    }

    void spine_system::process(ecs::registry&) {
    }
}

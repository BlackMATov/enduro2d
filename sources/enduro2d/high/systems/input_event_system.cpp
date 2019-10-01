/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/input_event_system.hpp>
#include <enduro2d/high/systems/shape_projection_system.hpp>

namespace e2d
{
    void input_event_system::add_systems(ecs::registry& owner) const {
        owner.assign_system<input_event_system_pre_update, pre_update_evt>();
        owner.assign_system<input_event_system_raycast, raycast_evt>();
        owner.assign_system<input_event_system_post_update, post_update_evt>();
        owner.assign_system<shape_projection_system, ecs::before_event<raycast_evt>>(); // TODO: move somewhere
    }

    void input_event_system::process(ecs::registry& owner, ecs::event_ref) {
        owner.enque_event(pre_update_evt());
    }
}

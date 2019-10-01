/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>

namespace e2d
{
    void ui_system::add_systems(ecs::registry& owner) const {
        owner.assign_system<ui_style_system, ecs::after_event<update_controllers_evt>>();
        owner.assign_system<ui_layout_system, update_layouts_evt>();
        owner.assign_system<ui_controller_system, update_controllers_evt>();
    }

    void ui_system::process(ecs::registry& owner, ecs::event_ref) {
        owner.enque_event(update_controllers_evt());
        owner.enque_event(update_layouts_evt());
    }
}

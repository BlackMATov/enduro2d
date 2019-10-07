/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>
#include <enduro2d/high/single_components/frame_params_comp.hpp>

namespace e2d
{
    void ui_system::add_systems(ecs::registry& owner) const {
        owner.assign_system<ui_style_system, ecs::after_event<update_controllers_evt>>();
        owner.assign_system<ui_layout_system, update_layouts_evt>();
        owner.assign_system<ui_controller_system, update_controllers_evt>();
        owner.assign_system<ui_controller_event_system, ecs::before_event<update_controllers_evt>>();
        owner.assign_system<ui_animation_system, update_animation_evt>();
    }

    void ui_system::process(ecs::registry& owner) {
        auto& params = owner.ensure_single_component<frame_params_comp>();
        params.delta_time = the<engine>().delta_time(); // TODO
        params.realtime_time = the<engine>().realtime_time();

        owner.enque_event<update_controllers_evt>();
        owner.enque_event<update_layouts_evt>();
        owner.enque_event<update_animation_evt>();
    }
}

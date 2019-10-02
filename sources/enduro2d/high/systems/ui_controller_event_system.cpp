/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>
#include <enduro2d/high/components/ui_controller.hpp>

namespace e2d
{
    void ui_controller_event_system::process(ecs::registry& owner, ecs::event_ref) {
        owner.remove_all_components<ui_controller_events>();
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "name_comp_system.hpp"
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/single_components/name_map_comp.hpp>
#include <enduro2d/high/components/name_comp_impl.hpp>

namespace e2d
{
    void name_comp_system::process(ecs::registry& owner) {
        auto& name_map = owner.ensure_single_component<name_map_comp>();

        owner.for_joined_components<name_comp_impl, actor>(
        [&name_map](ecs::entity e, const name_comp_impl& n, actor& act) {
            str_hash h = make_hash(n.name());
            name_map.insert(h, act.node()->owner());
            e.assign_component<name_comp>(h);
        });

        owner.remove_all_components<name_comp_impl>();
    }
}

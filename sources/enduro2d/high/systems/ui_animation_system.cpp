/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>
#include <enduro2d/high/components/ui_animation.hpp>

namespace e2d
{
    void ui_animation_system::process(ecs::registry& owner) {
        const secf t = the<engine>().time();
        vector<ecs::entity_id> to_remove;

        owner.for_joined_components<ui_animation>(
        [t, &to_remove](ecs::entity e, ui_animation& anim) noexcept {
            try {
                if ( !anim.animation() ||
                     !anim.animation()->update(t, e) )
                {
                    to_remove.push_back(e.id());
                }
            } catch(...) {
                to_remove.push_back(e.id());
            }
        });

        for ( auto& id : to_remove ) {
            ecs::entity(owner, id).remove_component<ui_animation>();
        }
    }
}

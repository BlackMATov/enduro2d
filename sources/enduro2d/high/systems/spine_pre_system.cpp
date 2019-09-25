/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/spine_systems.hpp>

#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/spine_player_cmd.hpp>
#include <enduro2d/high/components/spine_player_evt.hpp>

#include <spine/spine.h>

namespace e2d
{
    //
    // internal_state
    //

    class spine_pre_system::internal_state final {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner) {
            update_animations(owner);
        }
    private:
        void update_animations(ecs::registry& owner) {
            const float dt = the<engine>().delta_time();
            owner.for_each_component<spine_player>([dt](
                const ecs::const_entity&,
                spine_player& p)
            {
                spSkeleton* skeleton = p.skeleton().get();
                spAnimationState* anim_state = p.animation().get();

                if ( !skeleton || !anim_state ) {
                    return;
                }

                spSkeleton_update(skeleton, dt);
                spAnimationState_update(anim_state, dt);
                spAnimationState_apply(anim_state, skeleton);
                spSkeleton_updateWorldTransform(skeleton);
            });
        }
    };

    //
    // spine_pre_system
    //

    spine_pre_system::spine_pre_system()
    : state_(new internal_state()) {}
    spine_pre_system::~spine_pre_system() noexcept = default;

    void spine_pre_system::process(ecs::registry& owner) {
        state_->process(owner);
    }
}

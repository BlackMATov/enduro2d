/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/render_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/draw_order.hpp>
#include <enduro2d/high/components/renderer.hpp>

namespace e2d
{
    //
    // render_system::internal_state
    //

    class render_system::internal_state final : private noncopyable {
    public:
        internal_state() {}
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner);
    private:
        void update_draw_order_(ecs::registry& owner);
    };
    
    void render_system::internal_state::process(ecs::registry& owner) {
        owner.for_each_component<camera>(
        [this, &owner](const ecs::const_entity& cam_e, const camera& cam){
            const actor* const cam_a = cam_e.find_component<actor>();
            const const_node_iptr cam_n = cam_a ? cam_a->node() : nullptr;
            
            update_draw_order_(owner);

            auto rq = the<render>().create_render_queue();

            owner.set_event(render_system::render_with_camera_evt{rq});
        });
    }
    
    void render_system::internal_state::update_draw_order_(ecs::registry& owner) {
        owner.remove_all_components<draw_order>();
        owner.for_joined_components<renderer, actor>(
        [](ecs::entity e, const renderer&, const actor& act) {
            e.assign_component<draw_order>(act.node()->render_order());
        });
    }

    //
    // render_system
    //

    render_system::render_system()
    : state_(new internal_state()) {}

    render_system::~render_system() noexcept = default;
    
    void render_system::add_systems(ecs::registry& owner) const {
        owner.assign_system<spine_render_system, render_with_camera_evt>();
        owner.assign_system<sprite_render_system, render_with_camera_evt>();
        //owner.assign_system<sprite_9p_render_system, render_with_camera_evt>();
        owner.assign_system<model_render_system, render_with_camera_evt>();
        //owner.assign_system<label_render_system, render_with_camera_evt>();
    }

    void render_system::process(ecs::registry& owner, ecs::event_ref) {
        //  - build camera graph
        //  - for each camera:
        //      - create render queue
        //      - get render technique component and add passes to render queue
        //      - frustum culling
        //      - sort nodes
        //          - write to draw_order component
        //      - draw all
        //          - set_event(render_with_camera(...));

        state_->process(owner);
    }
}

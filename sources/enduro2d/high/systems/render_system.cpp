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
#include <enduro2d/high/components/render_technique.hpp>

#include "render_system_impl.hpp"

namespace e2d
{
    //
    // render_system::internal_state
    //

    class render_system::internal_state final : private noncopyable {
    public:
        internal_state();
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner);
    private:
        void update_draw_order_(ecs::registry& owner);
    private:
        render& render_;
        render_queue::memory_manager mem_mngr_;
        render_queue::resource_cache res_cache_;
    };
    
    render_system::internal_state::internal_state()
    : render_(the<render>())
    , mem_mngr_()
    , res_cache_(render_)
    {}

    void render_system::internal_state::process(ecs::registry& owner) {
        // prepare
        mem_mngr_.discard();

        // draw
        owner.for_each_component<camera>(
        [this, &owner](const ecs::const_entity& cam_e, const camera& cam){
            const auto* const cam_rt = cam_e.find_component<render_technique>();
            const auto* const cam_a = cam_e.find_component<actor>();
            const auto cam_n = cam_a ? cam_a->node() : nullptr;
            
            update_draw_order_(owner);

            auto rq = std::make_shared<render_queue>(the<render>(), mem_mngr_, res_cache_);
            
            render::renderpass_desc rp;
            const_buffer_ptr cbuf;
            
            if ( cam_rt && cam_rt->passes().size() ) {
                auto& pass = cam_rt->passes().front();
                rp = pass.desc;

                if ( rp.color_load_op() == render::attachment_load_op::clear ) {
                    rp.color_clear(cam.background());
                }
            
                if ( pass.templ ) {
                    render::property_map props = pass.properties;

                    const m4f& cam_w = cam_n
                        ? cam_n->world_matrix()
                        : m4f::identity();
                    const std::pair<m4f,bool> cam_w_inv = math::inversed(cam_w);

                    const m4f& m_v = cam_w_inv.second
                        ? cam_w_inv.first
                        : m4f::identity();
                    const m4f& m_p = cam.projection();

                    props.property(matrix_v_property_hash, m_v)
                        .property(matrix_p_property_hash, m_p)
                        .property(matrix_vp_property_hash, m_v * m_p)
                        .property(time_property_hash, the<engine>().time());

                    cbuf = render_.create_const_buffer(
                        pass.templ,
                        const_buffer::scope::render_pass);
                    render_.update_buffer(cbuf, props);
                }
            } else {
                rp.color_clear(cam.background())
                  .color_store()
                  .depth_clear(1.0f)
                  .depth_discard();
            }
            
            rp.target(cam.target())
              .viewport(cam.viewport());

            auto encoder = rq->create_pass(rp, {}, cbuf);

            owner.set_event(render_system::render_with_camera_evt{encoder});

            rq->submit();
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

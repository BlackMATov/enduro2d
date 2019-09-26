/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/input_event_system.hpp>
#include <enduro2d/high/components/screenspace_collider.hpp>
#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/touchable.hpp>
#include <enduro2d/high/world_ev.hpp>

namespace
{
    using namespace e2d;

    std::tuple<m4f,b2f> get_vp_and_viewport(
        const ecs::const_entity& e,
        const camera& cam) noexcept
    {
        const actor* cam_a = e.find_component<actor>();
        const m4f& cam_w = cam_a && cam_a->node()
            ? cam_a->node()->world_matrix()
            : m4f::identity();
        const auto cam_w_inv = math::inversed(cam_w);
        const m4f& m_v = cam_w_inv.second
            ? cam_w_inv.first
            : m4f::identity();
        const m4f& m_p = cam.projection();

        return std::make_tuple(
            m_v *  m_p,
            cam.viewport().cast_to<f32>());
    }
    
    using input_event_type = input_event::event_type;
    
    template < typename Collider >
    void raycast_on_convex_hull(ecs::registry& owner, const input_event::data_ptr& ev_data) {
        owner.for_joined_components<Collider, touchable, actor>(
        [&owner, &ev_data]
        (const ecs::entity& e, const Collider& shape, const touchable& t, const actor& act) {
            const v2f touch_center = ev_data->center;
            const f32 touch_radius = ev_data->radius;

            bool inside = true;
            for ( auto& p : shape.planes ) {
                const f32 d = math::dot(p.norm, touch_center) + p.dist;
                inside &= (d > -touch_radius);
            }
            if ( inside ) {
                u32 d = act.node()->render_order();
                owner.assign_component<touchable::capture>(e, d, t.stop_propagation(), ev_data);
            }
        });
    }

    void raycast_on_polygon(ecs::registry& owner, const input_event::data_ptr& ev_data) {
        owner.for_joined_components<polygon_screenspace_collider, touchable, actor>(
        [&owner, &ev_data]
        (const ecs::entity& e, const polygon_screenspace_collider& shape, const touchable& t, const actor& act) {
            const v2f touch_center = ev_data->center;
            const f32 touch_radius = ev_data->radius;
            
            bool intersects = false;
            for ( auto& tri : shape.triangles ) {
                bool inside = true;
                for ( auto& p : tri.planes ) {
                    const f32 d = math::dot(p.norm, touch_center) + p.dist;
                    inside &= (d > -touch_radius);
                }
                if ( inside ) {
                    intersects = true;
                    break;
                }
            }
            if ( intersects ) {
                u32 d = act.node()->render_order();
                owner.assign_component<touchable::capture>(e, d, t.stop_propagation(), ev_data);
            }
        });
    }
}

namespace e2d
{
    //
    // internal_state
    //

    class input_event_system::internal_state final {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;
        
        void pre_update(ecs::registry& owner);
        void raycast(ecs::registry& owner);
        void post_update(ecs::registry& owner);
    private:
        v2f mouse_delta_;
        v2f last_cursor_pos_ {-1.0e10f};
        u32 frame_id_ = 0;
    };

    void input_event_system::internal_state::pre_update(ecs::registry& owner) {
        owner.remove_all_components<input_event>();
        owner.remove_all_components<touch_down_event>();
        owner.remove_all_components<touch_up_event>();
        owner.remove_all_components<touch_move_event>();
        owner.remove_all_components<mouse_enter_event>();
        owner.remove_all_components<mouse_leave_event>();
        owner.remove_all_components<mouse_move_event>();
        
        const mouse& m = the<input>().mouse();
            
        input_event_type ev_type = input_event_type(0);
        mouse_delta_ = v2f();

        if ( m.is_button_just_pressed(mouse_button::left) ) {
            ev_type = input_event_type::touch_down;
            mouse_delta_ = v2f();
            last_cursor_pos_ = m.cursor_pos();

        } else if ( m.is_button_just_released(mouse_button::left) ) {
            ev_type = input_event_type::touch_up;

        } else if ( !math::approximately(last_cursor_pos_, m.cursor_pos(), 0.1f) ) {
            if ( m.is_button_pressed(mouse_button::left) ) {
                ev_type = input_event_type::touch_move;
            } else {
                ev_type = input_event_type::mouse_move;
            }
            mouse_delta_ = m.cursor_pos() - last_cursor_pos_;
            last_cursor_pos_ = m.cursor_pos();

        } else {
            return;
        }

        ecs::const_entity cam_e(owner);
        b2f viewport;
        m4f vp;

        owner.for_joined_components<camera::input_handler_tag, camera>(
        [&cam_e, &viewport, &vp, pos = m.cursor_pos()]
        (const ecs::const_entity& e, camera::input_handler_tag, const camera& cam) {
            if ( cam.target() ) {
                return;
            }
            viewport = cam.viewport().cast_to<f32>();

            if ( !math::inside(viewport, pos) ) {
                return;
            }
            cam_e = e;
            vp = std::get<0>(get_vp_and_viewport(e, cam));
        });

        if ( cam_e.valid() ) {
            input_event::data_ptr ev_data(new input_event::event_data{
                vp,
                viewport,
                v2f(m.cursor_pos().x, viewport.size.y - m.cursor_pos().y),
                v2f(mouse_delta_.x, -mouse_delta_.y),
                4.0f,
                ev_type});
            
            ecs::entity(owner, cam_e.id()).assign_component<input_event>(ev_data);

            switch ( ev_type ) {
                case input_event_type::mouse_move:
                case input_event_type::touch_down:
                    owner.add_event(world_ev::input_event_raycast());
                    break;

                case input_event_type::touch_up:
                    owner.for_each_component<touched_tag>(
                    [&owner, ev_data](ecs::entity e, touched_tag) {
                        e.assign_component<touch_up_event>(touch_up_event{ev_data});
                    });
                    owner.remove_all_components<touch_focus_tag>();
                    owner.remove_all_components<touched_tag>();
                    break;

                case input_event_type::touch_move:
                    owner.for_each_component<touch_focus_tag>(
                    [&owner, ev_data](ecs::entity e, touch_focus_tag) {
                        e.assign_component<touch_move_event>(touch_move_event{ev_data});
                    });
                    break;
            }
        }
        owner.add_event(world_ev::input_event_post_update());
        owner.add_event(world_ev::update_ui_style());
    }
    
    void input_event_system::internal_state::raycast(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner](const ecs::const_entity&, const input_event& input_ev, camera::input_handler_tag, const camera&) {
            if ( input_ev.data()->type != input_event::event_type::mouse_move &&
                 input_ev.data()->type != input_event::event_type::touch_down )
            {
                return;
            }

            raycast_on_convex_hull<rectangle_screenspace_collider>(owner, input_ev.data());
            raycast_on_convex_hull<circle_screenspace_collider>(owner, input_ev.data());
            raycast_on_polygon(owner, input_ev.data());
        });
    }

    void input_event_system::internal_state::post_update(ecs::registry& owner) {
        const auto add_mouse_leave_events = [this, &owner](const input_event::data_ptr& ev_data) {
            owner.for_each_component<mouse_over_tag>(
            [&owner, ev_data, fid = frame_id_](ecs::entity e, const mouse_over_tag& tag) {
                if ( tag.frame_id != fid ) {
                    e.assign_component<mouse_leave_event>(mouse_leave_event{ev_data});
                }
            });
            owner.for_each_component<mouse_leave_event>(
            [&owner](ecs::entity e, const mouse_leave_event&) {
                e.remove_component<mouse_over_tag>();
            });
        };

        ecs::entity_id capture_id;
        input_event::data_ptr ev_data;
        u32 depth = 0;
        bool stop_prop = false;

        owner.for_each_component<touchable::capture>(
        [&](ecs::entity_id e, const touchable::capture& c) {
            if ( c.depth() >= depth ) {
                stop_prop = c.stop_propagation();
                capture_id = e;
                depth = c.depth();
                ev_data = c.data();
            }
        });
        owner.remove_all_components<touchable::capture>();

        if ( !ev_data ) {
            // for mouse_move only
            owner.for_each_component<input_event>(
            [&ev_data](const ecs::const_entity&, const input_event& input) {
                if ( input.data()->type == input_event_type::mouse_move ) {
                    ev_data = input.data();
                }
            });
            if ( !ev_data ) {
                return;
            }

            ++frame_id_;
            add_mouse_leave_events(ev_data);
            return;
        }

        ++frame_id_;

        E2D_ASSERT(ev_data->type == input_event_type::mouse_move
            || ev_data->type == input_event_type::touch_down);

        auto add_tag = [&ev_data, fid = frame_id_](ecs::entity& e) {
            if ( ev_data->type == input_event_type::touch_down ) {
                e.assign_component<touched_tag>();
                e.assign_component<touch_down_event>(touch_down_event{ev_data});
            }
            if ( ev_data->type == input_event_type::mouse_move ) {
                if ( auto* tag = e.find_component<mouse_over_tag>() ) {
                    tag->frame_id = fid;
                    e.assign_component<mouse_move_event>(mouse_move_event{ev_data});
                } else {
                    e.assign_component<mouse_over_tag>(mouse_over_tag{fid});
                    e.assign_component<mouse_enter_event>(mouse_enter_event{ev_data});
                }
            }
        };

        ecs::entity_id last_touched = capture_id;
        ecs::entity top_e(owner, capture_id);

        add_tag(top_e);
           
        if ( !stop_prop ) {
            auto* act = top_e.find_component<actor>();
            if ( act && act->node() ) {
                for ( auto n = act->node()->parent(); n; n = n->parent() ) {
                    if ( !n->owner() ) {
                        continue;
                    }
                    auto* t_comp = n->owner()->entity().find_component<touchable>();
                    if ( t_comp ) {
                        ecs::entity e = n->owner()->entity();
                        last_touched = e.id();
                        add_tag(e);
                        if ( t_comp->stop_propagation() ) {
                            break;
                        }
                    }
                }
            }
        }
            
        if ( ev_data->type == input_event_type::touch_down ) {
            ecs::entity(owner, last_touched)
                .assign_component<touch_focus_tag>();
        }
            
        if ( ev_data->type == input_event_type::mouse_move ) {
            add_mouse_leave_events(ev_data);
        }
    }
    
    //
    // input_event_system
    //

    input_event_system::input_event_system()
    : state_(std::make_unique<internal_state>()) {}

    input_event_system::~input_event_system() = default;

    void input_event_system::pre_update(ecs::registry& owner) {
        state_->pre_update(owner);
    }

    void input_event_system::raycast(ecs::registry& owner) {
        state_->raycast(owner);
    }

    void input_event_system::post_update(ecs::registry& owner) {
        state_->post_update(owner);
    }
}
/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/input_event_system.hpp>
#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/touchable.hpp>

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
}

namespace e2d
{
    using input_event_type = input_event::event_type;

    void input_event_system_per_update::process(ecs::registry& owner) {
        owner.remove_all_components<input_event>();

        const mouse& m = the<input>().mouse();
            
        u32 ev_type = 0;
        mouse_delta_ = v2f();

        if ( m.is_button_just_pressed(mouse_button::left) ) {
            math::set_flags_inplace(ev_type, input_event_type::touch_down);
        }
        if ( m.is_button_just_released(mouse_button::left) ) {
            math::set_flags_inplace(ev_type, input_event_type::touch_up);
            math::set_flags_inplace(ev_type, input_event_type::mouse_move);
        }
        if ( !math::approximately(last_cursor_pos_, m.cursor_pos(), 0.1f) ) {
            if ( m.is_button_pressed(mouse_button::left) ) {
                math::set_flags_inplace(ev_type, input_event_type::touch_move);
            }
            mouse_delta_ = m.cursor_pos() - last_cursor_pos_;
            last_cursor_pos_ = m.cursor_pos();
            math::set_flags_inplace(ev_type, input_event_type::mouse_move);
        }

        if ( !ev_type ) {
            return;
        }

        ecs::const_entity cam_e(owner);
        b2f viewport;
        m4f vp;

        owner.for_joined_components<camera::input_handler_tag, camera>(
        [&cam_e, &viewport, &vp, pos = m.cursor_pos()](const ecs::const_entity& e, camera::input_handler_tag, const camera& cam) {
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
                input_event_type(ev_type)});
                
            // TODO: for input focus - save camera entity to allow event processing outside of the viewport
            bool has_focus = false;
            owner.for_joined_components<touchable::input_focus_tag, touchable>(
            [&owner, &has_focus, ev_data](ecs::entity_id e, touchable::input_focus_tag, const touchable& t) {
                has_focus |= true;
                ecs::entity(owner, e)
                    .assign_component<touchable::capture>(t.depth(), t.stop_propagation(), ev_data);
            });

            if ( !has_focus ) {
                ecs::entity(owner, cam_e.id())
                    .assign_component<input_event>(ev_data);
            }
        }

        // reset previous state
                
        if ( math::check_any_flags(ev_type, input_event_type::mouse_move) ) {
            /*owner.for_joined_components<touchable::mouse_over_tag, input_callback>(
            [&owner](ecs::entity_id e, const mouse_over_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                input_event_data ev_data;
                ev_data.type = input_event_type::mouse_over;
                cb.call(ent, false, ev_data);
            });*/
            owner.remove_all_components<touchable::mouse_over_tag>();
        }

        if ( math::check_any_flags(ev_type, input_event_type::touch_down) ) {
            /*owner.for_joined_components<touched_tag, input_callback>(
            [&owner](ecs::entity_id e, const touched_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                input_event_data ev_data;
                ev_data.type = input_event_type::touch_down;
                cb.call(ent, false, ev_data);
            });*/
            owner.remove_all_components<touchable::touched_tag>();
        }

        if ( !math::check_any_flags(ev_type, input_event_type::touch_up) ) {
            /*owner.for_joined_components<untouched_tag, input_callback>(
            [&owner](ecs::entity_id e, const untouched_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                input_event_data ev_data;
                ev_data.type = input_event_type::touch_up;
                cb.call(ent, false, ev_data);
            });*/
            owner.remove_all_components<touchable::untouched_tag>();
        }
           
        if ( math::check_any_flags(ev_type, input_event_type::touch_up) ) {
            /*owner.for_joined_components<touch_move_tag, input_callback>(
            [&owner](ecs::entity_id e, const touch_move_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                input_event_data ev_data;
                ev_data.type = input_event_type::touch_move;
                cb.call(ent, false, ev_data);
            });*/
            owner.remove_all_components<touchable::touch_move_tag>();
        }
    }
    
    void input_event_system_post_update::process(ecs::registry& owner) {
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
            return;
        }

        auto add_tag = [ev_type = u32(ev_data->type)](ecs::entity& e) {
            if ( math::check_any_flags(ev_type, input_event_type::mouse_move) ) {
                e.assign_component<touchable::mouse_over_tag>();
            }
            if ( math::check_any_flags(ev_type, input_event_type::touch_down) ) {
                e.assign_component<touchable::touched_tag>();
            }
            if ( math::check_any_flags(ev_type, input_event_type::touch_up) ) {
                e.assign_component<touchable::untouched_tag>();
            }
            if ( math::check_any_flags(ev_type, input_event_type::touch_move) ) {
                e.assign_component<touchable::touch_move_tag>();
            }
        };

        ecs::entity e(owner, capture_id);
        add_tag(e);
           
        if ( !stop_prop ) {
            auto* act = e.find_component<actor>();
            if ( act && act->node() ) {
                for ( auto n = act->node()->parent(); n; n = n->parent() ) {
                    if ( !n->owner() ) {
                        continue;
                    }
                    auto* t_comp = n->owner()->entity().find_component<touchable>();
                    if ( t_comp ) {
                        add_tag(n->owner()->entity());
                        if ( t_comp->stop_propagation() ) {
                            break;
                        }
                    }
                }
            }
        }
            
        if ( !!(ev_data->type & input_event_type::mouse_move) ) {
            /*owner.for_joined_components<mouse_over_tag, input_callback>(
            [&owner, &ev_data](ecs::entity_id e, const mouse_over_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                auto data = *ev_data;
                data.type = input_event_type::mouse_over;
                cb.call(ent, true, data);
            });*/
        }
        if ( !!(ev_data->type & input_event_type::touch_down) ) {
            /*owner.for_joined_components<touched_tag, input_callback>(
            [&owner, &ev_data](ecs::entity_id e, const touched_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                auto data = *ev_data;
                data.type = input_event_type::touch_down;
                cb.call(ent, true, data);
            });*/
        }
        if ( !!(ev_data->type & input_event_type::touch_move) ) {
            /*owner.for_joined_components<touch_move_tag, input_callback>(
            [&owner, &ev_data](ecs::entity_id e, const touch_move_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                auto data = *ev_data;
                data.type = input_event_type::touch_move;
                cb.call(ent, true, data);
            });*/
        }
        if ( !!(ev_data->type & input_event_type::touch_up) ) {
            /*owner.for_joined_components<untouched_tag, input_callback>(
            [&owner, &ev_data](ecs::entity_id e, const untouched_tag&, input_callback& cb) {
                ecs::entity ent(owner, e);
                auto data = *ev_data;
                data.type = input_event_type::touch_up;
                cb.call(ent, true, data);
            });*/
        }
    }
}

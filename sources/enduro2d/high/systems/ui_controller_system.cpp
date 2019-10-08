/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/ui_controller.hpp>
#include <enduro2d/high/components/ui_style.hpp>

#include <enduro2d/high/single_components/frame_params_comp.hpp>

namespace
{
    using namespace e2d;

    using ui_style_state = ui_style::ui_style_state;

    void process_buttons(ecs::registry& owner) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_button, ui_style>(
        [](ecs::entity e, const touch_up_event& touch, const ui_button&, ui_style& style) {
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::touched);
            style.set(ui_style_state::touched, false);
            e.ensure_component<ui_controller_events>()
                .add_event(ui_button::click_evt{touch.data->center});
        });
    }

    void process_selectable(ecs::registry& owner) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_selectable, ui_style>(
        [](ecs::entity e, const touch_up_event&, ui_selectable& sel, ui_style& style) {
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::touched)
                .set(ui_style_state::selected);
            style.set(ui_style_state::touched, false);
            sel.selected(!sel.selected());
            style.set(ui_style_state::selected, sel.selected());
            e.ensure_component<ui_controller_events>()
                .add_event(ui_selectable::changed_evt{sel.selected()});
        });
    }
    
    struct ui_draggable_private {
        v3f start_pos;
        v3f node_pos;
        v3f diff;
        bool started = false;
    };

    void process_draggable(ecs::registry& owner) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_draggable, ui_style>(
        [](ecs::entity e, const touch_up_event&, const ui_draggable&, ui_style& style) {
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::dragging)
                .set(ui_style_state::touched);
            style.set(ui_style_state::dragging, false);
            style.set(ui_style_state::touched, false);

            auto& drg = e.ensure_component<ui_draggable_private>();
            if ( drg.started ) {
                drg.started = false;
                e.ensure_component<ui_controller_events>()
                    .add_event(ui_draggable::drag_end_evt{});
            }
        });
        
        // process touch move events
        owner.for_joined_components<touch_move_event, ui_draggable, ui_style, actor>(
        [](ecs::entity e, const touch_move_event& ev, const ui_draggable&, ui_style& style, actor& act) {
            auto mvp = act.node()->world_matrix() * ev.data->view_proj;
            auto mvp_inv = math::inversed(mvp, 0.0f).first;
            const f32 z = 0.0f;
            v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
            v3f old_point = math::unproject(v3f(ev.data->center + ev.data->delta, z), mvp_inv, ev.data->viewport);
            auto& events = e.ensure_component<ui_controller_events>();
            auto& drg = e.ensure_component<ui_draggable_private>();

            // start dragging
            if ( !drg.started ) {
                style.set(ui_style_state::dragging, true);
                e.ensure_component<ui_style::style_changed_bits>()
                    .set(ui_style_state::dragging);
                drg.started = true;
                drg.diff = v3f();
                drg.start_pos = old_point; // touch down point in local coordinates
                drg.node_pos = act.node()->translation();
                events.add_event(ui_draggable::drag_begin_evt{});
            }
            
            v3f delta = (old_point - new_point) * act.node()->scale();
            v3f diff = drg.node_pos - act.node()->translation();
            drg.diff += diff;
 
            auto test = [&](int c) {
                // draggable component may be clamped to some area,
                // if this happens 'drg.diff' will be non-zero value
                if ( math::abs(drg.diff[c]) > 0.1f ) {
                    // pause dragging until cursor is not in start point
                    if ( (new_point[c] < drg.start_pos[c]) ^ math::sign(drg.diff[c]) ) {
                        drg.diff[c] = 0.0f;
                        delta[c] = new_point[c] - drg.start_pos[c];
                    } else {
                        delta[c] = 0.0f;
                    }
                }
            };
            test(0);
            test(1);
            test(2);

            act.node()->translation(act.node()->translation() + delta);
            drg.node_pos = act.node()->translation();

            events.add_event(ui_draggable::drag_update_evt{});
        });
    }
    
    struct ui_scrollable_private {
        enum class scroll_mode : u8 {
            idle,
            manual,
            inertion,
            overscroll
        };

        v3f velocity;
        scroll_mode mode = scroll_mode::idle;
        
        //static constexpr f32 v2_friction = 0.1f;
        static constexpr f32 v_friction = 0.8f;
        static constexpr f32 c_friction = 20.0f;
        static constexpr f32 wheel_scale = 40.0f;
        static constexpr f32 min_velocity = 0.1f;

        // for overscroll
        static constexpr f32 damping = 0.1f;
    };

    //struct inertion_scrolling_tag {};

    v3i int_direction(const v3f& v, f32 precission = 1.0e-4f) noexcept {
        return {
            math::abs(v.x) < precission ? 0 : v.x > 0.0f ? 1 : -1,
            math::abs(v.y) < precission ? 0 : v.y > 0.0f ? 1 : -1,
            math::abs(v.z) < precission ? 0 : v.z > 0.0f ? 1 : -1 };
    }

    void process_scrollable(ecs::registry& owner) {
        using scroll_mode = ui_scrollable_private::scroll_mode;

        auto& params = owner.get_single_component<frame_params_comp>();
        const f32 dt = params.delta_time.value;

        E2D_ASSERT(!math::is_near_zero(dt));

        // process touch down event
        owner.for_joined_components<touch_down_event, ui_scrollable, actor>(
        [](ecs::entity e, const touch_down_event& ev, const ui_scrollable&, const actor& act) {
            auto& scr_p = e.ensure_component<ui_scrollable_private>();
            if ( scr_p.mode == scroll_mode::inertion ) {
                // continue scrolling
                scr_p.mode = scroll_mode::manual;
                
                auto mvp = act.node()->world_matrix() * ev.data->view_proj;
                auto mvp_inv = math::inversed(mvp, 0.0f).first;
                const f32 z = 0.0f;
                v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
            }
        });

        // process touch up event
        owner.for_joined_components<touch_up_event, ui_scrollable, ui_style>(
        [](ecs::entity e, const touch_up_event&, const ui_scrollable&, ui_style& style) {
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::touched);
            style.set(ui_style_state::touched, false);

            auto& scr_p = e.ensure_component<ui_scrollable_private>();
            if ( scr_p.mode == scroll_mode::manual ) {
                scr_p.mode = scroll_mode::inertion;
                scr_p.velocity = v3f(); // TODO
            }
        });
        
        // process touch move events
        owner.for_joined_components<touch_move_event, ui_scrollable, ui_style, actor>(
        [](ecs::entity e, const touch_move_event& ev, const ui_scrollable&, ui_style& style, actor& act) {
            auto mvp = act.node()->world_matrix() * ev.data->view_proj;
            auto mvp_inv = math::inversed(mvp, 0.0f).first;
            const f32 z = 0.0f;
            v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
            v3f old_point = math::unproject(v3f(ev.data->center + ev.data->delta, z), mvp_inv, ev.data->viewport);
            auto& scr_p = e.ensure_component<ui_scrollable_private>();

            // start scrolling
            if ( scr_p.mode == scroll_mode::idle ) {
                style.set(ui_style_state::dragging, true);
                e.ensure_component<ui_style::style_changed_bits>()
                    .set(ui_style_state::dragging);
                scr_p.mode = scroll_mode::manual;
                e.ensure_component<ui_controller_events>()
                    .add_event(ui_scrollable::scroll_begin_evt{});
            }
            
            v3f delta = (old_point - new_point) * act.node()->scale();
            act.node()->translation(act.node()->translation() + delta);
        });

        // TODO: process mouse wheel events
        // TODO: process keyboard events ?

        // temp
        {
            const v2f scroll_delta = the<input>().mouse().scroll_delta();

            if ( !math::is_near_zero(scroll_delta.x) || !math::is_near_zero(scroll_delta.y) ) {
                owner.for_joined_components<ui_scrollable, ui_scrollable_private, actor>(
                [scroll_delta](ecs::entity e, const ui_scrollable&, ui_scrollable_private& scr_p, actor& act) {
                    if ( scr_p.mode == scroll_mode::idle ) {
                        scr_p.mode = scroll_mode::inertion;
                        e.ensure_component<ui_controller_events>()
                            .add_event(ui_scrollable::scroll_begin_evt{});
                    }
                    if ( scr_p.mode != scroll_mode::manual ) {
                        scr_p.velocity = scr_p.velocity + v3f(scroll_delta, 0.0f) * ui_scrollable_private::wheel_scale;
                    }
                });
            }
        }

        // update
        owner.for_joined_components<ui_scrollable, ui_scrollable_private, actor>(
        [dt](ecs::entity e, const ui_scrollable& scr, ui_scrollable_private& scr_p, actor& act) {
            auto& events = e.ensure_component<ui_controller_events>();

            auto calc_overscroll = [&act](const v3f& translation) {
                if ( auto parent = act.node()->parent() ) {
                    const b2f parent_r = b2f(parent->size());
                    const b2f self_r = b2f(act.node()->size()) + v2f(translation); // TODO: scale & rotation
                    v2f pos = self_r.position;

                    pos.x = math::max(pos.x + self_r.size.x, parent_r.position.x + parent_r.size.x) - self_r.size.x;
                    pos.y = math::max(pos.y + self_r.size.y, parent_r.position.y + parent_r.size.y) - self_r.size.y;
                    pos.x = math::min(pos.x, parent_r.position.x);
                    pos.y = math::min(pos.y, parent_r.position.y);

                    return v3f(pos - self_r.position, 0.f);
                }
                return v3f();
            };

            if ( scr_p.mode == scroll_mode::inertion ||
                 scr_p.mode == scroll_mode::overscroll )
            {
                v3f& vel = scr_p.velocity;
                const v3i old_dir = int_direction(vel);
                const f32 vel_l = math::length(vel);
                const bool has_vel = math::abs(vel_l) > ui_scrollable_private::min_velocity;

                if ( has_vel ) {
                    v3f pos = act.node()->translation();
                    v3f accel = -(vel * ui_scrollable_private::v_friction + (vel / vel_l) * ui_scrollable_private::c_friction);

                    // uniformly accelerated motion
                    pos += vel * dt * 0.5f;
                    vel += accel * dt;
                    pos += vel * dt * 0.5f;

                    v3f dpos = calc_overscroll(pos);
                    if ( dpos != v3f() ) {
                        if ( scr.overscroll_enabled() ) {
                            accel = -dpos * ui_scrollable_private::damping / dt;

                            // uniformly accelerated motion
                            pos += vel * dt * 0.5f;
                            vel += accel * dt;
                            pos += vel * dt * 0.5f;

                        } else {
                            pos += dpos;
                        }
                    }
                    
                    act.node()->translation(pos);


                    the<debug>().trace("scroll: vel: %0, accel: %1", vel, accel);
                }

                // if velocity direction has been changed -> stop scrolling
                const v3i new_dir = int_direction(vel);
                if ( /*old_dir != new_dir ||*/ !has_vel ) {
                    scr_p.mode = scroll_mode::idle;
                    scr_p.velocity = v3f();
                    events.add_event(ui_scrollable::scroll_end_evt{});
                    the<debug>().trace("scroll: stop");
                    return;
                }
            }

            events.add_event(ui_scrollable::scroll_update_evt{});
        });
    }
}

namespace e2d
{
    void ui_controller_system::process(ecs::registry& owner) {
        // process touch down event
        owner.for_joined_components<touch_down_event, ui_style>(
        [](ecs::entity e, const touch_down_event&, ui_style& style) {
            style.set(ui_style_state::touched, true);
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::touched);
        });
        
        // process mouse enter event
        owner.for_joined_components<mouse_enter_event, ui_style>(
        [](ecs::entity e, const mouse_enter_event&, ui_style& style) {
            style.set(ui_style_state::mouse_over, true);
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::mouse_over);
        });
        
        // process mouse leave event
        owner.for_joined_components<mouse_leave_event, ui_style>(
        [](ecs::entity e, const mouse_leave_event&, ui_style& style) {
            style.set(ui_style_state::mouse_over, false);
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::mouse_over);
        });

        process_buttons(owner);
        process_selectable(owner);
        process_draggable(owner);
        process_scrollable(owner);
    }
}

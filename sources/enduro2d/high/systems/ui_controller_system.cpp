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
        v3f start_pos;
        v3f node_pos;
        v3f diff;
        bool started = false;
    };

    void process_scrollable(ecs::registry& owner) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_scrollable, ui_style>(
        [](ecs::entity e, const touch_up_event&, const ui_scrollable&, ui_style& style) {
            e.ensure_component<ui_style::style_changed_bits>()
                .set(ui_style_state::dragging)
                .set(ui_style_state::touched);
            style.set(ui_style_state::dragging, false);
            style.set(ui_style_state::touched, false);

            auto& scr_p = e.ensure_component<ui_scrollable_private>();
            if ( scr_p.started ) {
                scr_p.started = false;
                e.ensure_component<ui_controller_events>()
                    .add_event(ui_scrollable::scroll_end_evt{});
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
            auto& events = e.ensure_component<ui_controller_events>();
            auto& scr_p = e.ensure_component<ui_scrollable_private>();

            // start scrolling
            if ( !scr_p.started ) {
                style.set(ui_style_state::dragging, true);
                e.ensure_component<ui_style::style_changed_bits>()
                    .set(ui_style_state::dragging);
                scr_p.started = true;
                scr_p.diff = v3f();
                scr_p.start_pos = old_point; // touch down point in local coordinates
                scr_p.node_pos = act.node()->translation();
                events.add_event(ui_scrollable::scroll_begin_evt{});
            }
            
            v3f delta = (old_point - new_point) * act.node()->scale();
            v3f diff = scr_p.node_pos - act.node()->translation();
            scr_p.diff += diff;
 
            auto test = [&](int c) {
                // draggable component may be clamped to some area,
                // if this happens 'scr_p.diff' will be non-zero value
                if ( math::abs(scr_p.diff[c]) > 0.1f ) {
                    // pause dragging until cursor is not in start point
                    if ( (new_point[c] < scr_p.start_pos[c]) ^ math::sign(scr_p.diff[c]) ) {
                        scr_p.diff[c] = 0.0f;
                        delta[c] = new_point[c] - scr_p.start_pos[c];
                    } else {
                        delta[c] = 0.0f;
                    }
                }
            };
            test(0);
            test(1);
            test(2);

            act.node()->translation(act.node()->translation() + delta);
            scr_p.node_pos = act.node()->translation();

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

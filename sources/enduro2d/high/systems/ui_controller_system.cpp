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
    using changed_states = ui_system::update_controllers_evt::changed_states;

    void process_buttons(ecs::registry& owner, changed_states& changed) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_button, ui_style>(
        [&changed](ecs::entity_id id, const touch_up_event&, const ui_button&, ui_style& style) {
            changed[id].set(ui_style_state::touched);
            style.set(ui_style_state::touched, false);
        });
    }

    void process_selectable(ecs::registry& owner, changed_states& changed) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_selectable, ui_style>(
        [&changed](ecs::entity_id id, const touch_up_event&, const ui_selectable&, ui_style& style) {
            changed[id].set(ui_style_state::touched)
                .set(ui_style_state::selected);
            style.set(ui_style_state::touched, false);
            style.set(ui_style_state::selected, !style[ui_style_state::selected]);
        });
    }

    void process_draggable(ecs::registry& owner, changed_states& changed) {
        // process touch up event
        owner.for_joined_components<touch_up_event, ui_draggable, ui_style>(
        [&changed](ecs::entity_id id, const touch_up_event&, ui_draggable& drg, ui_style& style) {
            changed[id].set(ui_style_state::dragging)
                .set(ui_style_state::touched);
            style.set(ui_style_state::dragging, false);
            style.set(ui_style_state::touched, false);
			drg.started = false;
        });
        
        // process touch move events
        owner.for_joined_components<touch_move_event, ui_draggable, ui_style, actor>(
        [&changed](ecs::entity_id id, const touch_move_event& ev, ui_draggable& drg, ui_style& style, actor& act) {
            auto mvp = act.node()->world_matrix() * ev.data->view_proj;
            auto mvp_inv = math::inversed(mvp, 0.0f).first;
            const f32 z = 0.0f;
            v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
            v3f old_point = math::unproject(v3f(ev.data->center + ev.data->delta, z), mvp_inv, ev.data->viewport);

			// start dragging
            if ( !drg.started ) {
                style.set(ui_style_state::dragging, true);
                changed[id].set(ui_style_state::dragging);
				drg.started = true;
				drg.diff = v3f();
                drg.start_pos = old_point; // touch down point in local coordinates
				drg.node_pos = act.node()->translation();
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
        });
    }
}

namespace e2d
{
    void ui_controller_system::process(ecs::registry& owner, ecs::event_ref event) {
        auto& changed = event.cast<ui_system::update_controllers_evt>().changed;

        // process touch down event
        owner.for_joined_components<touch_down_event, ui_style>(
        [&changed](ecs::entity_id id, const touch_down_event&, ui_style& style) {
            style.set(ui_style_state::touched, true);
            changed[id].set(ui_style_state::touched);
        });
        
        // process mouse enter event
        owner.for_joined_components<mouse_enter_event, ui_style>(
        [&changed](ecs::entity_id id, const mouse_enter_event&, ui_style& style) {
            style.set(ui_style_state::mouse_over, true);
            changed[id].set(ui_style_state::mouse_over);
        });
        
        // process mouse leave event
        owner.for_joined_components<mouse_leave_event, ui_style>(
        [&changed](ecs::entity_id id, const mouse_leave_event&, ui_style& style) {
            style.set(ui_style_state::mouse_over, false);
            changed[id].set(ui_style_state::mouse_over);
        });

        process_buttons(owner, changed);
        process_selectable(owner, changed);
        process_draggable(owner, changed);
    }
}

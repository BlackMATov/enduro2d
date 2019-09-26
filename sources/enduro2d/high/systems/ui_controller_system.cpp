/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_controller_system.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/ui_controller.hpp>
#include <enduro2d/high/components/ui_style.hpp>

namespace
{
    using namespace e2d;

    using ui_style_state = ui_style::ui_style_state;
    using changed_states = flat_map<ecs::entity_id, ui_style_state>;

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
        [&changed](ecs::entity_id id, const touch_up_event&, const ui_draggable&, ui_style& style) {
            changed[id].set(ui_style_state::dragging)
                .set(ui_style_state::touched);
            style.set(ui_style_state::dragging, false);
            style.set(ui_style_state::touched, false);
        });
        
        // process touch move events
        owner.for_joined_components<touch_move_event, ui_draggable, ui_style, actor>(
        [&changed](ecs::entity_id id, const touch_move_event& ev, const ui_draggable& drg, ui_style& style, actor& act) {
            if ( !style[ui_style_state::dragging] ) {
                style.set(ui_style_state::dragging, true);
                changed[id].set(ui_style_state::dragging);
            }
            auto m_model = act.node()->world_matrix();
            auto mvp_inv = math::inversed(m_model * ev.data->view_proj, 0.0f).first;
            const f32 z = 0.0f;
            v2f pos_delta = ev.data->delta * v2f(drg.lock_x() ? 0.f : 1.f, drg.lock_y() ? 0.f : 1.f);
            v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
            v3f old_point = math::unproject(v3f(ev.data->center + pos_delta, z), mvp_inv, ev.data->viewport);
            v3f delta = (old_point - new_point) * act.node()->scale();
            act.node()->translation(act.node()->translation() + delta);
        });
    }
    
    bool copy_flags_to(ui_style_state& changed, const ui_style& src, ui_style& dst) noexcept {
        changed.flags = ui_style::bits(changed.flags.to_ulong() & src.propagate().flags.to_ulong());
        if ( changed.flags.to_ulong() == 0 ) {
            return false;
        }
        for ( size_t i = 0; i < changed.flags.size(); ++i ) {
            if ( changed.flags[i] ) {
                dst.set(ui_style::type(i), src[ui_style::type(i)]);
            }
        }
        return true;
    }

    bool should_propagate(const ui_style& style, ui_style_state changed) noexcept {
        return changed.flags.to_ulong() & style.propagate().flags.to_ulong();
    }

    void propagate_new_style(ecs::registry& owner, const changed_states& changed) {
        // propagate style flags to childs
        struct child_visitor {
            void operator()(const node_iptr& n) const {
                if ( auto* dst_style = n->owner()->entity().find_component<ui_style>() ) {
                    n->owner()->entity_filler().component<ui_style::style_changed_tag>();
                    ui_style_state flags = changed;
                    if ( copy_flags_to(flags, style, *dst_style) ) {
                        child_visitor visitor{*dst_style, flags};
                        n->for_each_child(visitor);
                    }
                } else {
                    child_visitor visitor{style, changed};
                    n->for_each_child(visitor);
                }
            }
            ui_style const& style;
            ui_style_state changed;
        };

        for ( auto&[id, flags] : changed ) {
            ecs::entity e(owner, id);
            e.assign_component<ui_style::style_changed_tag>();

            auto[style, act] = e.find_components<ui_style, actor>();
            if ( style && act && act->node() && should_propagate(*style, flags) ) {
                child_visitor visitor{*style, flags};
                act->node()->for_each_child(visitor);
            }
        }
    }
}

namespace e2d
{
    ui_controller_system::ui_controller_system() = default;
    
    ui_controller_system::~ui_controller_system() noexcept = default;

    void ui_controller_system::process(ecs::registry& owner) {
        changed_states changed;

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

        propagate_new_style(owner, changed);
    }
}

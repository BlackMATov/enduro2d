/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_style_system.hpp>
#include <enduro2d/high/components/ui_style.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/sprite_9p_renderer.hpp>
#include <enduro2d/high/components/label.hpp>

namespace
{
    using namespace e2d;
    
    using ui_style_state = ui_style::ui_style_state::type;

    color32 get_current_color(const ui_style& state, const ui_color_style& style) noexcept {
        if ( state[ui_style_state::selected] ) {
            return style.selected();
        }
        if ( state[ui_style_state::dragging] ) {
            return style.dragging();
        }
        if ( state[ui_style_state::touched] ) {
            return style.touched();
        }
        if ( state[ui_style_state::mouse_over] ) {
            return style.mouse_over();
        }
        if ( state[ui_style_state::disabled] ) {
            return style.disabled();
        }
        return style.idle();
    }

    void update_sprite_color_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_color_style_comp, sprite_renderer>(
        [](const ecs::const_entity&,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_color_style_comp& color_style,
            sprite_renderer& spr)
        {
            spr.tint(get_current_color(state, color_style.style()->content()));
        });
    }

    void update_9patch_color_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_color_style_comp, sprite_9p_renderer>(
        [](const ecs::const_entity&,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_color_style_comp& color_style,
            sprite_9p_renderer& spr)
        {
            spr.tint(get_current_color(state, color_style.style()->content()));
        });
    }

    void update_label_color_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_color_style_comp, label>(
        [](ecs::entity e,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_color_style_comp& color_style,
            label& lbl)
        {
            lbl.tint(get_current_color(state, color_style.style()->content()));
            e.assign_component<label::dirty>();
        });
    }
}

namespace e2d
{
    ui_style_system::ui_style_system() = default;

    ui_style_system::~ui_style_system() noexcept = default;
    
    void ui_style_system::before_update(ecs::registry& owner) {
        owner.remove_all_components<ui_style::style_changed_tag>();
    }

    void ui_style_system::process(ecs::registry& owner) {
        update_sprite_color_style(owner);
        update_9patch_color_style(owner);
        update_label_color_style(owner);
    }
}

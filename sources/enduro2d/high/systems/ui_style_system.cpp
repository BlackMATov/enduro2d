/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/ui_style.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/sprite_9p_renderer.hpp>
#include <enduro2d/high/components/label.hpp>

namespace
{
    using namespace e2d;
    
    template < typename T >
    auto get_current_style(const ui_style& state, const T& style) noexcept {
        using ui_style_state = ui_style::ui_style_state::type;

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
            spr.tint(get_current_style(state, color_style.style()->content()));
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
            spr.tint(get_current_style(state, color_style.style()->content()));
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
            lbl.tint(get_current_style(state, color_style.style()->content()));
            e.assign_component<label::dirty>();
        });
    }

    void update_label_font_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_font_style_comp, label>(
        [](ecs::entity e,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_font_style_comp& font_style,
            label& lbl)
        {
            auto style = get_current_style(state, font_style.style()->content());
            lbl.tint(style.tint);
            lbl.outline_color(style.outline_color);
            lbl.outline_width(style.outline_width);
            e.assign_component<label::dirty>();
        });
    }

    void update_sprite_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_sprite_style_comp, sprite_renderer>(
        [](const ecs::const_entity&,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_sprite_style_comp& sprite_style,
            sprite_renderer& spr)
        {
            spr.sprite(get_current_style(state, sprite_style.style()->content()));
        });
    }
    
    void update_sprite_9p_style(ecs::registry& owner) {
        owner.for_joined_components<ui_style::style_changed_tag, ui_style, ui_sprite_9p_style_comp, sprite_9p_renderer>(
        [](const ecs::const_entity&,
            ui_style::style_changed_tag,
            const ui_style& state,
            const ui_sprite_9p_style_comp& sprite_style,
            sprite_9p_renderer& spr)
        {
            spr.sprite(get_current_style(state, sprite_style.style()->content()));
        });
    }
    
    using ui_style_state = ui_style::ui_style_state;
    using changed_states = ui_system::update_controllers_evt::changed_states;

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
        // clear tag
        //owner.remove_all_components<ui_style::style_changed_tag>();

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
    void ui_style_system::process(ecs::registry& owner, ecs::event_ref event) {
        const auto& changed = event.cast<ecs::after_event<ui_system::update_controllers_evt>>().data.changed;
        propagate_new_style(owner, changed);

        update_sprite_color_style(owner);
        update_9patch_color_style(owner);
        update_label_color_style(owner);
        update_label_font_style(owner);
        update_sprite_style(owner);
        update_sprite_9p_style(owner);
    }
}

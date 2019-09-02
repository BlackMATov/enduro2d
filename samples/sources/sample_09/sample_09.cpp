/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    class button final {
    public:
        button() = default;

        button& selectable(bool value) noexcept { selectable_ = value; return *this; }
        bool selectable() const noexcept { return selectable_; }

        button& draggable(bool value) noexcept { draggable_ = value; return *this; }
        bool draggable() const noexcept { return draggable_; }
    private:
        bool selectable_ = false;
        bool draggable_ = false;
    };

    class button_color_style final {
    public:
        color32 disabled = color32(125, 125, 125, 255);
        color32 idle = color32::white();
        color32 mouse_over = color32::white();
        color32 touched = color32::white();
        color32 selected = color32::white();
        color32 dragging = color32::white();
    };
    using button_color_style_ptr = std::shared_ptr<const button_color_style>;

    class button_color_style_ref final {
    public:
        button_color_style_ptr ref;

        button_color_style_ref(const button_color_style_ptr& p) : ref(p) {}
    };

    class ui_style_state final {
    public:
        enum type {
            disabled,
            mouse_over,
            touched,
            selected,
            dragging,
            count_
        };
        using bits = std::bitset<u32(count_)>;
        
    public:
        ui_style_state& set(type flag, bool value = true) noexcept {
            flags.set(u32(flag), value);
            return *this;
        }

        ui_style_state& set_all() noexcept {
            flags = bits(~0ull);
            return *this;
        }

        bool get(type flag) const noexcept {
            return flags[u32(flag)];
        }

        bits flags;
    };

    class ui_style final {
    public:
        class style_changed_tag {};
        using type = ui_style_state::type;
        using bits = ui_style_state::bits;
    public:
        ui_style() = default;
        ui_style(const ui_style&) = default;

        ui_style& propagate(type flag, bool value) noexcept {
            propagate_bits_.set(flag, value);
            return *this;
        }

        ui_style& propagate_all() noexcept {
            propagate_bits_.set_all();
            return *this;
        }

        bool propagate(type flag) const noexcept {
            return propagate_bits_.get(flag);
        }

        bool should_propagate(ui_style_state changed) const noexcept {
            return changed.flags.to_ulong() & propagate_bits_.flags.to_ulong();
        }

        void set(type flag, bool value) noexcept {
            bits_.set(flag, value);
        }

        bool operator[](type flag) const noexcept {
            return bits_.get(flag);
        }

        bool copy_to(ui_style_state& changed, ui_style& dst) const noexcept {
            changed.flags = bits(changed.flags.to_ulong() & propagate_bits_.flags.to_ulong());
            if ( changed.flags.to_ulong() == 0 ) {
                return false;
            }
            for ( size_t i = 0; i < changed.flags.size(); ++i ) {
                if ( changed.flags[i] ) {
                    dst.bits_.flags[i] = bits_.flags[i];
                }
            }
            return true;
        }
    private:
        ui_style_state bits_;
        ui_style_state propagate_bits_;
    };
    
    class button_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            flat_map<ecs::entity_id, ui_style_state> changed;

            // process touch down event
            owner.for_joined_components<touch_down_event, button, ui_style>(
            [&changed](ecs::entity_id id, const touch_down_event&, const button&, ui_style& style) {
                style.set(ui_style_state::touched, true);
                changed[id].set(ui_style_state::touched);
            });
            
            // process touch up event
            owner.for_joined_components<touch_up_event, button, ui_style>(
            [&changed](ecs::entity_id id, const touch_up_event&, const button& btn, ui_style& style) {
                auto& flags = changed[id]
                    .set(ui_style_state::dragging)
                    .set(ui_style_state::touched);
                style.set(ui_style_state::dragging, false);
                style.set(ui_style_state::touched, false);
                if ( btn.selectable() ) {
                    style.set(ui_style_state::selected, !style[ui_style_state::selected]);
                    flags.set(ui_style_state::selected);
                }
            });

            // process touch move events
            owner.for_joined_components<touch_move_event, button, ui_style, actor>(
            [&changed](ecs::entity_id id, const touch_move_event& ev, const button& btn, ui_style& style, actor& act) {
                if ( !btn.draggable() ) {
                    return;
                }
                if ( !style[ui_style_state::dragging] ) {
                    style.set(ui_style_state::dragging, true);
                    changed[id].set(ui_style_state::dragging);
                }
                auto m_model = act.node()->world_matrix();
                auto mvp_inv = math::inversed(m_model * ev.data->view_proj, 0.0f).first;
                const f32 z = 0.0f;
                v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
                v3f old_point = math::unproject(v3f(ev.data->center + ev.data->delta, z), mvp_inv, ev.data->viewport);
                v3f delta = (old_point - new_point) * act.node()->scale();
                act.node()->translation(act.node()->translation() + delta);
            });
            
            // process mouse enter event
            owner.for_joined_components<mouse_enter_event, button, ui_style>(
            [&changed](ecs::entity_id id, const mouse_enter_event&, const button&, ui_style& style) {
                style.set(ui_style_state::mouse_over, true);
                changed[id].set(ui_style_state::mouse_over);
            });

            // process mouse leave event
            owner.for_joined_components<mouse_leave_event, button, ui_style>(
            [&changed](ecs::entity_id id, const mouse_leave_event&, const button&, ui_style& style) {
                style.set(ui_style_state::mouse_over, false);
                changed[id].set(ui_style_state::mouse_over);
            });

            // propagate style flags to childs
            struct child_visitor {
                void operator()(const node_iptr& n) const {
                    if ( auto* st = n->owner()->entity().find_component<ui_style>() ) {
                        n->owner()->entity_filler().component<ui_style::style_changed_tag>();
                        ui_style_state flags = changed;
                        if ( style.copy_to(flags, *st) ) {
                            child_visitor visitor{*st, flags};
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
                if ( style && act && act->node() && style->should_propagate(flags) ) {
                    child_visitor visitor{*style, flags};
                    act->node()->for_each_child(visitor);
                }
            }
        }
    };

    class ui_style_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_joined_components<ui_style::style_changed_tag, ui_style, button_color_style_ref, sprite_renderer>(
            [](const ecs::const_entity&,
               ui_style::style_changed_tag,
               const ui_style& state,
               const button_color_style_ref& color_style,
               sprite_renderer& spr)
            {
                if ( state[ui_style_state::selected] ) {
                    spr.tint(color_style.ref->selected);
                } else if ( state[ui_style_state::dragging] ) {
                    spr.tint(color_style.ref->dragging);
                } else if ( state[ui_style_state::touched] ) {
                    spr.tint(color_style.ref->touched);
                } else if ( state[ui_style_state::mouse_over] ) {
                    spr.tint(color_style.ref->mouse_over);
                } else if ( state[ui_style_state::disabled] ) {
                    spr.tint(color_style.ref->disabled);
                } else {
                    spr.tint(color_style.ref->idle);
                }
            });
            owner.for_joined_components<ui_style::style_changed_tag, ui_style, button_color_style_ref, label>(
            [&owner](ecs::entity_id id,
               ui_style::style_changed_tag,
               const ui_style& state,
               const button_color_style_ref& color_style,
               label& lbl)
            {
                if ( state[ui_style_state::selected] ) {
                    lbl.tint(color_style.ref->selected);
                } else if ( state[ui_style_state::dragging] ) {
                    lbl.tint(color_style.ref->dragging);
                } else if ( state[ui_style_state::touched] ) {
                    lbl.tint(color_style.ref->touched);
                } else if ( state[ui_style_state::mouse_over] ) {
                    lbl.tint(color_style.ref->mouse_over);
                } else if ( state[ui_style_state::disabled] ) {
                    lbl.tint(color_style.ref->disabled);
                } else {
                    lbl.tint(color_style.ref->idle);
                }
                ecs::entity(owner, id).assign_component<label::dirty>();
            });
            owner.remove_all_components<ui_style::style_changed_tag>();
        }
    };

    class game_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            E2D_UNUSED(owner);
            const keyboard& k = the<input>().keyboard();

            if ( k.is_key_just_released(keyboard_key::f12) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_just_released(keyboard_key::escape) ) {
                the<window>().set_should_close(true);
            }

            if ( k.is_key_pressed(keyboard_key::lsuper) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }
        }
    };

    class camera_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_each_component<camera>(
            [](const ecs::const_entity&, camera& cam){
                if ( !cam.target() ) {
                    cam.viewport(
                        the<window>().real_size());
                    cam.projection(math::make_orthogonal_lh_matrix4(
                        the<window>().real_size().cast_to<f32>(), 0.f, 1000.f));
                }
            });
        }
    };

    class game final : public starter::application {
    public:
        bool initialize() final {
            return create_scene()
                && create_camera()
                && create_systems();
        }
    private:
        bool create_scene() {
            auto button_res = the<library>().load_asset<sprite_asset>("button_sprite.json");
            auto sprite_mat = the<library>().load_asset<material_asset>("sprite_material.json");
            auto font_mat = the<library>().load_asset<material_asset>("font_sdf_material.json");
            auto font = the<library>().load_asset<font_asset>("arial_sdf.fnt");

            if ( !button_res || !sprite_mat || !font_mat || !font ) {
                return false;
            }

            button_color_style_ptr window_style;
            button_color_style_ptr title_style;
            button_color_style_ptr title_label_style;
            button_color_style_ptr button_style;
            {
                button_color_style style;
                style.idle = color32(0, 0, 140, 255);
                style.mouse_over = style.idle;
                style.touched = style.idle;
                style.dragging = color32(0, 0, 255, 255);
                style.selected = style.idle;
                window_style = std::make_shared<const button_color_style>(style);
            }
            {
                button_color_style style;
                style.idle = color32(0, 100, 140, 255);
                style.mouse_over = color32(0, 150, 140, 255);
                style.touched = style.mouse_over;
                style.dragging = color32(0, 100, 255, 255);
                style.selected = style.idle;
                title_style = std::make_shared<const button_color_style>(style);
            }
            {
                button_color_style style;
                style.idle = color32(100, 200, 255, 255);
                style.mouse_over = color32(200, 0, 220, 255);
                style.touched = style.mouse_over;
                style.dragging = color32(255, 100, 255, 255);
                style.selected = style.idle;
                title_label_style = std::make_shared<const button_color_style>(style);
            }
            {
                button_color_style style;
                style.idle = color32(200, 200, 200, 255);
                style.mouse_over = color32(255, 200, 200, 255);
                style.touched = color32(200, 255, 200, 255);
                style.selected = color32(200, 255, 100, 255);
                style.dragging = style.idle;
                button_style = std::make_shared<const button_color_style>(style);
            }

            auto scene_i = the<world>().instantiate();

            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i));

            node_iptr scene_r = scene_i->get_component<actor>().get().node();

            auto window_i = the<world>().instantiate();

            window_i->entity_filler()
                .component<actor>(node::create(window_i, scene_r))
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(button_res)
                .component<touchable>(true)
                .component<button>(button()
                    .draggable(true))
                .component<ui_style>(ui_style()
                    .propagate(ui_style_state::dragging, true))
                .component<button_color_style_ref>(window_style);
            
            node_iptr window_n = window_i->get_component<actor>().get().node();
            window_n->scale(v3f(2.0f, 2.0f, 1.f));
            window_n->translation(v3f{0.f, 0.f, 0.f});
            
            {
                auto title_i = the<world>().instantiate();
                title_i->entity_filler()
                    .component<actor>(node::create(title_i, window_n))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(button_res)
                    .component<touchable>(false)
                    .component<rectangle_shape>(b2f(
                        button_res->content().texrect().position - button_res->content().pivot(),
                        button_res->content().texrect().size))
                    .component<ui_style>(ui_style()
                        .propagate_all())
                    .component<button>()
                    .component<button_color_style_ref>(title_style);
                
                node_iptr title_n = title_i->get_component<actor>().get().node();
                title_n->scale(v3f(1.0f, 0.1f, 1.f));
                title_n->translation(v3f{0.f, 110.f, 0.f});
            
                auto title_label_i = the<world>().instantiate();
                title_label_i->entity_filler()
                    .component<actor>(node::create(title_label_i, title_n))
                    .component<renderer>(renderer()
                        .materials({font_mat}))
                    .component<model_renderer>()
                    .component<label>(label()
                        .font(font)
                        .text("title")
                        .haligh(label::haligns::center)
                        .valigh(label::valigns::center))
                    .component<label::dirty>()
                    .component<ui_style>()
                    .component<button>()
                    .component<button_color_style_ref>(title_label_style);
                
                node_iptr title_label_n = title_label_i->get_component<actor>().get().node();
                title_label_n->scale(v3f(0.5f, 5.f, 1.f));
                title_label_n->translation(v3f{0.f, 0.f, 0.f});
            }
            {
                auto button_i = the<world>().instantiate();
                button_i->entity_filler()
                    .component<actor>(node::create(button_i, window_n))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(button_res)
                    .component<touchable>(true)
                    .component<rectangle_shape>(b2f(
                        button_res->content().texrect().position - button_res->content().pivot(),
                        button_res->content().texrect().size))
                    .component<ui_style>()
                    .component<button>(button()
                        .selectable(true))
                    .component<button_color_style_ref>(button_style);
                
                node_iptr button_n = button_i->get_component<actor>().get().node();
                button_n->scale(v3f(0.2f, 0.2f, 1.f));
                button_n->translation(v3f{-40.f, -50.f, 0.f});
                button_n->rotation(math::make_quat_from_axis_angle(make_deg(45.f), v3f::unit_z()));
            }
            {
                auto button_i = the<world>().instantiate();
                button_i->entity_filler()
                    .component<actor>(node::create(button_i, window_n))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(button_res)
                    .component<touchable>(true)
                    .component<rectangle_shape>(b2f(
                        button_res->content().texrect().position - button_res->content().pivot(),
                        button_res->content().texrect().size))
                    .component<ui_style>()
                    .component<button>(button())
                    .component<button_color_style_ref>(button_style);
                
                node_iptr button_n = button_i->get_component<actor>().get().node();
                button_n->scale(v3f(0.2f, 0.2f, 1.f));
                button_n->translation(v3f{40.f, -50.f, 0.f});
            }
            {
                auto button_i = the<world>().instantiate();
                button_i->entity_filler()
                    .component<actor>(node::create(button_i, window_n))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(button_res)
                    .component<touchable>(true)
                    .component<circle_shape>(
                        (button_res->content().texrect().position - button_res->content().pivot() +
                         button_res->content().texrect().size * 0.5f),
                        math::max(button_res->content().texrect().size.x, button_res->content().texrect().size.y) * 0.5f)
                    .component<ui_style>()
                    .component<button>(button())
                    .component<button_color_style_ref>(button_style);
                
                node_iptr button_n = button_i->get_component<actor>().get().node();
                button_n->scale(v3f(0.2f, 0.2f, 1.f));
                button_n->translation(v3f{0.f, 0.f, 0.f});
            }

            the<world>().registry().for_each_component<ui_style>(
            [](ecs::entity_id id, const ui_style&) {
                ecs::entity(the<world>().registry(), id)
                    .assign_component<ui_style::style_changed_tag>();
            });
            return true;
        }

        bool create_camera() {
            auto camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera::input_handler_tag>()
                .component<camera>(camera()
                    .background({1.f, 0.4f, 0.f, 1.f}))
                .component<actor>(node::create(camera_i));
            return true;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update)
                .system<button_system>(world::priority_update)
                .system<ui_style_system>(world::priority_update + 100)
                .system<camera_system>(world::priority_pre_render);
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters("sample_09", "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

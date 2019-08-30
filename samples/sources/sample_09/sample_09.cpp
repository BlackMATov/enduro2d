/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    using input_event_type = input_event::event_type;
    
    class rotator final {
    public:
        rotator(const v3f& axis, f32 delay)
        : axis(axis), delay(delay) {}
    public:
        v3f axis;
        f32 delay;
    };

    class button final {
    public:
        button() = default;

        button& selected(bool value) noexcept { selected_ = value; return *this; }
        button& selectable(bool value) noexcept { selectable_ = value; return *this; }

        bool selected() const noexcept { return selected_; }
        bool selectable() const noexcept { return selectable_; }
    private:
        bool selected_ = false;
        bool selectable_ = false;
    };

    class draggable final {
    };

    class button_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            bool has_touch_down = false;
            owner.for_each_component<input_event>(
            [&has_touch_down](const ecs::const_entity&, const input_event& input) {
                has_touch_down |= (input.data()->type == input_event_type::touch_down);
            });

            // reset selected state
            if ( has_touch_down ) {
                owner.for_joined_components<button, sprite_renderer>(
                [](const ecs::const_entity& e, button& btn, sprite_renderer& spr) {
                    if ( btn.selectable() && btn.selected() && !e.find_component<touch_down_event>() ) {
                        btn.selected(false);
                        spr.tint(color32::white());
                    }
                });
            }

            // process touch down event
            owner.for_joined_components<touch_down_event, button, sprite_renderer>(
            [](ecs::entity_id id, const touch_down_event&, button& btn, sprite_renderer& spr) {
                if ( btn.selectable() ) {
                    btn.selected(!btn.selected());
                }
                spr.tint(btn.selected() ? color32::blue() : color32::red());
            });
            
            // process touch up event
            owner.for_joined_components<touch_up_event, button, sprite_renderer>(
            [](ecs::entity_id, const touch_up_event&, const button& btn, sprite_renderer& spr) {
                spr.tint(btn.selected() ? color32::blue() : color32::white());
            });

            // process mouse enter event
            owner.for_joined_components<mouse_enter_event, button, sprite_renderer>(
            [](ecs::entity_id, const mouse_enter_event&, const button& btn, sprite_renderer& spr) {
                spr.tint(color32::magenta());
            });

            // process mouse leave event
            owner.for_joined_components<mouse_leave_event, button, sprite_renderer>(
            [](ecs::entity_id, const mouse_leave_event&, const button& btn, sprite_renderer& spr) {
                spr.tint(btn.selected() ? color32::blue() : color32::white());
            });
        }
    };

    class draggable_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_joined_components<touch_down_event, draggable, sprite_renderer>(
            [](const ecs::const_entity& e, const touch_down_event&, const draggable&, sprite_renderer& spr) {
                spr.tint(color32::red());
            });
            
            owner.for_joined_components<touch_up_event, draggable, sprite_renderer>(
            [](const ecs::const_entity& e, const touch_up_event&, const draggable&, sprite_renderer& spr) {
                spr.tint(color32::green());
            });
            
            owner.for_joined_components<touch_move_event, draggable, sprite_renderer, actor>(
            [](const ecs::const_entity& e, const touch_move_event& ev, const draggable&, sprite_renderer& spr, actor& act) {
                auto m_model = act.node()->world_matrix();
                auto mvp_inv = math::inversed(m_model * ev.data->view_proj, 0.0f).first;
                const f32 z = 0.0f;
                v3f new_point = math::unproject(v3f(ev.data->center, z), mvp_inv, ev.data->viewport);
                v3f old_point = math::unproject(v3f(ev.data->center + ev.data->delta, z), mvp_inv, ev.data->viewport);
                v3f delta = (old_point - new_point) * act.node()->scale();
                act.node()->translation(act.node()->translation() + delta);
            });
        }
    };

    class rotator_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            const f32 time = the<engine>().time();
            owner.for_joined_components<rotator, actor>(
                [&time](const ecs::const_entity&, const rotator& rot, actor& act){
                    const node_iptr node = act.node();
                    if ( node ) {
                        const q4f q = math::make_quat_from_axis_angle(make_rad(time + rot.delay), rot.axis);
                        node->rotation(q);
                    }
                });
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

            if ( !button_res || !sprite_mat ) {
                return false;
            }

            auto scene_i = the<world>().instantiate();

            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i));

            node_iptr scene_r = scene_i->get_component<actor>().get().node();

            auto background_i = the<world>().instantiate();

            background_i->entity_filler()
                .component<actor>(node::create(background_i, scene_r))
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(button_res)
                .component<touchable>(true);

            node_iptr background_n = background_i->get_component<actor>().get().node();
            background_n->scale(v3f(2.0f, 2.0f, 1.0f));
            background_n->translation(v3f{0.0f, 0.f, 0.0f});
            
            {
                auto sprite_i = the<world>().instantiate();
                sprite_i->entity_filler()
                    .component<actor>(node::create(sprite_i, scene_r))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(sprite_renderer(button_res)
                        .tint(color32::green()))
                    .component<touchable>(true)
                    .component<rectangle_shape>(b2f(
                        button_res->content().texrect().position - button_res->content().pivot(),
                        button_res->content().texrect().size))
                    .component<draggable>();

                node_iptr sprite_n = sprite_i->get_component<actor>().get().node();
                sprite_n->scale(v3f(0.4f, 0.4f, 1.f));
                sprite_n->translation(v3f{80.f, 80.f, 0});
            }

            {
                for ( std::size_t i = 0; i < 3; ++i )
                for ( std::size_t j = 0; j < 3; ++j ) {
                    auto sprite_i = the<world>().instantiate();
                    bool stop_prop = !((i|j)&1);

                    sprite_i->entity_filler()
                        .component<actor>(node::create(sprite_i, background_n))
                        .component<rotator>(rotator{v3f::unit_z(), i*10.0f+j})
                        .component<renderer>(renderer()
                            .materials({sprite_mat}))
                        .component<sprite_renderer>(button_res)
                        .component<touchable>(stop_prop)
                        .component<rectangle_shape>(b2f(
                            button_res->content().texrect().position - button_res->content().pivot(),
                            button_res->content().texrect().size))
                        .component<button>(button().selectable(true));

                    node_iptr sprite_n = sprite_i->get_component<actor>().get().node();
                    sprite_n->scale(v3f(0.02f, 0.02f, 1.f));
                    sprite_n->translation(v3f{j * 3.f, i * 3.f, 0} - v3f(10.0f, 6.0f, 0.0f));
                }
            }
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
                .system<rotator_system>(world::priority_update)
                .system<button_system>(world::priority_update)
                .system<draggable_system>(world::priority_update)
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

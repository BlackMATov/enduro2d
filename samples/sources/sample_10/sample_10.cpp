/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
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
            auto sprite_res = the<library>().load_asset<sprite_asset>("rect_sprite.json");
            auto sprite_mat = the<library>().load_asset<material_asset>("sprite_material.json");

            if ( !sprite_res || !sprite_mat ) {
                return false;
            }
            
            auto scene_i = the<world>().instantiate();
            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i))
                .component<ui_layout>()
                .component<ui_layout::root_tag>();
            node_iptr scene_r = scene_i->get_component<actor>().get().node();
            
            auto auto_i = the<world>().instantiate();
            auto_i->entity_filler()
                .component<actor>(node::create(auto_i, scene_r))
                .component<ui_layout>()
                .component<auto_layout>()
                .component<auto_layout::dirty_flag>();
            node_iptr auto_n = auto_i->get_component<actor>().get().node();
            
            auto image_i = the<world>().instantiate();
            image_i->entity_filler()
                .component<actor>(node::create(image_i, auto_n))
                .component<ui_layout>()
                .component<image_layout>(image_layout()
                    .preserve_aspect(false))
                .component<image_layout::dirty_flag>()
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(sprite_renderer(sprite_res)
                    .tint(color32::white()));

            auto stack_i = the<world>().instantiate();
            stack_i->entity_filler()
                .component<actor>(node::create(stack_i, auto_n))
                .component<ui_layout>()
                .component<stack_layout>(stack_layout::stack_origin::left)
                .component<stack_layout::dirty_flag>();
            node_iptr stack_n = stack_i->get_component<actor>().get().node();
            stack_n->scale(v3f(1.0f));
            
            auto fixed1_i = the<world>().instantiate();
            fixed1_i->entity_filler()
                .component<actor>(node::create(fixed1_i, stack_n))
                .component<ui_layout>()
                .component<fixed_layout>(v2f(64.0f, 64.0f))
                .component<fixed_layout::dirty_flag>()
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(sprite_renderer(sprite_res)
                    .tint(color32::green()));
            node_iptr fixed1_n = fixed1_i->get_component<actor>().get().node();
            fixed1_n->scale(v3f(1.0f));
            
            auto fixed2_i = the<world>().instantiate();
            fixed2_i->entity_filler()
                .component<actor>(node::create(fixed2_i, stack_n))
                .component<ui_layout>()
                .component<fixed_layout>(v2f(64.0f, 64.0f))
                .component<fixed_layout::dirty_flag>()
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(sprite_renderer(sprite_res)
                    .tint(color32::red()));
            node_iptr fixed2_n = fixed2_i->get_component<actor>().get().node();
            fixed2_n->scale(v3f(2.0f));
            
            auto fixed3_i = the<world>().instantiate();
            fixed3_i->entity_filler()
                .component<actor>(node::create(fixed3_i, stack_n))
                .component<ui_layout>()
                .component<fixed_layout>(v2f(64.0f, 64.0f))
                .component<fixed_layout::dirty_flag>()
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(sprite_renderer(sprite_res)
                    .tint(color32::blue()));
            node_iptr fixed3_n = fixed3_i->get_component<actor>().get().node();
            fixed3_n->scale(v3f(1.0f));
            /*
            auto fixed4_i = the<world>().instantiate();
            fixed4_i->entity_filler()
                .component<actor>(node::create(fixed4_i, scene_r))
                .component<ui_layout>()
                .component<fixed_layout>(v2f(64.0f, 64.0f))
                .component<fixed_layout::dirty_flag>()
                .component<renderer>(renderer()
                    .materials({sprite_mat}))
                .component<sprite_renderer>(sprite_renderer(sprite_res)
                    .tint(color32::magenta()));
            node_iptr fixed4_n = fixed4_i->get_component<actor>().get().node();
            fixed4_n->translation(v3f(64.0f, 128.0f, 0.0f));
            fixed4_n->scale(v3f(1.0f));
            */
            return true;
        }

        bool create_camera() {
            auto camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera>(camera()
                    .background({0.f, 0.f, 0.f, 1.f}))
                .component<actor>(node::create(camera_i));
            return true;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update)
                .system<camera_system>(world::priority_pre_render);
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters("sample_10", "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

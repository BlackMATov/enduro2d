/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    struct rotator {
        v3f axis;
    };

    class game_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::entity_id) override {
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
            
            owner.process_systems_in_range(
                ecs::entity_id(0),
                world::priority_update_section_begin,
                world::priority_update_section_end);
        }
    };

    class camera_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::entity_id) override {
            static vector<std::pair<ecs::const_entity,camera&>> temp_components;
            try {
                temp_components.reserve(owner.component_count<camera>());
                owner.for_joined_components<camera>([](const ecs::const_entity& e, camera& cam){
                    temp_components.emplace_back(e, cam);
                });
                std::sort(
                    temp_components.begin(),
                    temp_components.end(),
                    [](const auto& l, const auto& r){
                        return l.second.depth() < r.second.depth();
                    });
                for ( auto[e, cam] : temp_components ) {
                    if ( !cam.target() ) {
                        cam.viewport(
                            the<window>().real_size());
                        cam.projection(math::make_orthogonal_lh_matrix4(
                            the<window>().real_size().cast_to<f32>(), 0.f, 1000.f));
                        
                        the<render>().execute(render::command_block<3>()
                            .add_command(render::target_command(cam.target()))
                            .add_command(render::viewport_command(cam.viewport()))
                            .add_command(render::clear_command()
                                .color_value(cam.background())));

                        owner.process_systems_in_range(
                            e.id(),
                            world::priority_render_section_begin,
                            world::priority_render_section_end);
                    }
                }
            } catch (...) {
                temp_components.clear();
                throw;
            }
            temp_components.clear();
        }
    };

    class rotator_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::entity_id) override {
            const f32 time = the<engine>().time();
            owner.for_joined_components<rotator, actor>(
                [&time](const ecs::const_entity&, const rotator& rot, actor& act){
                    const node_iptr node = act.node();
                    if ( node ) {
                        const q4f q = math::make_quat_from_axis_angle(make_rad(time), rot.axis);
                        node->rotation(q);
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
            auto model_res = the<library>().load_asset<model_asset>("gnome_model.json");
            auto model_mat = the<library>().load_asset<material_asset>("gnome_material.json");
            auto sprite_res = the<library>().load_asset<sprite_asset>("ship_sprite.json");
            auto sprite_mat = the<library>().load_asset<material_asset>("sprite_material.json");
            auto flipbook_res = the<library>().load_asset<flipbook_asset>("cube_flipbook.json");

            if ( !model_res || !model_mat || !sprite_res || !sprite_mat || !flipbook_res ) {
                return false;
            }

            auto scene_i = the<world>().instantiate();

            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i));

            node_iptr scene_r = scene_i->get_component<actor>().get().node();

            {
                auto model_i = the<world>().instantiate();

                model_i->entity_filler()
                    .component<rotator>(rotator{v3f::unit_y()})
                    .component<actor>(node::create(model_i, scene_r))
                    .component<renderer>(renderer()
                        .materials({model_mat}))
                    .component<model_renderer>(model_res);

                node_iptr model_n = model_i->get_component<actor>().get().node();
                model_n->scale(v3f{20.f});
                model_n->translation(v3f{0.f,50.f,0.f});
            }

            {
                auto sprite_i = the<world>().instantiate();

                sprite_i->entity_filler()
                    .component<rotator>(rotator{v3f::unit_z()})
                    .component<actor>(node::create(sprite_i, scene_r))
                    .component<renderer>(renderer()
                        .materials({sprite_mat}))
                    .component<sprite_renderer>(sprite_res);

                node_iptr sprite_n = sprite_i->get_component<actor>().get().node();
                sprite_n->translation(v3f{0,-50.f,0});
            }

            {
                for ( std::size_t i = 0; i < 2; ++i )
                for ( std::size_t j = 0; j < 5; ++j ) {
                    auto flipbook_i = the<world>().instantiate();

                    flipbook_i->entity_filler()
                        .component<actor>(node::create(flipbook_i, scene_r))
                        .component<renderer>(renderer()
                            .materials({sprite_mat}))
                        .component<sprite_renderer>(sprite_renderer()
                            .filtering(false))
                        .component<flipbook_source>(flipbook_res)
                        .component<flipbook_player>(flipbook_player()
                            .play("idle")
                            .looped(true));

                    node_iptr flipbook_n = flipbook_i->get_component<actor>().get().node();
                    flipbook_n->scale(v3f(2.f,2.f,1.f));
                    flipbook_n->translation(v3f{-80.f + j * 40.f, -200.f + i * 40.f, 0});
                }
            }

            return true;
        }

        bool create_camera() {
            auto camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera>(camera()
                    .background({1.f, 0.4f, 0.f, 1.f}))
                .component<actor>(node::create(camera_i));
            return true;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update_scheduler)
                .system<rotator_system>(world::priority_pre_update)
                .system<camera_system>(world::priority_render_scheduler);
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters("sample_03", "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

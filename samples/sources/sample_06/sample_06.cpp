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

            // use keys R, J, G to start animations
            const bool roar = k.is_key_just_pressed(keyboard_key::r);
            const bool jump = k.is_key_just_pressed(keyboard_key::j);
            const bool gun_grab = k.is_key_just_pressed(keyboard_key::g);

            if ( roar || jump || gun_grab ) {
                owner.for_each_component<spine_player>([
                    roar, jump, gun_grab
                ](ecs::entity e, const spine_player& p) {
                    if ( roar && p.has_animation("roar") ) {
                        e.ensure_component<spine_player_cmd>().add_command(
                            spine_player_cmd::set_anim_cmd(0, "roar")
                                .complete_message("to_walk"));
                    } else if ( jump && p.has_animation("jump") ) {
                        e.ensure_component<spine_player_cmd>().add_command(
                            spine_player_cmd::set_anim_cmd(0, "jump")
                                .complete_message("to_walk"));
                    } else if ( gun_grab && p.has_animation("gun-grab") ) {
                        e.ensure_component<spine_player_cmd>()
                            .add_command(spine_player_cmd::set_anim_cmd(1, "gun-grab"))
                            .add_command(spine_player_cmd::add_anim_cmd(1, "gun-holster")
                                .delay(secf(3.f)));
                    }
                });
            }

            owner.for_joined_components<spine_player_evt>([
            ](ecs::entity e, const spine_player_evt& pe) {
                for ( const auto& evt : pe.events() ) {
                    if ( auto complete_evt = std::get_if<spine_player_evt::complete_evt>(&evt);
                        complete_evt && complete_evt->message() == "to_walk" )
                    {
                        e.ensure_component<spine_player_cmd>().add_command(
                            spine_player_cmd::add_anim_cmd(0, "walk")
                                .loop(true));
                    }
                }
            });
        }
    };

    class camera_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_joined_components<camera>(
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
                && create_systems();
        }
    private:
        bool create_scene() {
            auto scene_prefab_res = the<library>().load_asset<prefab_asset>("scenes/spine_scene_prefab.json");
            auto scene_go = scene_prefab_res
                ? the<world>().instantiate(scene_prefab_res->content())
                : nullptr;
            return !!scene_go;
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
        engine::parameters("sample_06", "enduro2d")
            .window_params(engine::window_parameters()
                .size({1024, 768}))
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

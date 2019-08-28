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
                owner.for_each_component<spine_player>(
                [&owner, roar, jump, gun_grab] (ecs::entity_id e, const spine_player& player) {
                    if ( !player.has_animation("walk") ) {
                        return;
                    }
                    auto& events = ecs::entity(owner, e).assign_component<spine_animation_event>();
                    if ( roar ) {
                        events.commands().push_back(spine_animation_event::set_anim{0, "roar", false, "1"});
                    } else if ( jump ) {
                        events.commands().push_back(spine_animation_event::set_anim{0, "jump", false, "1"});
                    } else if ( gun_grab ) {
                        events.commands().push_back(spine_animation_event::set_anim{1, "gun-grab", false, ""});
                        events.commands().push_back(spine_animation_event::add_anim{1, "gun-holster", false, secf(3.0f), ""});
                    }
                });
            }
            
            owner.for_joined_components<spine_player::on_complete_event, spine_player>(
            [&owner](ecs::entity_id e, const spine_player::on_complete_event& events, spine_player& player) {
                auto& new_events = ecs::entity(owner, e).assign_component<spine_animation_event>();

                for ( auto& ev : events.completed() ) {
                    if ( ev == "1" ) {
                        new_events.commands().push_back(spine_animation_event::add_anim{0, "walk", true, secf(), ""});
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
                && create_camera()
                && create_systems();
        }
    private:
        bool create_scene() {
            auto scene_prefab_res = the<library>().load_asset<prefab_asset>("scene_spine_prefab.json");
            auto scene_go = scene_prefab_res
                ? the<world>().instantiate(scene_prefab_res->content())
                : nullptr;
            return !!scene_go;
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
                .system<game_system>(world::priority_update)
                .system<camera_system>(world::priority_pre_render);
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters("sample_07", "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100)));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

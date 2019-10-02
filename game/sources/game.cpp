/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "common.hpp"
using namespace e2d;

namespace
{
    class game_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref) override {
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
        void process(ecs::registry& owner, ecs::event_ref) override {
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

    class ui_event_system final : public ecs::system {
    public:
        ui_event_system() {
            // main menu
            callbacks_["boost_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["game_view_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["map_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["settings_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["store_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["by_soft_currency_btn"] = [](){ E2D_ASSERT(false); };
            callbacks_["by_hard_currency_btn"] = [](){ E2D_ASSERT(false); };

            // overview
        }

        void process(ecs::registry& owner, ecs::event_ref) override {
            owner.for_joined_components<ui_controller_event_name, ui_controller_events>(
            [this](const ecs::const_entity&, const ui_controller_event_name& evt_name, const ui_controller_events& events){
                for ( auto& ev : events.events() ) {
                    if ( !std::any_cast<ui_button::click_evt>(&ev) ) {
                        continue;
                    }
                    auto iter = callbacks_.find(evt_name.name());
                    if ( iter == callbacks_.end() ) {
                        continue;
                    }
                    iter->second();
                }
            });
        }
    private:
        using callback_t = void (*)();
        hash_map<str, callback_t> callbacks_;
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
            //auto scene_prefab_res = the<library>().load_asset<prefab_asset>("main/ui.json");
            auto scene_prefab_res = the<library>().load_asset<prefab_asset>("overview/ui.json");
            auto scene_go = scene_prefab_res
                ? the<world>().instantiate(scene_prefab_res->content())
                : nullptr;
            return !!scene_go;
        }

        bool create_camera() {
            auto camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera>(camera()
                    .background({0.f, 0.f, 0.f, 1.f}))
                .component<actor>(node::create(camera_i))
                .component<camera::input_handler_tag>();
            return true;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system, world_ev::update>()
                .system<camera_system, world_ev::pre_update>()
                .system<ui_event_system, world_ev::post_update>();
            return true;
        }
    };
}

int e2d_main(int argc, char *argv[]) {
    const auto starter_params = starter::parameters(
        engine::parameters("game", "enduro2d")
            .timer_params(engine::timer_parameters()
                .maximal_framerate(100))
            .window_params(engine::window_parameters()
                .size({960, 1280})));
    modules::initialize<starter>(argc, argv, starter_params).start<game>();
    modules::shutdown<starter>();
    return 0;
}

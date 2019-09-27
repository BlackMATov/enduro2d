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
        game_system(gobject_iptr raptor)
        : raptor_gobj_(raptor) {}

        ~game_system() noexcept override {}

        void process(ecs::registry& owner) override {
            E2D_UNUSED(owner);
            const keyboard& k = the<input>().keyboard();

            if ( k.is_key_just_released(keyboard_key::f1) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_just_released(keyboard_key::escape) ) {
                the<window>().set_should_close(true);
            }

            if ( k.is_key_pressed(keyboard_key::lsuper) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }
            
            // use keys R, J, G to start animations
            if ( raptor_gobj_ ) {
                if ( k.is_key_just_pressed(keyboard_key::r) ) {
                    auto player = raptor_gobj_->get_component<spine_player>();
                    if ( player ) {
                        (*player).set_animation(0, "roar")
                            .add_animation(0, "walk", true);
                    }
                }
                if ( k.is_key_just_pressed(keyboard_key::j) ) {
                    auto player = raptor_gobj_->get_component<spine_player>();
                    if ( player ) {
                        (*player).set_animation(0, "jump")
                            .add_animation(0, "walk", true);
                    }
                }
                if ( k.is_key_just_pressed(keyboard_key::g) ) {
                    auto player = raptor_gobj_->get_component<spine_player>();
                    if ( player ) {
                        (*player).set_animation(1, "gun-grab")
                            .add_animation(1, "gun-holster", secf(3.0f));
                    }
                }
            }
        }

    private:
        gobject_iptr raptor_gobj_;
    };
    
    class camera_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override {
            owner.for_each_component<camera>(
            [this](const ecs::const_entity&, camera& cam){
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
            auto spine_raptor = the<library>().load_asset<spine_model_asset>("spine_raptor.json");
            auto spine_coin = the<library>().load_asset<spine_model_asset>("spine_coin.json");
            auto spine_mat = the<library>().load_asset<material_asset>("spine_material.json");

            if ( !spine_raptor || !spine_coin || !spine_mat ) {
                return false;
            }
            
            auto scene_i = the<world>().instantiate();

            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i));

            node_iptr scene_r = scene_i->get_component<actor>().get().node();
            
        #if 1
            auto coin_i = the<world>().instantiate();
            coin_i->entity_filler()
                .component<actor>(node::create(coin_i, scene_r))
                .component<renderer>(renderer()
                    .materials({spine_mat}))
                .component<spine_renderer>(spine_renderer(spine_coin))
                .component<spine_player>(spine_player(spine_coin)
                    .set_animation(0, "animation", true));
            
            node_iptr coin_n = coin_i->get_component<actor>().get().node();
            coin_n->scale(v3f(0.125f));
            coin_n->translation(v3f{200.0f, 200.0f, 0.0f});

            raptor_gobj_ = the<world>().instantiate();
            raptor_gobj_->entity_filler()
                .component<actor>(node::create(raptor_gobj_, scene_r))
                .component<renderer>(renderer()
                    .materials({spine_mat}))
                .component<spine_renderer>(spine_renderer(spine_raptor))
                .component<spine_player>(spine_player(spine_raptor)
                    .set_animation(0, "walk", true));
            
            node_iptr raptor_n = raptor_gobj_->get_component<actor>().get().node();
            raptor_n->scale(v3f(0.25f));
            raptor_n->translation(v3f{-80.f, -100.f, 0.0f});
            
        #else
            // performace test
            for ( std::size_t i = 0; i < 20; ++i )
            for ( std::size_t j = 0; j < 40; ++j ) {
                auto spine_i = the<world>().instantiate();
                spine_i->entity_filler()
                    .component<actor>(node::create(spine_i, scene_r))
                    .component<renderer>(renderer()
                        .materials({spine_mat}))
                    .component<spine_renderer>(spine_renderer(spine_raptor))
                    .component<spine_player>(spine_player(spine_raptor)
                        .set_animation(0, "walk", true)
                        .add_animation(1, "gun-grab", false, secf(2.0f)));
            
                node_iptr spine_n = spine_i->get_component<actor>().get().node();
                spine_n->scale(v3f(0.05f));
                spine_n->translation(v3f{-400.f, -300.f, 0.0f} + v3f{j * 30.f, i * 30.f, 0});
            }
        #endif
            return true;
        }

        bool create_camera() {
            auto camera_prefab_res = the<library>().load_asset<prefab_asset>("camera_prefab.json");
            auto camera_go = camera_prefab_res
                ? the<world>().instantiate(camera_prefab_res->content())
                : nullptr;
            return !!camera_go;
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system>(world::priority_update, raptor_gobj_)
                .system<camera_system>(world::priority_pre_render)
                .system<spine_system>(world::priority_update);
            return true;
        }

    private:
        gobject_iptr raptor_gobj_;
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

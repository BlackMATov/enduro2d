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

    class ui_window_mngr final {
    public:
        ui_window_mngr(const gobject_iptr& scene_gobj)
        : scene_gobj_(scene_gobj)
        {
            // main menu
            callbacks_["boost_btn"] = [](auto){ E2D_ASSERT(false); };
            callbacks_["game_view_btn"] = [this](auto){ show_animated_window("overview"); };
            callbacks_["map_btn"] = [](auto){ E2D_ASSERT(false); };
            callbacks_["settings_btn"] = [](auto){ E2D_ASSERT(false); };
            callbacks_["store_btn"] = [](auto){ E2D_ASSERT(false); };
            callbacks_["by_soft_currency_btn"] = [](auto){ E2D_ASSERT(false); };
            callbacks_["by_hard_currency_btn"] = [](auto){ E2D_ASSERT(false); };

            // overview

            // shared
            callbacks_["close_window_btn"] = [this](ecs::entity& e){
                auto wnd_go = get_window_(e);
                if ( wnd_go ) {
                    close_animated_window(wnd_go);
                }
            };
            callbacks_["window_bg_mask"] = [this](ecs::entity& e){
                auto wnd_go = get_window_(e);
                if ( wnd_go ) {
                    close_animated_window(wnd_go);
                }
            };
        }

        void add_window(str name, const gobject_iptr& gobj) {
            window_map_.insert_or_assign(name, gobj);
        }

        bool load_window(const str_view address, str name, bool add_to_stack = false) {
            auto wnd_prefab_res = the<library>().load_asset<prefab_asset>(address);
            auto wnd_go = wnd_prefab_res
                ? the<world>().instantiate(wnd_prefab_res->content())
                : nullptr;
            if ( !wnd_go ) {
                E2D_ASSERT(false);
                return false;
            }
            if ( add_to_stack ) {
                show_window(wnd_go);
            }
            add_window(std::move(name), wnd_go);
            return true;
        }

        void show_window(const gobject_iptr& gobj) {
            for ( auto& w : window_stack_ ) {
                if ( gobj == w ) {
                    E2D_ASSERT(false);
                    return;
                }
            }
            window_stack_.push_back(gobj);

            auto& owner = the<world>().registry();
            owner.for_each_component<ui_layout::root_tag>(
            [&owner, &gobj](ecs::entity_id id, ui_layout::root_tag) {
                ecs::entity e(owner, id);
                actor& obj_a = gobj->get_component<actor>().get();
                e.get_component<actor>().node()->add_child_to_front(obj_a.node());
            });
        }

        void show_window(const str& name) {
            auto i = window_map_.find(name);
            if ( i != window_map_.end() ) {
                return show_window(i->second);
            } else {
                E2D_ASSERT(false);
            }
        }

        void show_animated_window(const gobject_iptr& gobj) {
            auto root = gobj->get_component<actor>().get().node();
            auto bg_node = the<world>().find_child_node(root, "window_bg_mask");
            auto wnd_node = the<world>().find_child_node(root, "window_form");
            if ( !bg_node || !wnd_node ) {
                E2D_ASSERT(false);
                return;
            }
            bg_node->owner()->entity().assign_component<ui_animation>(
                ui_animation::custom([](f32 f, sprite_renderer& spr) {
                    color32 c(20, 20, 20, 0);
                    c.a = u8(100.0f * f);
                    spr.tint(c);
                })
                .duration(secf(0.5f)));
            wnd_node->owner()->entity().assign_component<ui_animation>(
                ui_animation::custom([wnd_node](f32 f) {
                    wnd_node->scale(v3f(v2f(math::lerp(0.2f, 1.0f, f)), 1.0f));
                })
                .duration(secf(0.5f))
                .ease([](f32 t) { return easing::out_back(t, 2.0f); }));
            show_window(gobj);
        }
        
        void show_animated_window(const str& name) {
            auto i = window_map_.find(name);
            if ( i != window_map_.end() ) {
                return show_animated_window(i->second);
            } else {
                E2D_ASSERT(false);
            }
        }
        
        void close_window(const gobject_iptr& gobj) {
            for ( auto i = window_stack_.begin(); i != window_stack_.end(); ++i ) {
                if ( gobj == *i ) {
                    window_stack_.erase(i);
                    gobj->get_component<actor>().get().node()->remove_from_parent();
                    return;
                }
            }
            E2D_ASSERT(false);
        }

        bool has_window(const gobject_iptr& gobj) const {
            for ( auto i = window_stack_.begin(); i != window_stack_.end(); ++i ) {
                if ( gobj == *i ) {
                    return true;
                }
            }
            return false;
        }

        void close_animated_window(const gobject_iptr& gobj) {
            E2D_ASSERT(has_window(gobj));
            auto root = gobj->get_component<actor>().get().node();
            auto bg_node = the<world>().find_child_node(root, "window_bg_mask");
            auto wnd_node = the<world>().find_child_node(root, "window_form");
            if ( !bg_node || !wnd_node ) {
                E2D_ASSERT(false);
                return;
            }
            bg_node->owner()->entity().assign_component<ui_animation>(
                ui_animation::custom([](f32 f, sprite_renderer& spr) {
                    color32 c(20, 20, 20, 0);
                    c.a = u8(100.0f * (1.0f - f));
                    spr.tint(c);
                })
                .duration(secf(0.5f))
                .on_complete([this, go = gobj](auto...) {
                    close_window(go);
                }));
            wnd_node->owner()->entity().assign_component<ui_animation>(
                ui_animation::custom([wnd_node](f32 f) {
                    wnd_node->scale(v3f(v2f(math::lerp(1.0f, 0.2f, f)), 1.0f));
                })
                .duration(secf(0.5f))
                .ease(easing::in_cubic));
        }

        void close_animated_window(const str& name) {
            auto i = window_map_.find(name);
            if ( i != window_map_.end() ) {
                return close_animated_window(i->second);
            } else {
                E2D_ASSERT(false);
            }
        }

        void process_event(ecs::entity& e, str_hash name) {
            auto iter = callbacks_.find(name);
            if ( iter == callbacks_.end() ) {
                E2D_ASSERT(false);
                return;
            }
            iter->second(e);
        }

    private:
        gobject_iptr get_window_(const ecs::entity& el_e) {
            auto* el_a = el_e.find_component<actor>();
            if ( !el_a || !el_a->node() ) {
                return nullptr;
            }
            for ( auto wnd = window_stack_.rbegin(); wnd != window_stack_.rend(); ++wnd ) {
                auto* wnd_a = (*wnd)->entity().find_component<actor>();
                if ( !wnd_a || !wnd_a->node() ) {
                    continue;
                }
                for ( auto n = el_a->node(); n; n = n->parent() ) {
                    if ( n == wnd_a->node() ) {
                        return *wnd;
                    }
                }
            }
            return nullptr;
        }

    private:
        using callback_t = std::function<void (ecs::entity& e)>;
        hash_map<str_hash, callback_t> callbacks_;
        hash_map<str, gobject_iptr> window_map_;
        vector<gobject_iptr> window_stack_;
        gobject_iptr scene_gobj_;
    };
    using ui_window_mngr_ptr = std::shared_ptr<ui_window_mngr>;

    class ui_event_system final : public ecs::system {
    public:
        ui_event_system(const ui_window_mngr_ptr& ptr)
        : wnd_mngr_(ptr) {}

        void process(ecs::registry& owner) override {
            owner.for_joined_components<name_comp, ui_controller_events>(
            [this, &owner](ecs::entity e, const name_comp& evt_name, const ui_controller_events& events){
                for ( auto& ev : events.events() ) {
                    if ( !std::any_cast<ui_button::click_evt>(&ev) ) {
                        continue;
                    }
                    wnd_mngr_->process_event(e, evt_name.name());
                }
            });
        }
    private:
        ui_window_mngr_ptr wnd_mngr_;
    };

    class game final : public starter::application {
    public:
        bool initialize() final {
            return create_scene()
                && create_systems();
        }
    private:
        bool create_scene() {
            auto scene_prefab_res = the<library>().load_asset<prefab_asset>("scene.json");
            auto scene_go = scene_prefab_res
                ? the<world>().instantiate(scene_prefab_res->content())
                : nullptr;
            if ( !scene_go ) {
                return false;
            }

            wnd_mngr_ = std::make_shared<ui_window_mngr>(scene_go);
            return wnd_mngr_->load_window("main/ui.json", "main", true)
                && wnd_mngr_->load_window("overview/ui.json", "overview");
        }

        bool create_systems() {
            ecs::registry_filler(the<world>().registry())
                .system<game_system, world_ev::update>()
                .system<camera_system, world_ev::pre_update>()
                .system<ui_event_system, world_ev::post_update>(wnd_mngr_);
            return true;
        }
    private:
        ui_window_mngr_ptr wnd_mngr_;
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

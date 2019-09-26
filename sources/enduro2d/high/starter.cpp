/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/starter.hpp>

#include <enduro2d/high/world.hpp>
#include <enduro2d/high/world_ev.hpp>
#include <enduro2d/high/factory.hpp>
#include <enduro2d/high/library.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/flipbook_player.hpp>
#include <enduro2d/high/components/label.hpp>
#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/scene.hpp>
#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/spine_player_cmd.hpp>
#include <enduro2d/high/components/spine_player_evt.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/sprite_9p_renderer.hpp>
#include <enduro2d/high/components/shape2d.hpp>
#include <enduro2d/high/components/touchable.hpp>
#include <enduro2d/high/components/ui_layout.hpp>
#include <enduro2d/high/components/ui_controller.hpp>
#include <enduro2d/high/components/ui_style.hpp>

#include <enduro2d/high/systems/flipbook_system.hpp>
#include <enduro2d/high/systems/label_system.hpp>
#include <enduro2d/high/systems/render_system.hpp>
#include <enduro2d/high/systems/spine_systems.hpp>
#include <enduro2d/high/systems/shape_projection_system.hpp>
#include <enduro2d/high/systems/input_event_system.hpp>
#include <enduro2d/high/systems/ui_layout_system.hpp>
#include <enduro2d/high/systems/ui_controller_system.hpp>
#include <enduro2d/high/systems/ui_style_system.hpp>

namespace
{
    using namespace e2d;

    template < typename Module, typename... Args >
    Module& safe_module_initialize(Args&&... args) {
        return modules::is_initialized<Module>()
            ? modules::instance<Module>()
            : modules::initialize<Module>(std::forward<Args>(args)...);
    }

    class engine_application final : public engine::application {
    public:
        engine_application(starter::application_uptr application)
        : application_(std::move(application)) {}

        bool initialize() final {
            ecs::registry_filler(the<world>().registry())
                .system<flipbook_system>()
                .system<label_system>()
                .system<render_system>()
                .system<ui_layout_system>()
                .system<ui_controller_system>()
                .system<ui_style_system>()
                .system<input_event_system>()
                .system<shape_projection_system>()
                .system<spine_pre_system>()
                .system<spine_post_system>()
                .listener<world_ev::update_frame>(&flipbook_system::process)
                .listener<world_ev::update_frame>(&label_system::process)
                .listener<world_ev::update_frame>(&input_event_system::pre_update)
                .listener<ecs::before_event_ev<world_ev::input_event_raycast>>(&shape_projection_system::process)
                .listener<world_ev::input_event_raycast>(&input_event_system::raycast)
                .listener<world_ev::input_event_post_update>(&input_event_system::post_update)
                .listener<world_ev::update_ui_style>(&ui_controller_system::process)
                .listener<ecs::before_event_ev<world_ev::update_frame>>(&ui_style_system::before_update)
                .listener<world_ev::update_frame>(&ui_layout_system::process)
                .listener<ecs::after_event_ev<world_ev::update_ui_style>>(&ui_style_system::process)
                .listener<world_ev::render_frame>(&render_system::process)
                .listener<world_ev::update_frame>(&spine_pre_system::process)
                .listener<ecs::before_event_ev<world_ev::update_frame>>(&spine_post_system::process);
            return !application_ || application_->initialize();
        }

        void shutdown() noexcept final {
            if ( application_ ) {
                application_->shutdown();
            }
        }

        bool frame_tick() final {
            the<world>().registry().process_systems(world_ev::update_frame());
            return !the<window>().should_close()
                || (application_ && !application_->on_should_close());
        }

        void frame_render() final {
            the<world>().registry().process_systems(world_ev::render_frame());
        }
    private:
        starter::application_uptr application_;
    };
}

namespace e2d
{
    //
    // starter::application
    //

    bool starter::application::initialize() {
        return true;
    }

    void starter::application::shutdown() noexcept {
    }

    bool starter::application::on_should_close() {
        return true;
    }

    //
    // starter::parameters
    //

    starter::parameters::parameters(const engine::parameters& engine_params)
    : engine_params_(engine_params) {}

    starter::parameters& starter::parameters::library_root(const url& value) {
        library_root_ = value;
        return *this;
    }

    starter::parameters& starter::parameters::engine_params(const engine::parameters& value) {
        engine_params_ = value;
        return *this;
    }

    url& starter::parameters::library_root() noexcept {
        return library_root_;
    }

    engine::parameters& starter::parameters::engine_params() noexcept {
        return engine_params_;
    }

    const url& starter::parameters::library_root() const noexcept {
        return library_root_;
    }

    const engine::parameters& starter::parameters::engine_params() const noexcept {
        return engine_params_;
    }

    //
    // starter
    //

    starter::starter(int argc, char *argv[], const parameters& params) {
        safe_module_initialize<engine>(argc, argv, params.engine_params());
        safe_module_initialize<factory>()
            .register_component<actor>("actor")
            .register_component<camera>("camera")
            .register_component<flipbook_player>("flipbook_player")
            .register_component<label>("label")
            .register_component<label::dirty>("label.dirty")
            .register_component<model_renderer>("model_renderer")
            .register_component<renderer>("renderer")
            .register_component<scene>("scene")
            .register_component<spine_player>("spine_player")
            .register_component<spine_player_cmd>("spine_player_cmd")
            .register_component<spine_player_evt>("spine_player_evt")
            .register_component<sprite_renderer>("sprite_renderer")
            .register_component<sprite_9p_renderer>("sprite_9p_renderer")
            .register_component<ui_layout::root_tag>("ui_layout.root_tag")
            .register_component<ui_layout::shape2d_update_size_tag>("ui_layout.shape2d_update_size_tag")
            .register_component<fixed_layout>("fixed_layout")
            .register_component<fixed_layout::dirty>("fixed_layout.dirty")
            .register_component<auto_layout>("auto_layout")
            .register_component<auto_layout::dirty>("auto_layout.dirty")
            .register_component<stack_layout>("stack_layout")
            .register_component<stack_layout::dirty>("stack_layout.dirty")
            .register_component<dock_layout>("dock_layout")
            .register_component<dock_layout::dirty>("dock_layout.dirty")
            .register_component<image_layout>("image_layout")
            .register_component<image_layout::dirty>("image_layout.dirty")
            .register_component<margin_layout>("margin_layout")
            .register_component<margin_layout::dirty>("margin_layout.dirty")
            .register_component<padding_layout>("padding_layout")
            .register_component<padding_layout::dirty>("padding_layout.dirty")
            .register_component<anchor_layout>("anchor_layout")
            .register_component<anchor_layout::dirty>("anchor_layout.dirty")
            .register_component<label_layout>("label_layout")
            .register_component<label_layout::dirty>("label_layout.dirty")
            .register_component<label_autoscale_layout>("label_autoscale_layout")
            .register_component<label_autoscale_layout::dirty>("label_autoscale_layout.dirty")
            .register_component<ui_button>("ui_button")
            .register_component<ui_selectable>("ui_selectable")
            .register_component<ui_draggable>("ui_draggable")
            .register_component<ui_style>("ui_style")
            .register_component<ui_style::style_changed_tag>("ui_style.style_changed_tag")
            .register_component<ui_color_style_comp>("ui_color_style_comp")
            .register_component<ui_font_style_comp>("ui_font_style_comp")
            .register_component<ui_sprite_style_comp>("ui_sprite_style_comp")
            .register_component<ui_sprite_9p_style_comp>("ui_sprite_9p_style_comp")
            .register_component<rectangle_shape>("rectangle_shape")
            .register_component<circle_shape>("circle_shape")
            .register_component<polygon_shape>("polygon_shape")
            .register_component<touchable>("touchable");
        safe_module_initialize<library>(params.library_root(), the<deferrer>());
        safe_module_initialize<world>();
    }

    starter::~starter() noexcept {
        modules::shutdown<world>();
        modules::shutdown<library>();
        modules::shutdown<factory>();
        modules::shutdown<engine>();
    }

    bool starter::start(application_uptr app) {
        return the<engine>().start(
            std::make_unique<engine_application>(
                std::move(app)));
    }
}

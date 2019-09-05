/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/starter.hpp>

#include <enduro2d/high/world.hpp>
#include <enduro2d/high/factory.hpp>
#include <enduro2d/high/library.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/flipbook_player.hpp>
#include <enduro2d/high/components/flipbook_source.hpp>
#include <enduro2d/high/components/label.hpp>
#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/scene.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/shape2d.hpp>
#include <enduro2d/high/components/ui_layout.hpp>

#include <enduro2d/high/systems/flipbook_system.hpp>
#include <enduro2d/high/systems/label_system.hpp>
#include <enduro2d/high/systems/render_system.hpp>
#include <enduro2d/high/systems/screenspace_raycast_system.hpp>
#include <enduro2d/high/systems/shape_projection_system.hpp>
#include <enduro2d/high/systems/input_event_system.hpp>
#include <enduro2d/high/systems/ui_layout_system.hpp>

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
                .system<flipbook_system>(world::priority_update)
                .system<label_system>(world::priority_update)
                .system<render_system>(world::priority_render)
                .system<input_event_system_pre_update>(world::priority_pre_update)
                .system<shape_projection_system>(world::priority_pre_update + 1)
                .system<convex_hull_screenspace_raycast_system>(world::priority_pre_update + 2)
                .system<input_event_system_post_update>(world::priority_pre_update + 3)
                .system<ui_layout_system>(world::priority_update);
            return !application_ || application_->initialize();
        }

        void shutdown() noexcept final {
            if ( application_ ) {
                application_->shutdown();
            }
        }

        bool frame_tick() final {
            the<world>().registry().process_systems_in_range(
                world::priority_update_section_begin,
                world::priority_update_section_end);
            return !the<window>().should_close()
                || (application_ && !application_->on_should_close());
        }

        void frame_render() final {
            the<world>().registry().process_systems_in_range(
                world::priority_render_section_begin,
                world::priority_render_section_end);
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
            .register_component<flipbook_source>("flipbook_source")
            .register_component<label>("label")
            .register_component<label::dirty>("label.dirty")
            .register_component<model_renderer>("model_renderer")
            .register_component<renderer>("renderer")
            .register_component<scene>("scene")
            .register_component<sprite_renderer>("sprite_renderer")
            .register_component<ui_layout>("ui_layout")
            .register_component<fixed_layout>("fixed_layout")
            .register_component<auto_layout>("auto_layout")
            .register_component<stack_layout>("stack_layout")
            .register_component<dock_layout>("dock_layout");
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

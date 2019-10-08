/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_high.hpp"
using namespace e2d;

namespace
{
    const v2u screen_size = v2u(800, 600);
    const m4f proj_matrix = math::make_orthogonal_lh_matrix4(
        screen_size.cast_to<f32>(), 0.f, 1000.f);

    class safe_world_initializer final : private noncopyable {
    public:
        safe_world_initializer() {
            modules::initialize<world>();
            
            scene_i = the<world>().instantiate();
            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i))
                .component<ui_layout::root_tag>();
            scene_r = scene_i->get_component<actor>().get().node();
        
            camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera>(camera()
                    .viewport(b2u(screen_size))
                    .projection(proj_matrix))
                .component<actor>(node::create(camera_i));
        }

        ~safe_world_initializer() noexcept {
            scene_i = nullptr;
            scene_r = nullptr;
            camera_i = nullptr;

            modules::shutdown<world>();
        }
        
        gobject_iptr scene_i;
        node_iptr scene_r;
        
        gobject_iptr camera_i;
    };
    
    gobject_iptr create_fixed_layout(const node_iptr& root, const v2f& pos, const v2f& size) {
        gobject_iptr fl = the<world>().instantiate();
        fl->entity_filler()
            .component<actor>(node::create(fl, root))
            .component<fixed_layout>(size)
            .component<fixed_layout::dirty>();
        node_iptr fl_node = fl->get_component<actor>().get().node();
        fl_node->translation(v3f(pos.x, pos.y, 0.f));
        return fl;
    }
}

TEST_CASE("ui_animation") {
    safe_world_initializer initializer;

    SECTION("move animation") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const v3f from_pos = {1.0f, 2.0f, 0.0f};
        const v3f to_pos = {20.0f, 5.0f, 0.1f};
        const secf dur = secf(2.0f);
        const secf step {1.0f / 60.0f};
        fl->entity().assign_component<ui_animation>(ui_animation::move()
            .from(from_pos)
            .to(to_pos)
            .duration(dur));

        ui_layout_system lsystem;
        ui_animation_system asystem;

        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(111.72f);

        for ( u32 i = 0; i < 3*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i == 0 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                auto exp = math::lerp(from_pos, to_pos, v3f(step.value / dur.value));
                REQUIRE(math::approximately(cur, exp, 0.001f));
            }
            if ( i > 2*60 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                REQUIRE(math::approximately(cur, to_pos, 0.001f));
            }
        }
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("sequential animation") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const secf step {1.0f / 60.0f};
        u32 tick0 = 0;
        u32 tick1 = 0;
        bool is_complete = false;
        fl->entity().assign_component<ui_animation>(ui_animation::sequential()
            .add(ui_animation::custom([&tick0]() { ++tick0; }).duration(secf(1.0f)))
            .add(ui_animation::custom([&tick1]() { ++tick1; }).duration(secf(2.0f)))
            .on_complete([&is_complete, &tick0, &tick1](auto&) {
                is_complete = ((tick0 >= 60 && tick0 <= 61) && (tick1 >= 120 && tick1 <= 121));
            }));

        ui_layout_system lsystem;
        ui_animation_system asystem;
        
        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(333.93f);

        for ( u32 i = 0; i < 4*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i <= 60 ) {
                REQUIRE(tick0 == i+1);
                REQUIRE(tick1 == 0);
            } else if ( i <= 3*60 ) {
                bool b_tick0 = (tick0 >= 60 && tick0 <= 61);
                REQUIRE(b_tick0);
                REQUIRE(tick1 == i - 60);
            } else {
                bool b_tick0 = (tick0 >= 60 && tick0 <= 61);
                REQUIRE(b_tick0);
                bool b_tick1 = (tick1 >= 120 && tick1 <= 121);
                REQUIRE(b_tick1);
            }
        }
        REQUIRE(is_complete);
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("parallel animation") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const secf step {1.0f / 60.0f};
        u32 tick0 = 0;
        u32 tick1 = 0;
        bool is_complete = false;
        fl->entity().assign_component<ui_animation>(ui_animation::parallel()
            .add(ui_animation::custom([&tick0]() { ++tick0; }).duration(secf(1.0f)))
            .add(ui_animation::custom([&tick1]() { ++tick1; }).duration(secf(2.0f)))
            .on_complete([&is_complete, &tick0, &tick1](auto&) {
                is_complete = ((tick0 >= 60 && tick0 <= 61) && (tick1 >= 120 && tick1 <= 121));
            }));
        
        ui_layout_system lsystem;
        ui_animation_system asystem;
        
        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(44.61f);

        for ( u32 i = 0; i < 4*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i <= 60 ) {
                REQUIRE(tick0 == i+1);
                REQUIRE(tick1 == i+1);
            } else if ( i <= 2*60 ) {
                bool b_tick0 = (tick0 >= 60 && tick0 <= 61);
                REQUIRE(b_tick0);
                REQUIRE(tick1 == i+1);
            } else {
                bool b_tick0 = (tick0 >= 60 && tick0 <= 61);
                REQUIRE(b_tick0);
                bool b_tick1 = (tick1 >= 120 && tick1 <= 121);
                REQUIRE(b_tick1);
            }
        }
        REQUIRE(is_complete);
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("animation callbacks") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        u32 tick = 0;
        bool is_started = false;
        bool is_completed = false;
        const secf step {1.0f / 60.0f};
        fl->entity().assign_component<ui_animation>(
            ui_animation::custom([&tick]() { ++tick; })
                .duration(secf(2.0f))
                .on_start([&tick, &is_started](auto...) { is_started = (tick == 0); })
                .on_complete([&tick, &is_completed](auto...) { is_completed = (tick >= 120); }));

        ui_layout_system lsystem;
        ui_animation_system asystem;

        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(626.72f);

        for ( u32 i = 0; i < 3*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));
        }
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
        REQUIRE(is_started);
        REQUIRE(is_completed);
    }
    SECTION("exception") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        
        fl->entity().assign_component<ui_animation>(
            ui_animation::custom([](f32, sprite_renderer& spr) { spr.tint(color32::red()); }));

        ui_layout_system lsystem;
        ui_animation_system asystem;
        
        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = secf(1.0f / 60.0f);
        params.realtime_time = secf(45.8f);
        
        REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
        REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("loops") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const v3f from_pos = {1.0f, 2.0f, 0.0f};
        const v3f to_pos = {20.0f, 5.0f, 0.1f};
        const secf dur = secf(1.0f);
        const secf step {1.0f / 60.0f};
        fl->entity().assign_component<ui_animation>(ui_animation::move()
            .from(from_pos)
            .to(to_pos)
            .duration(dur)
            .repeat(1));

        ui_layout_system lsystem;
        ui_animation_system asystem;

        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(391.62f);

        for ( u32 i = 0; i < 3*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i == 0 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                auto exp = math::lerp(from_pos, to_pos, v3f(step.value / dur.value));
                REQUIRE(math::approximately(cur, exp, 0.001f));
            }
            if ( i == 60 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                REQUIRE(math::approximately(cur, to_pos, 0.001f));
            }
            if ( i == 61 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                auto exp = math::lerp(from_pos, to_pos, v3f(step.value / dur.value));
                REQUIRE(math::approximately(cur, exp, 0.001f));
            }
            if ( i == 121 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                REQUIRE(math::approximately(cur, to_pos, 0.001f));
            }
        }
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("inversed repeat") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const v3f from_pos = {1.0f, 2.0f, 0.0f};
        const v3f to_pos = {20.0f, 5.0f, 0.1f};
        const secf dur = secf(1.0f);
        const secf step {1.0f / 60.0f};
        fl->entity().assign_component<ui_animation>(ui_animation::move()
            .from(from_pos)
            .to(to_pos)
            .duration(dur)
            .repeat(1)
            .repeat_inversed());

        ui_layout_system lsystem;
        ui_animation_system asystem;

        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(391.62f);

        for ( u32 i = 0; i < 3*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i == 0 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                auto exp = math::lerp(from_pos, to_pos, v3f(step.value / dur.value));
                REQUIRE(math::approximately(cur, exp, 0.001f));
            }
            if ( i == 60 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                REQUIRE(math::approximately(cur, to_pos, 0.001f));
            }
            if ( i == 61 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                auto exp = math::lerp(from_pos, to_pos, v3f(1.0f - step.value / dur.value));
                REQUIRE(math::approximately(cur, exp, 0.001f));
            }
            if ( i == 121 ) {
                auto cur = fl->get_component<actor>().get().node()->translation();
                REQUIRE(math::approximately(cur, from_pos, 0.001f));
            }
        }
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("sequential animation loops") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        const secf step {1.0f / 60.0f};
        u32 tick0 = 0;
        u32 tick1 = 0;
        bool is_restarted = false;
        bool is_complete = false;
        fl->entity().assign_component<ui_animation>(ui_animation::sequential()
            .add(ui_animation::custom([&tick0]() { ++tick0; }).duration(secf(1.0f)))
            .add(ui_animation::custom([&tick1]() { ++tick1; }).duration(secf(2.0f)))
            .on_step_complete([&is_restarted, &tick0, &tick1](auto&) {
                is_restarted = ((tick0 >= 60 && tick0 <= 61) && (tick1 >= 120 && tick1 <= 121));
                tick0 = tick1 = 0;
            })
            .on_complete([&is_complete, &tick0, &tick1](auto&) {
                is_complete = ((tick0 >= 60 && tick0 <= 61) && (tick1 >= 120 && tick1 <= 121));
            })
            .repeat(1)
            .repeat_inversed());

        ui_layout_system lsystem;
        ui_animation_system asystem;
        
        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(333.93f);

        for ( u32 i = 0; i < 7*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));
        }
        REQUIRE(is_restarted);
        REQUIRE(is_complete);
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
    SECTION("delay") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        u32 tick = 0;
        const secf step {1.0f / 60.0f};
        fl->entity().assign_component<ui_animation>(ui_animation::custom([&tick]() { ++tick; })
            .delay(secf(1.0f))
            .duration(secf(2.0f)));

        ui_layout_system lsystem;
        ui_animation_system asystem;

        auto& params = the<world>().registry().ensure_single_component<frame_params_comp>();
        params.delta_time = step;
        params.realtime_time = secf(626.72f);

        for ( u32 i = 0; i < 4*60; ++i ) {
            params.realtime_time += step;

            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));

            if ( i < 60 ) {
                REQUIRE(tick == 0);
            }
        }
        REQUIRE(tick >= 60);
        REQUIRE_FALSE(fl->entity().find_component<ui_animation>());
    }
}

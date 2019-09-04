/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_high.hpp"
using namespace e2d;

namespace
{
    class safe_world_initializer final : private noncopyable {
    public:
        safe_world_initializer() {
            modules::initialize<world>();
            
            scene_i = the<world>().instantiate();
            scene_i->entity_filler()
                .component<scene>()
                .component<actor>(node::create(scene_i))
                .component<ui_layout>()
                .component<ui_layout::root_tag>();
            scene_r = scene_i->get_component<actor>().get().node();
        
            camera_i = the<world>().instantiate();
            camera_i->entity_filler()
                .component<camera>(camera()
                    .viewport(b2u(0, 0, 800, 600))
                    .projection(math::make_orthogonal_lh_matrix4(
                        v2f(800.0f, 600.0f), 0.f, 1000.f)))
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
}

TEST_CASE("ui_layout") {
    safe_world_initializer initializer;

    SECTION("fixed_layout") {
        gobject_iptr fl1 = the<world>().instantiate();
        fl1->entity_filler()
            .component<actor>(node::create(fl1, initializer.scene_r))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(20.0f, 30.0f, 100.0f, 200.0f))
            .component<fixed_layout::dirty_flag>();
        node_iptr fl1_node = fl1->get_component<actor>().get().node();
        
        gobject_iptr fl2 = the<world>().instantiate();
        fl2->entity_filler()
            .component<actor>(node::create(fl2, fl1_node))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(40.0f, 50.0f, 200.0f, 300.0f))
            .component<fixed_layout::dirty_flag>();

        ui_layout_system system;
        system.process(the<world>().registry());

        REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty_flag>().exists());

        auto* layout1 = fl1->get_component<ui_layout>().find();
        auto* layout2 = fl2->get_component<ui_layout>().find();
        REQUIRE(layout1 != nullptr);
        REQUIRE(layout2 != nullptr);
        REQUIRE(layout1->region() == b2f(20.0f, 30.0f, 100.0f, 200.0f));
        REQUIRE(layout2->region() == b2f(40.0f, 50.0f, 200.0f, 300.0f));
    }

    SECTION("auto_layout") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<ui_layout>()
            .component<auto_layout>()
            .component<auto_layout::dirty_flag>();
        node_iptr al_node = al->get_component<actor>().get().node();

        gobject_iptr fl1 = the<world>().instantiate();
        fl1->entity_filler()
            .component<actor>(node::create(fl1, al_node))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(50.0f, 50.0f) + v2f(-50.0f, 0.0f))
            .component<fixed_layout::dirty_flag>();
        
        gobject_iptr fl2 = the<world>().instantiate();
        fl2->entity_filler()
            .component<actor>(node::create(fl2, al_node))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(100.0f, 100.0f) + v2f(100.0f, 100.0f))
            .component<fixed_layout::dirty_flag>();
        
        gobject_iptr fl3 = the<world>().instantiate();
        fl3->entity_filler()
            .component<actor>(node::create(fl3, al_node))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(120.0f, 700.0f) + v2f(50.0f, 180.0f))
            .component<fixed_layout::dirty_flag>();
        
        ui_layout_system system;
        system.process(the<world>().registry());
        
        REQUIRE_FALSE(al->get_component<auto_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty_flag>().exists());
        
        auto* layout1 = fl1->get_component<ui_layout>().find();
        auto* layout2 = fl2->get_component<ui_layout>().find();
        auto* layout3 = fl3->get_component<ui_layout>().find();
        auto* layout4 = al->get_component<ui_layout>().find();
        REQUIRE(layout1 != nullptr);
        REQUIRE(layout2 != nullptr);
        REQUIRE(layout3 != nullptr);
        REQUIRE(layout4 != nullptr);
        REQUIRE(layout1->region() == b2f(50.0f, 50.0f) + v2f(-50.0f, 0.0f));
        REQUIRE(layout2->region() == b2f(100.0f, 100.0f) + v2f(100.0f, 100.0f));
        REQUIRE(layout3->region() == b2f(120.0f, 700.0f) + v2f(50.0f, 180.0f));
        REQUIRE(layout4->region() == b2f(-50.0f, 0.0f, 250.0f, 880.0f));
    }
}

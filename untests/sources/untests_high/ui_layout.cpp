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
        world& w = the<world>();

        gobject_iptr g1 = w.instantiate();
        g1->entity_filler()
            .component<actor>(node::create(g1, initializer.scene_r))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(20.0f, 30.0f, 100.0f, 200.0f))
            .component<fixed_layout::dirty_flag>();
        node_iptr g1_node = g1->get_component<actor>().get().node();
        
        gobject_iptr g2 = w.instantiate();
        g2->entity_filler()
            .component<actor>(node::create(g2, g1_node))
            .component<ui_layout>()
            .component<fixed_layout>(b2f(40.0f, 50.0f, 200.0f, 300.0f))
            .component<fixed_layout::dirty_flag>();
        node_iptr g2_node = g2->get_component<actor>().get().node();

        ui_layout_system system;
        system.process(w.registry());

        REQUIRE_FALSE(g1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(g2->get_component<fixed_layout::dirty_flag>().exists());

        auto* layout1 = g1->get_component<ui_layout>().find();
        auto* layout2 = g2->get_component<ui_layout>().find();
        REQUIRE(layout1 != nullptr);
        REQUIRE(layout2 != nullptr);
        REQUIRE(layout1->region() == b2f(20.0f, 30.0f, 100.0f, 200.0f));
        REQUIRE(layout2->region() == b2f(40.0f, 50.0f, 200.0f, 300.0f));
    }
}

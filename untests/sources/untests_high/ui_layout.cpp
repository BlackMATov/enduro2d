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
                .component<ui_layout>()
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

    b2f get_region(const gobject_iptr& go) {
        auto e = go->entity();
        auto& layout = e.get_component<ui_layout>();
        auto& act = e.get_component<actor>();
        const v2f size = layout.size();
        const m4f m = act.node()->local_matrix();
        const v4f points[] = {
            v4f(0.0f,   0.0f,   0.0f, 1.0f) * m,
            v4f(0.0f,   size.y, 0.0f, 1.0f) * m,
            v4f(size.x, size.y, 0.0f, 1.0f) * m,
            v4f(size.x, 0.0f,   0.0f, 1.0f) * m
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        b2f result(min, max - min);
        return result;
    }

    gobject_iptr create_fixed_layout(const node_iptr& root, const v2f& pos, const v2f& size) {
        gobject_iptr fl = the<world>().instantiate();
        fl->entity_filler()
            .component<actor>(node::create(fl, root))
            .component<ui_layout>()
            .component<fixed_layout>(size)
            .component<fixed_layout::dirty_flag>();
        node_iptr fl_node = fl->get_component<actor>().get().node();
        fl_node->translation(v3f(pos.x, pos.y, 0.f));
        return fl;
    }
    /*
    b2f project_to_parent(const node_iptr& n, const b2f& r) noexcept {
        const m4f m = n->local_matrix();
        const v4f points[] = {
            v4f(r.position.x,            r.position.y,            0.0f, 1.0f) * m,
            v4f(r.position.x,            r.position.y + r.size.y, 0.0f, 1.0f) * m,
            v4f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f, 1.0f) * m,
            v4f(r.position.x + r.size.x, r.position.y,            0.0f, 1.0f) * m
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        return b2f(min, max - min);
    }

    v2f project_to_local(const node_iptr& n, const b2f& r) noexcept {
        // TODO:
        // - rotation is not supported
        const auto inv_opt = math::inversed(n->local_matrix());
        const m4f inv = inv_opt.second
            ? inv_opt.first
            : m4f::identity();
        const v4f points[] = {
            v4f(r.position.x,            r.position.y,            0.0f, 1.0f) * inv,
            v4f(r.position.x,            r.position.y + r.size.y, 0.0f, 1.0f) * inv,
            v4f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f, 1.0f) * inv,
            v4f(r.position.x + r.size.x, r.position.y,            0.0f, 1.0f) * inv
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        n->translation(n->translation() + v3f(min.x, min.y, 0.0f));
        return max - min;
    }*/
}

TEST_CASE("ui_layout") {
    safe_world_initializer initializer;
    
    /*SECTION("projection") {
        auto n0 = node::create();
        auto n1 = node::create();
        n1->translation(v3f(20.0f, 30.0f, 0.0f));

        b2f b1(100.0f, 200.0f);
        b2f b2 = project_to_parent(n1, b1);

        b2.position += v2f(10.0f);
        v2f b3 = project_to_local(n1, b2);

        REQUIRE(n1->translation() == v3f(30.0f, 40.0f, 0.0f));
    }*/

    SECTION("fixed_layout") {
        gobject_iptr fl1 = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        node_iptr fl1_node = fl1->get_component<actor>().get().node();
        
        gobject_iptr fl2 = create_fixed_layout(initializer.scene_r, {40.0f, 50.0f}, {200.0f, 300.0f});

        ui_layout_system system;
        system.process(the<world>().registry());

        REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty_flag>().exists());

        REQUIRE(get_region(fl1) == b2f(20.0f, 30.0f, 100.0f, 200.0f));
        REQUIRE(get_region(fl2) == b2f(40.0f, 50.0f, 200.0f, 300.0f));
    }
    /*
    SECTION("auto_layout") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<ui_layout>()
            .component<auto_layout>()
            .component<auto_layout::dirty_flag>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(al_node, {-50.0f, 0.0f}, {50.0f, 50.0f});
        gobject_iptr fl2 = create_fixed_layout(al_node, {100.0f, 100.0f}, {100.0f, 100.0f});
        gobject_iptr fl3 = create_fixed_layout(al_node, {50.0f, 180.0f}, {120.0f, 700.0f});
        
        ui_layout_system system;
        system.process(the<world>().registry());
        
        REQUIRE_FALSE(al->get_component<auto_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty_flag>().exists());
        
        REQUIRE(get_region(fl1) == b2f(50.0f, 50.0f) + v2f(-50.0f, 0.0f));
        REQUIRE(get_region(fl2) == b2f(100.0f, 100.0f) + v2f(100.0f, 100.0f));
        REQUIRE(get_region(fl3) == b2f(120.0f, 700.0f) + v2f(50.0f, 180.0f));
        REQUIRE(get_region(al) == b2f(-50.0f, 0.0f, 250.0f, 880.0f));
    }
    /*/
    SECTION("stack_layout") {
        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, initializer.scene_r))
            .component<ui_layout>()
            .component<stack_layout>(stack_layout::stack_origin::bottom)
            .component<stack_layout::dirty_flag>();
        node_iptr sl_node = sl->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(sl_node, {}, {100.0f, 200.0f});
        gobject_iptr fl2 = create_fixed_layout(sl_node, {}, {300.0f, 50.0f});
        gobject_iptr fl3 = create_fixed_layout(sl_node, {}, {500.0f, 77.0f});
        gobject_iptr fl4 = create_fixed_layout(sl_node, {}, {400.0f, 150.0f});
        gobject_iptr fl5 = create_fixed_layout(sl_node, {}, {600.0f, 280.0f});
        
        ui_layout_system system;
        system.process(the<world>().registry());
        
        REQUIRE_FALSE(sl->get_component<stack_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl4->get_component<fixed_layout::dirty_flag>().exists());
        REQUIRE_FALSE(fl5->get_component<fixed_layout::dirty_flag>().exists());
        
        REQUIRE(get_region(fl1) == b2f(100.0f, 200.0f) + v2f(0.0f, 0.0f));
        REQUIRE(get_region(fl2) == b2f(300.0f, 50.0f) + v2f(0.0f, 200.0f));
        REQUIRE(get_region(fl3) == b2f(500.0f, 77.0f) + v2f(0.0f, 250.0f));
        REQUIRE(get_region(fl4) == b2f(400.0f, 150.0f) + v2f(0.0f, 327.0f));
        REQUIRE(get_region(fl5) == b2f(600.0f, 280.0f) + v2f(0.0f, 477.0f));
        REQUIRE(get_region(sl) == b2f(600.0f, 757.0f));
    }
}

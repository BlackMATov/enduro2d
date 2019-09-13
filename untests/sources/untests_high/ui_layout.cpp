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

    b2f get_region(const gobject_iptr& go) {
        auto e = go->entity();
        auto& act = e.get_component<actor>();
        const m4f m = act.node()->local_matrix();
        const v2f size = act.node()->size();
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

    gobject_iptr create_fixed_layout(const node_iptr& root, const v2f& pos, const v2f& size, const v2f& scale) {
        gobject_iptr fl = the<world>().instantiate();
        fl->entity_filler()
            .component<actor>(node::create(fl, root))
            .component<fixed_layout>(size)
            .component<fixed_layout::dirty>();
        node_iptr fl_node = fl->get_component<actor>().get().node();
        fl_node->translation(v3f(pos.x, pos.y, 0.f));
        fl_node->scale(v3f(scale, 1.0f));
        return fl;
    }

    gobject_iptr create_fixed_layout(const node_iptr& root, const v2f& pos, const v2f& size) {
        return create_fixed_layout(root, pos, size, v2f(1.0f));
    }
    
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

    b2f project_to_local(const node_iptr& n, const b2f& r) noexcept {
        const auto inv_opt = math::inversed(n->local_matrix(), 0.0f);
        const m4f inv = inv_opt.second
            ? inv_opt.first
            : m4f::identity();
        const v4f points[] = {
            v4f(r.position.x,            r.position.y,            0.0f, 1.0f) * inv,
            v4f(r.position.x,            r.position.y + r.size.y, 0.0f, 1.0f) * inv,
            v4f(r.position.x + r.size.x, r.position.y,            0.0f, 1.0f) * inv
        };
        return b2f({
            math::length(points[2] - points[0]),
            math::length(points[1] - points[0])});
    }

    b2f project_to_parent_opt(const node_iptr& n, const b2f& r) noexcept {
        const v3f off = v3f(n->size() * n->pivot(), 0.0f);
        const auto tr = [&n, off](const v2f& p) {
            return v2f(
                ((v3f(p, 0.0f) - off) * n->transform().scale)
                * n->transform().rotation
                + off);
        };
        const v2f points[] = {
            tr(v2f(r.position.x,            r.position.y           )),
            tr(v2f(r.position.x,            r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y           ))
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

    b2f project_to_local_opt(const node_iptr& n, const b2f& r) noexcept {
        const v3f off = v3f(n->size() * n->pivot(), 0.0f);
        const auto tr = [&n, off](const v2f& p) {
            return v2f(
                ((v3f(p, 0.0f) - off) * math::inversed(n->transform().rotation))
                / n->transform().scale
                + off);
        };
        const v2f points[] = {
            tr(v2f(r.position.x,            r.position.y           )),
            tr(v2f(r.position.x,            r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y           ))
        };
        return b2f({
            math::length(points[2] - points[0]),
            math::length(points[1] - points[0])});
    }
}

TEST_CASE("ui_layout") {
    safe_world_initializer initializer;
    
    SECTION("projection") {
        node_iptr n = node::create();

        n->size({10.0f, 10.0f});
        n->pivot({0.2f, 0.8f});
        n->rotation(math::make_quat_from_axis_angle(make_deg(45.0f), v3f::unit_z()));
        
        auto r0 = project_to_local(n, b2f(4.0f, 5.0f, 10.0f, 20.0f));
        auto r1 = project_to_local_opt(n, b2f(4.0f, 5.0f, 10.0f, 20.0f));
        REQUIRE(math::approximately(r0, r1, 0.01f));

        n->scale({1.2f, 2.3f, 1.0f});
        
        auto r2 = project_to_local(n, b2f(4.0f, 5.0f, 10.0f, 20.0f));
        auto r3 = project_to_local_opt(n, b2f(4.0f, 5.0f, 10.0f, 20.0f));
        REQUIRE(math::approximately(r2, r3, 0.01f));
    }
    SECTION("fixed_layout") {
        gobject_iptr fl1 = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        gobject_iptr fl2 = create_fixed_layout(initializer.scene_r, {40.0f, 50.0f}, {200.0f, 300.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());

            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());

            REQUIRE(get_region(fl1) == b2f(20.0f, 30.0f, 100.0f, 200.0f));
            REQUIRE(get_region(fl2) == b2f(40.0f, 50.0f, 200.0f, 300.0f));
        }
    }
    SECTION("fixed_layout-2") {
        gobject_iptr fl1 = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f}, {1.5f, 2.1f});
        gobject_iptr fl2 = create_fixed_layout(initializer.scene_r, {40.0f, 50.0f}, {200.0f, 300.0f}, {1.8f, 3.4f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());

            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());

            REQUIRE(get_region(fl1) == b2f(20.0f, 30.0f, 100.0f * 1.5f, 200.0f * 2.1f));
            REQUIRE(get_region(fl2) == b2f(40.0f, 50.0f, 200.0f * 1.8f, 300.0f * 3.4f));
        }
    }
    SECTION("stack_layout - bottom") {
        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, initializer.scene_r))
            .component<stack_layout>(stack_layout::stack_origin::bottom)
            .component<stack_layout::dirty>();
        node_iptr sl_node = sl->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(sl_node, {}, {100.0f, 200.0f});
        gobject_iptr fl2 = create_fixed_layout(sl_node, {}, {300.0f, 50.0f});
        gobject_iptr fl3 = create_fixed_layout(sl_node, {}, {500.0f, 77.0f});
        gobject_iptr fl4 = create_fixed_layout(sl_node, {}, {400.0f, 150.0f});
        gobject_iptr fl5 = create_fixed_layout(sl_node, {}, {600.0f, 280.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(sl->get_component<stack_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl4->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl5->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl1) == b2f(100.0f, 200.0f) + v2f(0.0f, 0.0f));
            REQUIRE(get_region(fl2) == b2f(300.0f, 50.0f) + v2f(0.0f, 200.0f));
            REQUIRE(get_region(fl3) == b2f(500.0f, 77.0f) + v2f(0.0f, 250.0f));
            REQUIRE(get_region(fl4) == b2f(400.0f, 150.0f) + v2f(0.0f, 327.0f));
            REQUIRE(get_region(fl5) == b2f(600.0f, 280.0f) + v2f(0.0f, 477.0f));
            REQUIRE(get_region(sl) == b2f(600.0f, 757.0f));
        }
    }
    SECTION("stack_layout - left") {
        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, initializer.scene_r))
            .component<stack_layout>(stack_layout::stack_origin::left)
            .component<stack_layout::dirty>();
        node_iptr sl_node = sl->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(sl_node, {}, {55.0f, 100.0f});
        gobject_iptr fl2 = create_fixed_layout(sl_node, {}, {60.0f, 100.0f}, {2.0f, 2.0f});
        gobject_iptr fl3 = create_fixed_layout(sl_node, {}, {33.0f, 150.0f});
        gobject_iptr fl4 = create_fixed_layout(sl_node, {}, {90.0f, 170.0f});
        gobject_iptr fl5 = create_fixed_layout(sl_node, {}, {111.0f, 90.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(sl->get_component<stack_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl4->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl5->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl1) == b2f(55.0f, 100.0f) + v2f(0.0f, 0.0f));
            REQUIRE(get_region(fl2) == b2f(120.0f, 200.0f) + v2f(55.0f, 0.0f));
            REQUIRE(get_region(fl3) == b2f(33.0f, 150.0f) + v2f(175.0f, 0.0f));
            REQUIRE(get_region(fl4) == b2f(90.0f, 170.0f) + v2f(208.0f, 0.0f));
            REQUIRE(get_region(fl5) == b2f(111.0f, 90.0f) + v2f(298.0f, 0.0f));
            REQUIRE(get_region(sl) == b2f(409.0f, 200.0f));
        }
    }
    SECTION("stack_layout - right") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 400.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, root_node))
            .component<stack_layout>(stack_layout::stack_origin::right)
            .component<stack_layout::dirty>();
        node_iptr sl_node = sl->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(sl_node, {}, {55.0f, 100.0f});
        gobject_iptr fl2 = create_fixed_layout(sl_node, {}, {120.0f, 200.0f});
        gobject_iptr fl3 = create_fixed_layout(sl_node, {}, {33.0f, 150.0f});
        gobject_iptr fl4 = create_fixed_layout(sl_node, {}, {90.0f, 170.0f});
        gobject_iptr fl5 = create_fixed_layout(sl_node, {}, {111.0f, 90.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(sl->get_component<stack_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl4->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl5->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl1) == b2f(55.0f, 100.0f) + v2f(354.0f, 0.0f));
            REQUIRE(get_region(fl2) == b2f(120.0f, 200.0f) + v2f(234.0f, 0.0f));
            REQUIRE(get_region(fl3) == b2f(33.0f, 150.0f) + v2f(201.0f, 0.0f));
            REQUIRE(get_region(fl4) == b2f(90.0f, 170.0f) + v2f(111.0f, 0.0f));
            REQUIRE(get_region(fl5) == b2f(111.0f, 90.0f) + v2f(0.0f, 0.0f));
            REQUIRE(get_region(sl) == b2f(409.0f, 200.0f));
        }
    }
    SECTION("stack_layout - item rotation") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 400.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, root_node))
            .component<stack_layout>(stack_layout::stack_origin::left)
            .component<stack_layout::dirty>();
        node_iptr sl_node = sl->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(sl_node, {}, {100.0f, 100.0f});
        gobject_iptr fl2 = create_fixed_layout(sl_node, {}, {100.0f, 100.0f});
        gobject_iptr fl3 = create_fixed_layout(sl_node, {}, {100.0f, 100.0f});
        node_iptr fl1_node = fl1->get_component<actor>().get().node();
        node_iptr fl2_node = fl2->get_component<actor>().get().node();
        node_iptr fl3_node = fl3->get_component<actor>().get().node();
        fl1_node->scale({1.2f, 1.0f, 1.0f});
        fl2_node->rotation(math::make_quat_from_axis_angle(make_deg(75.0f), v3f::unit_z()));
        fl2_node->scale({1.0f, 2.0f, 1.0f});
        fl3_node->scale({0.8f, 1.4f, 1.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());

            REQUIRE_FALSE(sl->get_component<stack_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
            
            REQUIRE(math::approximately(get_region(fl1), b2f(0.0f, 0.0f, 120.0f, 100.0f), 0.1f));
            REQUIRE(math::approximately(get_region(fl2), b2f(120.0f, 0.0f, 219.0f, 148.3f), 0.1f));
            REQUIRE(math::approximately(get_region(fl3), b2f(339.0f, 0.0f, 80.0f, 140.0f), 0.1f));
            REQUIRE(math::approximately(get_region(sl), b2f(419.0f, 148.3f), 0.1f));
        }
    }
    SECTION("auto_layout") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(al_node, {-50.0f, 10.0f}, {50.0f, 50.0f});
        gobject_iptr fl2 = create_fixed_layout(al_node, {100.0f, 100.0f}, {100.0f, 100.0f});
        gobject_iptr fl3 = create_fixed_layout(al_node, {50.0f, 180.0f}, {120.0f, 700.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(0.0f, 0.0f, 250.0f, 870.0f));
            REQUIRE(get_region(fl1) == b2f(50.0f, 50.0f) + v2f(0.0f, 0.0f));
            REQUIRE(get_region(fl2) == b2f(100.0f, 100.0f) + v2f(150.0f, 90.0f));
            REQUIRE(get_region(fl3) == b2f(120.0f, 700.0f) + v2f(100.0f, 170.0f));
        }
    }
    SECTION("auto_layout - scale") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(al_node, {-50.0f, 10.0f}, {50.0f, 50.0f});
        gobject_iptr fl2 = create_fixed_layout(al_node, {100.0f, 100.0f}, {100.0f, 100.0f}, {1.5f, 2.0f});
        gobject_iptr fl3 = create_fixed_layout(al_node, {50.0f, 180.0f}, {120.0f, 500.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(0.0f, 0.0f, 300.0f, 670.0f));
            REQUIRE(get_region(fl1) == b2f(50.0f, 50.0f) + v2f(0.0f, 0.0f));
            REQUIRE(get_region(fl2) == b2f(150.0f, 200.0f) + v2f(150.0f, 90.0f));
            REQUIRE(get_region(fl3) == b2f(120.0f, 500.0f) + v2f(100.0f, 170.0f));
        }
    }
    SECTION("auto_layout in stack layout") {
        gobject_iptr sl = the<world>().instantiate();
        sl->entity_filler()
            .component<actor>(node::create(sl, initializer.scene_r))
            .component<stack_layout>(stack_layout::stack_origin::left)
            .component<stack_layout::dirty>();
        node_iptr sl_node = sl->get_component<actor>().get().node();

        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, sl_node))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();

        gobject_iptr fl1 = create_fixed_layout(al_node, {-50.0f, 10.0f}, {50.0f, 50.0f});
        gobject_iptr fl2 = create_fixed_layout(al_node, {100.0f, 100.0f}, {100.0f, 100.0f});
        gobject_iptr fl3 = create_fixed_layout(al_node, {50.0f, 180.0f}, {120.0f, 220.0f});
        
        gobject_iptr fl4 = create_fixed_layout(sl_node, {}, {70.0f, 80.0f});
        gobject_iptr fl5 = create_fixed_layout(sl_node, {}, {110.0f, 30.0f});
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(sl->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl3->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl4->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl5->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(250.0f, 390.0f));
            REQUIRE(get_region(fl4) == b2f(70.0f, 80.0f) + v2f(250.0f, 0.0f));
            REQUIRE(get_region(fl5) == b2f(110.0f, 30.0f) + v2f(320.0f, 0.0f));
            REQUIRE(get_region(sl) == b2f(430.0f, 390.0f));
        }
    }
    SECTION("auto_layout - min size") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>(auto_layout()
                .min_size({10.0f, 20.0f}))
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(10.0f, 20.0f));
        }
    }
    SECTION("auto_layout - item scale+rotation") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl1 = create_fixed_layout(al_node, {-50.0f, 10.0f}, {50.0f, 50.0f});
        gobject_iptr fl2 = create_fixed_layout(al_node, {100.0f, 100.0f}, {100.0f, 100.0f});
        node_iptr fl1_node = fl1->get_component<actor>().get().node();
        node_iptr fl2_node = fl2->get_component<actor>().get().node();
        fl1_node->scale({1.2f, 2.1f, 1.0f});
        fl2_node->scale({1.0f, 1.5f, 1.0f});
        fl2_node->rotation(math::make_quat_from_axis_angle(make_deg(45.0f), v3f::unit_z()));

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(fl1->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(fl2->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(math::approximately(get_region(fl1), b2f(0.0f, 0.0f, 60.0f, 105.0f), 0.1f));
            REQUIRE(math::approximately(get_region(fl2), b2f(43.9f, 90.0f, 176.7f, 176.7f), 0.1f));
            REQUIRE(math::approximately(get_region(al), b2f(0.0f, 0.0f, 220.7f, 266.7f), 0.1f));
        }
    }
    SECTION("dock_layout - fill") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 600.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, root_node))
            .component<dock_layout>(dock_layout::dock_type::fill)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(dl_node, {}, {100.0f, 100.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(100.0f, 100.0f));
            REQUIRE(get_region(dl) == b2f(600.0f, 600.0f));
        }
    }
    SECTION("dock_layout - left-top") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 600.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, root_node))
            .component<dock_layout>(dock_layout::dock_type::left | dock_layout::dock_type::top)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(dl_node, {}, {100.0f, 100.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(100.0f, 100.0f));
            REQUIRE(get_region(dl) == b2f(100.0f, 100.0f) + v2f(0.0f, 500.0f));
        }
    }
    SECTION("dock_layout - right-bottom") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 600.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, root_node))
            .component<dock_layout>(dock_layout::dock_type::right | dock_layout::dock_type::bottom)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(dl_node, {}, {100.0f, 100.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(100.0f, 100.0f));
            REQUIRE(get_region(dl) == b2f(100.0f, 100.0f) + v2f(500.0f, 0.0f));
        }
    }
    SECTION("dock_layout - fill-x center-y") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 600.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, root_node))
            .component<dock_layout>(dock_layout::dock_type::left | dock_layout::dock_type::right | dock_layout::dock_type::center_y)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(dl_node, {}, {100.0f, 100.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(100.0f, 100.0f));
            REQUIRE(get_region(dl) == b2f(600.0f, 100.0f) + v2f(0.0f, 250.0f));
        }
    }
    SECTION("dock_layout fill auto_layout") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(al_node, {}, {300.0f, 300.0f});

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, al_node))
            .component<dock_layout>(dock_layout::dock_type::fill)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(300.0f, 300.0f));
            REQUIRE(get_region(dl) == b2f(300.0f, 300.0f));
            REQUIRE(get_region(fl) == b2f(300.0f, 300.0f));
        }
    }
    SECTION("dock_layout - child offset") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 600.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        gobject_iptr dl = the<world>().instantiate();
        dl->entity_filler()
            .component<actor>(node::create(dl, root_node))
            .component<dock_layout>(dock_layout::dock_type::left | dock_layout::dock_type::top)
            .component<dock_layout::dirty>();
        node_iptr dl_node = dl->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(dl_node, {60.0f, -20.0f}, {100.0f, 100.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(dl->get_component<dock_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(60.0f, -20.0f, 100.0f, 100.0f));
            REQUIRE(get_region(dl) == b2f(100.0f, 100.0f) + v2f(0.0f, 500.0f));
        }
    }
    SECTION("image_layout") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 500.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        sprite_asset::ptr sprite_res = sprite_asset::create(sprite()
            .set_texrect(b2f(24.0f, 24.0f)));
        
        gobject_iptr il = the<world>().instantiate();
        il->entity_filler()
            .component<actor>(node::create(il, root_node))
            .component<image_layout>(image_layout()
                .preserve_aspect(false))
            .component<image_layout::dirty>()
            .component<sprite_renderer>(sprite_res);
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(il->get_component<image_layout::dirty>().exists());

            REQUIRE(get_region(il) == b2f(600.0f, 500.0f));
        }
    }
    SECTION("image_layout - pivot") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 500.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        sprite_asset::ptr sprite_res = sprite_asset::create(sprite()
            .set_texrect(b2f(24.0f, 24.0f)));
        
        gobject_iptr il = the<world>().instantiate();
        il->entity_filler()
            .component<actor>(node::create(il, root_node))
            .component<image_layout>(image_layout()
                .preserve_aspect(false))
            .component<image_layout::dirty>()
            .component<sprite_renderer>(sprite_res);
        node_iptr il_node = il->get_component<actor>().get().node();
        il_node->rotation(math::make_quat_from_axis_angle(make_deg(50.0f), v3f::unit_z()));
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(il->get_component<image_layout::dirty>().exists());

            REQUIRE(math::approximately(get_region(il), b2f(54.4f, 0.0f, 545.6f, 554.4f), 0.1f));
        }
    }
    SECTION("image_layout - preserve_aspect") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 500.0f});
        node_iptr root_node = root->get_component<actor>().get().node();

        sprite_asset::ptr sprite_res = sprite_asset::create(sprite()
            .set_texrect(b2f(24.0f, 24.0f)));
        
        gobject_iptr il = the<world>().instantiate();
        il->entity_filler()
            .component<actor>(node::create(il, root_node))
            .component<image_layout>(image_layout()
                .preserve_aspect(true))
            .component<image_layout::dirty>()
            .component<sprite_renderer>(sprite_res);
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(il->get_component<image_layout::dirty>().exists());

            REQUIRE(get_region(il) == b2f(500.0f, 500.0f));
        }
    }
    SECTION("auto_layout + fixed_layout + image_layout") {
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, initializer.scene_r))
            .component<auto_layout>()
            .component<auto_layout::dirty>();
        node_iptr al_node = al->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(al_node, {}, {200.0f, 300.0f});
        
        sprite_asset::ptr sprite_res = sprite_asset::create(sprite()
            .set_texrect(b2f(24.0f, 24.0f)));
        
        gobject_iptr il = the<world>().instantiate();
        il->entity_filler()
            .component<actor>(node::create(il, al_node))
            .component<image_layout>(image_layout()
                .preserve_aspect(true))
            .component<image_layout::dirty>()
            .component<sprite_renderer>(sprite_res);
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<auto_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(il->get_component<image_layout::dirty>().exists());
        
            REQUIRE(get_region(fl) == b2f(200.0f, 300.0f));
            REQUIRE(get_region(al) == b2f(200.0f, 300.0f));
            REQUIRE(get_region(il) == b2f(200.0f, 200.0f));
        }
    }
    SECTION("padding_layout") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {200.0f, 300.0f});
        node_iptr root_node = root->get_component<actor>().get().node();
        
        gobject_iptr pl = the<world>().instantiate();
        pl->entity_filler()
            .component<actor>(node::create(pl, root_node))
            .component<padding_layout>(1.0f, 2.0f, 3.0f, 4.0f)
            .component<padding_layout::dirty>();
        
        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(root->get_component<fixed_layout::dirty>().exists());
            REQUIRE_FALSE(pl->get_component<padding_layout::dirty>().exists());
        
            REQUIRE(get_region(root) == b2f(200.0f, 300.0f));
            REQUIRE(get_region(pl) == b2f(1.0f, 2.0f, 196.0f, 294.0f));
        }
    }
    SECTION("margin_layout") {
        gobject_iptr ml = the<world>().instantiate();
        ml->entity_filler()
            .component<actor>(node::create(ml, initializer.scene_r))
            .component<margin_layout>(1.0f, 2.0f, 3.0f, 4.0f)
            .component<margin_layout::dirty>();
        node_iptr ml_node = ml->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(ml_node, {}, {200.0f, 300.0f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(ml->get_component<margin_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(get_region(ml) == b2f(204.0f, 306.0f));
            REQUIRE(get_region(fl) == b2f(1.0f, 2.0f, 200.0f, 300.0f));
        }
    }
    SECTION("margin_layout - scale+rotation") {
        gobject_iptr ml = the<world>().instantiate();
        ml->entity_filler()
            .component<actor>(node::create(ml, initializer.scene_r))
            .component<margin_layout>(1.0f, 2.0f, 3.0f, 4.0f)
            .component<margin_layout::dirty>();
        node_iptr ml_node = ml->get_component<actor>().get().node();
        
        gobject_iptr fl = create_fixed_layout(ml_node, {}, {200.0f, 300.0f});
        node_iptr fl_node = fl->get_component<actor>().get().node();
        fl_node->scale(v3f(0.75f));
        fl_node->rotation(math::make_quat_from_axis_angle(make_deg(30.0f), v3f::unit_z()));
        fl_node->pivot({0.2f, 0.8f});

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(ml->get_component<margin_layout::dirty>().exists());
            REQUIRE_FALSE(fl->get_component<fixed_layout::dirty>().exists());
        
            REQUIRE(math::approximately(get_region(ml), b2f(246.4f, 275.8f), 0.1f));
            REQUIRE(math::approximately(get_region(fl), b2f(1.0f, 2.0f, 242.4f, 269.8f), 0.1f));
        }
    }
    SECTION("anchor_layout") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 400.0f});
        node_iptr root_node = root->get_component<actor>().get().node();
        
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, root_node))
            .component<anchor_layout>(anchor_layout()
                .left_bottom({v2f(0.5f, 0.5f), v2f(-0.1f, -0.2f), true})
                .right_top({v2f(0.5f, 0.5f), v2f(50.0f, -50.0f), false}))
            .component<anchor_layout::dirty>();

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<anchor_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(240.0f, 120.0f, 110.0f, 30.0f));
        }
    }
    SECTION("anchor_layout - 2") {
        gobject_iptr root = create_fixed_layout(initializer.scene_r, {}, {600.0f, 400.0f});
        node_iptr root_node = root->get_component<actor>().get().node();
        
        gobject_iptr al = the<world>().instantiate();
        al->entity_filler()
            .component<actor>(node::create(al, root_node))
            .component<anchor_layout>(anchor_layout()
                .left_bottom({v2f(0.5f, 0.5f), v2f(-0.1f, -0.1f), true})
                .right_top({v2f(0.5f, 0.5f), v2f(50.0f, -80.0f), false}))
            .component<anchor_layout::dirty>();

        ui_layout_system system;
        for ( u32 i = 0; i < 9; ++i ) {
            system.process(the<world>().registry());
        
            REQUIRE_FALSE(al->get_component<anchor_layout::dirty>().exists());
        
            REQUIRE(get_region(al) == b2f(240.0f, 160.0f, 110.0f, 0.0f));
        }
    }
}

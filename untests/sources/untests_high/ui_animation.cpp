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
    /*SECTION("custom") {
        ui_animation::parallel()
            .add(ui_animation::custom([](f32 f) {}))
            .add(ui_animation::custom([](f32 f, actor& act) { act.node()->scale(v3f(math::lerp(1.0f, 2.0f, f))); }));
    }*/
    
    safe_world_initializer initializer;

    SECTION("animation") {
        gobject_iptr fl = create_fixed_layout(initializer.scene_r, {20.0f, 30.0f}, {100.0f, 200.0f});
        
        ui_layout_system lsystem;
        ui_animation_system asystem;
        for ( u32 i = 0;; ++i ) {
            REQUIRE_NOTHROW(lsystem.process(the<world>().registry()));
            REQUIRE_NOTHROW(asystem.process(the<world>().registry()));
        }
    }

    // test any animation
    // test sequential
    // test parallel
    // test loops (with and without inversion)
    // test easing
    // test on start callback
    // test exception
    // multi component animator ?
}

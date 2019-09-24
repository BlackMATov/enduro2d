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
                .component<actor>(node::create(camera_i))
                .component<camera::input_handler_tag>();
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

TEST_CASE("screenspace_collider") {
    safe_world_initializer initializer;
    
    SECTION("rectangle_shape touch-1") {
        gobject_iptr rshape = the<world>().instantiate();
        rshape->entity_filler()
            .component<actor>(node::create(rshape, initializer.scene_r))
            .component<rectangle_shape>(b2f(10.0f, 10.0f))
            .component<touchable>(true);
        node_iptr rshape_node = rshape->get_component<actor>().get().node();
        rshape_node->translation(v3f(25.0f, 45.0f, 0.0f));

        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(425.0f, 345.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));

        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());

        REQUIRE(rshape->get_component<touched_tag>().find());
        REQUIRE(rshape->get_component<touch_down_event>().find());
        REQUIRE(rshape->get_component<touch_focus_tag>().find());
        REQUIRE(rshape->get_component<touchable::capture>().find() == nullptr);
    }
    SECTION("rectangle_shape touch-2") {
        gobject_iptr rshape = the<world>().instantiate();
        rshape->entity_filler()
            .component<actor>(node::create(rshape, initializer.scene_r))
            .component<rectangle_shape>(b2f(10.0f, 10.0f))
            .component<touchable>(true);
        node_iptr rshape_node = rshape->get_component<actor>().get().node();
        rshape_node->translation(v3f(25.0f, 45.0f, 0.0f));

        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(435.0f, 355.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());

        REQUIRE(rshape->get_component<touched_tag>().find());
        REQUIRE(rshape->get_component<touch_down_event>().find());
        REQUIRE(rshape->get_component<touch_focus_tag>().find());
        REQUIRE(rshape->get_component<touchable::capture>().find() == nullptr);
    }
    SECTION("rectangle_shape touch outside") {
        gobject_iptr rshape = the<world>().instantiate();
        rshape->entity_filler()
            .component<actor>(node::create(rshape, initializer.scene_r))
            .component<rectangle_shape>(b2f(10.0f, 10.0f))
            .component<touchable>(true);
        node_iptr rshape_node = rshape->get_component<actor>().get().node();
        rshape_node->translation(v3f(25.0f, 45.0f, 0.0f));

        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(424.0f, 360.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());

        REQUIRE(rshape->get_component<touched_tag>().find() == nullptr);
        REQUIRE(rshape->get_component<touch_down_event>().find() == nullptr);
        REQUIRE(rshape->get_component<touch_focus_tag>().find() == nullptr);
        REQUIRE(rshape->get_component<touchable::capture>().find() == nullptr);
    }

    SECTION("circle_shape touch-1") {
        gobject_iptr cshape = the<world>().instantiate();
        cshape->entity_filler()
            .component<actor>(node::create(cshape, initializer.scene_r))
            .component<circle_shape>(v2f(), 5.0f)
            .component<touchable>(true);
        node_iptr cshape_node = cshape->get_component<actor>().get().node();
        cshape_node->translation(v3f(55.0f, 45.0f, 0.0f));
        
        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(460.0f, 350.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());

        REQUIRE(cshape->get_component<touched_tag>().find());
        REQUIRE(cshape->get_component<touch_down_event>().find());
        REQUIRE(cshape->get_component<touch_focus_tag>().find());
        REQUIRE(cshape->get_component<touchable::capture>().find() == nullptr);
    }
    SECTION("circle_shape touch-2") {
        gobject_iptr cshape = the<world>().instantiate();
        cshape->entity_filler()
            .component<actor>(node::create(cshape, initializer.scene_r))
            .component<circle_shape>(v2f(), 5.0f)
            .component<touchable>(true);
        node_iptr cshape_node = cshape->get_component<actor>().get().node();
        cshape_node->translation(v3f(55.0f, 45.0f, 0.0f));
        
        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(452.0f, 350.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());

        REQUIRE(cshape->get_component<touched_tag>().find());
        REQUIRE(cshape->get_component<touch_down_event>().find());
        REQUIRE(cshape->get_component<touch_focus_tag>().find());
        REQUIRE(cshape->get_component<touchable::capture>().find() == nullptr);
    }
    SECTION("circle_shape touch outside") {
        gobject_iptr cshape = the<world>().instantiate();
        cshape->entity_filler()
            .component<actor>(node::create(cshape, initializer.scene_r))
            .component<circle_shape>(v2f(), 5.0f)
            .component<touchable>(true);
        node_iptr cshape_node = cshape->get_component<actor>().get().node();
        cshape_node->translation(v3f(55.0f, 45.0f, 0.0f));
        
        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(447.0f, 337.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());
        
        REQUIRE(cshape->get_component<touched_tag>().find() == nullptr);
        REQUIRE(cshape->get_component<touch_down_event>().find() == nullptr);
        REQUIRE(cshape->get_component<touch_focus_tag>().find() == nullptr);
        REQUIRE(cshape->get_component<touchable::capture>().find() == nullptr);
    }

    SECTION("polygon_shape touch-1") {
        gobject_iptr cshape = the<world>().instantiate();
        cshape->entity_filler()
            .component<actor>(node::create(cshape, initializer.scene_r))
            .component<polygon_shape>(std::initializer_list<polygon_shape::triangle>{
                {v3f(0.0f, 10.0f, 0.0f), v3f(10.0f, 3.0f, 0.0f), v3f(-2.0f, -1.0f, 0.0f)}})
            .component<touchable>(true);
        node_iptr cshape_node = cshape->get_component<actor>().get().node();
        cshape_node->translation(v3f(40.0f, 30.0f, 0.0f));
        
        initializer.camera_i->entity().assign_component<input_event>(
            input_event::data_ptr(new input_event::event_data{
                proj_matrix,
                b2u(screen_size).cast_to<f32>(),
                v2f(438.0f, 330.0f),
                v2f(),
                4.0f,
                input_event::event_type::touch_down}));
        
        shape_projection_system().process(the<world>().registry());
        input_event_system().raycast(the<world>().registry());
        input_event_system().post_update(the<world>().registry());
        
        REQUIRE(cshape->get_component<touched_tag>().find() == nullptr);
        REQUIRE(cshape->get_component<touch_down_event>().find() == nullptr);
        REQUIRE(cshape->get_component<touch_focus_tag>().find() == nullptr);
        REQUIRE(cshape->get_component<touchable::capture>().find() == nullptr);
    }
}

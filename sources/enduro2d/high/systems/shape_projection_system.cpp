/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/shape_projection_system.hpp>
#include <enduro2d/high/components/convex_hull_screenspace_collider.hpp>
#include <enduro2d/high/components/rectangle_shape.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/input_event.hpp>

namespace
{
    using namespace e2d;
    
    const v3f forward = v3f(0.0f, 0.0f, 1.0f);
    
    void project_rectangle_shape(
        rectangle_screenspace_collider& screenspace_shape,
        const rectangle_shape& shape,
        const actor& act,
        const input_event& input_ev)
    {
        using plane2d = rectangle_screenspace_collider::plane2d;

        const b2f& viewport = input_ev.data()->viewport;
        const m4f& view_proj = input_ev.data()->view_proj;
        const m4f m = act.node()
            ? act.node()->world_matrix()
            : m4f::identity();
        const m4f mvp = m * view_proj;
        const b2f& r = shape.rectangle();
            
        const std::array<v3f, 4> projected = {{
            math::project(v3f(r.position.x,            r.position.y,            0.0f), mvp, viewport),
            math::project(v3f(r.position.x + r.size.x, r.position.y,            0.0f), mvp, viewport),
            math::project(v3f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f), mvp, viewport),
            math::project(v3f(r.position.x,            r.position.y + r.size.y, 0.0f), mvp, viewport)
        }};

        const v3f face = math::cross(
            projected[1] - projected[0],
            projected[3] - projected[0]);
        const f32 sign = math::dot(face, forward) > 0.0f ? 1.0f : -1.0f;

        const auto make_plane = [sign](const v3f& a, const v3f& b) {
            v3f e2 = v3f(b.x, b.y, 0.0f) - v3f(a.x, a.y, 0.0f);
            v2f n = math::normalized(v2f(math::cross(forward, e2)));
            return plane2d{n, sign * -math::dot(n, v2f(a))};
        };
            
        screenspace_shape.planes = std::array<plane2d, 4>{{
            make_plane(projected[0], projected[1]),
            make_plane(projected[1], projected[2]),
            make_plane(projected[2], projected[3]),
            make_plane(projected[3], projected[0])
        }};
    }
    
    void project_circle_shape(
        circle_screenspace_collider& screenspace_shape,
        const circle_shape& shape,
        const actor& act,
        const input_event& input_ev)
    {
        using plane2d = circle_screenspace_collider::plane2d;
            
        const b2f& viewport = input_ev.data()->viewport;
        const m4f& view_proj = input_ev.data()->view_proj;
        const m4f m = act.node()
            ? act.node()->world_matrix()
            : m4f::identity();
        const m4f mvp = m * view_proj;

        constexpr u32 cnt = circle_shape::detail_level;
        std::array<v3f, cnt> projected;
        for ( size_t i = 0; i < cnt; ++i ) {
            rad<f32> a = 2.0f * math::pi<f32>() * f32(i) / f32(cnt);
            v2f p = shape.center() + shape.radius() * v2f(math::cos(a), math::sin(a));
            projected[i] = math::project(v3f(p, 0.0f), mvp, viewport);
        }

        const v3f face = math::cross(
            projected[cnt/2] - projected[0],
            projected[cnt/2+1] - projected[0]);
        const f32 sign = math::dot(face, forward) > 0.0f ? 1.0f : -1.0f;

        const auto make_plane = [sign](const v3f& a, const v3f& b) {
            v3f e2 = v3f(b.x, b.y, 0.0f) - v3f(a.x, a.y, 0.0f);
            v2f n = math::normalized(v2f(math::cross(forward, e2)));
            return plane2d{n, sign * -math::dot(n, v2f(a))};
        };
        for ( size_t i = 0; i < cnt; ++i ) {
            screenspace_shape.planes[i] = make_plane(projected[i], projected[(i+1) % cnt]);
        }
    }

    void create_rectangle_shapes(ecs::registry& owner, const input_event& input_ev) {
        owner.for_joined_components<rectangle_shape, actor>(
        [&owner, &input_ev](ecs::entity_id id, const rectangle_shape& shape, const actor& act) {
            ecs::entity e(owner, id);
            if ( !e.find_component<rectangle_screenspace_collider>() ) {
                project_rectangle_shape(
                    e.assign_component<rectangle_screenspace_collider>(),
                    shape,
                    act,
                    input_ev);
            }
        });
    }
    
    void create_circle_shapes(ecs::registry& owner, const input_event& input_ev) {
        owner.for_joined_components<circle_shape, actor>(
        [&owner, &input_ev](ecs::entity_id id, const circle_shape& shape, const actor& act) {
            ecs::entity e(owner, id);
            if ( !e.find_component<circle_screenspace_collider>() ) {
                project_circle_shape(
                    e.assign_component<circle_screenspace_collider>(),
                    shape,
                    act,
                    input_ev);
            }
        });
    }

    void project_rectangle_shapes(ecs::registry& owner, const input_event& input_ev) {
        owner.for_joined_components<rectangle_shape, rectangle_screenspace_collider, actor>(
        [&input_ev](
            const ecs::const_entity& e,
            const rectangle_shape& shape,
            rectangle_screenspace_collider& screenspace_shape,
            const actor& act)
        {
            project_rectangle_shape(screenspace_shape, shape, act, input_ev);
        });
    }

    void project_circle_shapes(ecs::registry& owner, const input_event& input_ev) {
        owner.for_joined_components<circle_shape, circle_screenspace_collider, actor>(
        [&input_ev](
            const ecs::const_entity& e,
            const circle_shape& shape,
            circle_screenspace_collider& screenspace_shape,
            const actor& act)
        {
            project_circle_shape(screenspace_shape, shape, act, input_ev);
        });
    }
}

namespace e2d
{
    void shape_projection_system::process(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner, this]
        (ecs::entity_id id, const input_event& input_ev, camera::input_handler_tag, const camera&) {
            const b2f& viewport = input_ev.data()->viewport;
            const m4f& view_proj = input_ev.data()->view_proj;

            if ( camera_entity_ != id ||
                 !math::approximately(last_vp_matrix_, view_proj) ||
                 !math::approximately(last_viewport_, viewport) )
            {
                camera_entity_ = id;
                last_vp_matrix_ = view_proj;
                last_viewport_ = viewport;

                project_rectangle_shapes(owner, input_ev);
                project_circle_shapes(owner, input_ev);
            }

            create_rectangle_shapes(owner, input_ev);
            create_circle_shapes(owner, input_ev);
        });
    }
}

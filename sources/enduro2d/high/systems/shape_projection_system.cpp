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

    void project_rectangle_shape(ecs::registry& owner, const input_event& input_ev) {
        using collider = rectangle_screenspace_collider;

        // add projected shapes
        owner.for_each_component<rectangle_shape>(
        [&owner](ecs::entity_id id, const rectangle_shape&) {
            ecs::entity e(owner, id);
            if ( !e.find_component<collider>() ) {
                e.assign_component<collider>();
            }
        });

        // update projected shapes
        owner.for_joined_components<rectangle_shape, collider, actor>(
        [&input_ev]
        (const ecs::const_entity& e, const rectangle_shape& shape, collider& hull, const actor& act) {
            using plane2d = collider::plane2d;

            const b2f viewport = input_ev.data()->viewport;
            const m4f view_proj = input_ev.data()->view_proj;
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
            
            hull.planes = std::array<plane2d, 4>{{
                make_plane(projected[0], projected[1]),
                make_plane(projected[1], projected[2]),
                make_plane(projected[2], projected[3]),
                make_plane(projected[3], projected[0])
            }};
        });
    }

    void project_circle_shape(ecs::registry& owner, const input_event& input_ev) {
        using collider = circle_screenspace_collider;
        
        // add projected shapes
        owner.for_each_component<circle_shape>(
        [&owner](ecs::entity_id id, const circle_shape&) {
            ecs::entity e(owner, id);
            if ( !e.find_component<collider>() ) {
                e.assign_component<collider>();
            }
        });
        
        // update projected shapes
        owner.for_joined_components<circle_shape, collider, actor>(
        [&input_ev]
        (const ecs::const_entity& e, const circle_shape& shape, collider& hull, const actor& act) {
            using plane2d = collider::plane2d;
            
            const b2f viewport = input_ev.data()->viewport;
            const m4f view_proj = input_ev.data()->view_proj;
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
            for ( size_t i = 0; i < projected.size(); ++i ) {
                hull.planes[i] = make_plane(projected[i], projected[(i+1) % cnt]);
            }
        });
    }
}

namespace e2d
{
    void shape_projection_system::process(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner](const ecs::const_entity& e, const input_event& input_ev, camera::input_handler_tag, const camera&) {
            project_rectangle_shape(owner, input_ev);
            project_circle_shape(owner, input_ev);
        });
    }
}

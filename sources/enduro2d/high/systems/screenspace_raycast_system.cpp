/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/screenspace_raycast_system.hpp>
#include <enduro2d/high/components/screenspace_collider.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/touchable.hpp>
#include <enduro2d/high/components/input_event.hpp>

namespace
{
    using namespace e2d;

    std::tuple<m4f,b2f> get_vp_and_viewport(
        const ecs::const_entity& e,
        const camera& cam) noexcept
    {
        const actor* cam_a = e.find_component<actor>();
        const m4f& cam_w = cam_a && cam_a->node()
            ? cam_a->node()->world_matrix()
            : m4f::identity();
        const auto cam_w_inv = math::inversed(cam_w);
        const m4f& m_v = cam_w_inv.second
            ? cam_w_inv.first
            : m4f::identity();
        const m4f& m_p = cam.projection();

        return std::make_tuple(
            m_v *  m_p,
            cam.viewport().cast_to<f32>());
    }

    template < typename Collider >
    void raycast_on_convex_hull(
        ecs::registry& owner,
        const m4f& view_proj,
        const b2f& viewport,
        const input_event::data_ptr& ev_data)
    {
        owner.for_joined_components<Collider, touchable, actor>(
        [&owner, &view_proj, &viewport, &ev_data]
        (const ecs::entity& e, const Collider& shape, const touchable& t, const actor& act) {
            const m4f m = act.node()
                ? act.node()->world_matrix()
                : m4f::identity();
            const m4f mvp = m * view_proj;
            const v2f touch_center = ev_data->center;
            const f32 touch_radius = ev_data->radius;

            bool inside = true;
            for ( auto& p : shape.planes ) {
                const f32 d = math::dot(p.norm, touch_center) + p.dist;
                inside &= (d > -touch_radius);
            }
            if ( inside ) {
                owner.assign_component<touchable::capture>(e, t.depth(), t.stop_propagation(), ev_data);
            }
        });
    }

    void raycast_on_polygon(
        ecs::registry& owner,
        const m4f& view_proj,
        const b2f& viewport,
        const input_event::data_ptr& ev_data)
    {
        owner.for_joined_components<polygon_screenspace_collider, touchable, actor>(
        [&owner, &view_proj, &viewport, &ev_data]
        (const ecs::entity& e, const polygon_screenspace_collider& shape, const touchable& t, const actor& act) {
            const m4f m = act.node()
                ? act.node()->world_matrix()
                : m4f::identity();
            const m4f mvp = m * view_proj;
            const v2f touch_center = ev_data->center;
            const f32 touch_radius = ev_data->radius;
            
            bool intersects = false;
            for ( auto& tri : shape.triangles ) {
                bool inside = true;
                for ( auto& p : tri.planes ) {
                    const f32 d = math::dot(p.norm, touch_center) + p.dist;
                    inside &= (d > -touch_radius);
                }
                if ( inside ) {
                    intersects = true;
                    break;
                }
            }
                    
            if ( intersects ) {
                owner.assign_component<touchable::capture>(e, t.depth(), t.stop_propagation(), ev_data);
            }
        });
    }
}

namespace e2d
{
    convex_hull_screenspace_raycast_system::convex_hull_screenspace_raycast_system() = default;

    convex_hull_screenspace_raycast_system::~convex_hull_screenspace_raycast_system() noexcept = default;

    void convex_hull_screenspace_raycast_system::process(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner](const ecs::const_entity& e, const input_event& input_ev, camera::input_handler_tag, const camera& cam) {
            auto&[view_proj, viewport] = get_vp_and_viewport(e, cam);

            raycast_on_convex_hull<rectangle_screenspace_collider>(owner, view_proj, viewport, input_ev.data());
            raycast_on_convex_hull<circle_screenspace_collider>(owner, view_proj, viewport, input_ev.data());
            raycast_on_polygon(owner, view_proj, viewport, input_ev.data());
        });
    }
}

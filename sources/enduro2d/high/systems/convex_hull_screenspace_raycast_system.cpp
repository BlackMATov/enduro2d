/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/convex_hull_screenspace_raycast_system.hpp>
#include <enduro2d/high/components/convex_hull_screenspace_collider.hpp>
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
}

namespace e2d
{
    void convex_hull_screenspace_raycast_system::process(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner](const ecs::const_entity& e, const input_event& input_ev, camera::input_handler_tag, const camera& cam) {
            auto vp_and_viewport = get_vp_and_viewport(e, cam);

            owner.for_joined_components<convex_hull_screenspace_collider, touchable, actor>(
            [&owner, view_proj = std::get<0>(vp_and_viewport), viewport = std::get<1>(vp_and_viewport), &input_ev]
            (const ecs::entity& e, const convex_hull_screenspace_collider& shape, const touchable& t, const actor& act) {
                const m4f m = act.node()
                    ? act.node()->world_matrix()
                    : m4f::identity();
                const m4f mvp = m * view_proj;
                const v2f touch_center = input_ev.data()->center;
                const f32 touch_radius = input_ev.data()->radius;

                bool inside = true;
                for ( size_t i = 0; i < shape.plane_count(); ++i ) {
                    auto& p = shape.plane(i);
                    const f32 d = math::dot(p.norm, touch_center) + p.dist;
                    inside &= (d > -touch_radius);
                }
                if ( inside ) {
                    owner.assign_component<touchable::capture>(e, t.depth(), t.stop_propagation(), input_ev.data());
                }
            });
        });
    }
}

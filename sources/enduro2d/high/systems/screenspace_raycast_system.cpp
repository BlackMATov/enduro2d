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

    template < typename Collider >
    void raycast_on_convex_hull(ecs::registry& owner, const input_event::data_ptr& ev_data) {
        owner.for_joined_components<Collider, touchable, actor>(
        [&owner, &ev_data]
        (const ecs::entity& e, const Collider& shape, const touchable& t, const actor& act) {
            const v2f touch_center = ev_data->center;
            const f32 touch_radius = ev_data->radius;

            bool inside = true;
            for ( auto& p : shape.planes ) {
                const f32 d = math::dot(p.norm, touch_center) + p.dist;
                inside &= (d > -touch_radius);
            }
            if ( inside ) {
                u32 d = act.node()->render_order();
                owner.assign_component<touchable::capture>(e, d, t.stop_propagation(), ev_data);
            }
        });
    }

    void raycast_on_polygon(ecs::registry& owner, const input_event::data_ptr& ev_data) {
        owner.for_joined_components<polygon_screenspace_collider, touchable, actor>(
        [&owner, &ev_data]
        (const ecs::entity& e, const polygon_screenspace_collider& shape, const touchable& t, const actor& act) {
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
                u32 d = act.node()->render_order();
                owner.assign_component<touchable::capture>(e, d, t.stop_propagation(), ev_data);
            }
        });
    }
}

namespace e2d
{
    screenspace_raycast_system::screenspace_raycast_system() = default;

    screenspace_raycast_system::~screenspace_raycast_system() noexcept = default;

    void screenspace_raycast_system::process(ecs::registry& owner) {
        owner.for_joined_components<input_event, camera::input_handler_tag, camera>(
        [&owner](const ecs::const_entity&, const input_event& input_ev, camera::input_handler_tag, const camera&) {
            if ( input_ev.data()->type != input_event::event_type::mouse_move &&
                 input_ev.data()->type != input_event::event_type::touch_down )
            {
                return;
            }

            raycast_on_convex_hull<rectangle_screenspace_collider>(owner, input_ev.data());
            raycast_on_convex_hull<circle_screenspace_collider>(owner, input_ev.data());
            raycast_on_polygon(owner, input_ev.data());
        });
    }
}

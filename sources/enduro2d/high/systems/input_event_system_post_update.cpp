/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/input_event_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/touchable.hpp>

namespace e2d
{
    void input_event_system_post_update::process(ecs::registry& owner, ecs::event_ref) {
        using input_event_type = input_event::event_type;

        const u32 frame_id = the<engine>().frame_count();

        const auto add_mouse_leave_events = [&owner, frame_id](const input_event::data_ptr& ev_data) {
            owner.for_each_component<mouse_over_tag>(
            [&owner, ev_data, fid = frame_id](ecs::entity e, const mouse_over_tag& tag) {
                if ( tag.frame_id != fid ) {
                    e.assign_component<mouse_leave_event>(mouse_leave_event{ev_data});
                }
            });
            owner.for_each_component<mouse_leave_event>(
            [&owner](ecs::entity e, const mouse_leave_event&) {
                e.remove_component<mouse_over_tag>();
            });
        };

        ecs::entity_id capture_id;
        input_event::data_ptr ev_data;
        u32 depth = 0;
        bool stop_prop = false;

        owner.for_each_component<touchable::capture>(
        [&](ecs::entity_id e, const touchable::capture& c) {
            if ( c.depth() >= depth ) {
                stop_prop = c.stop_propagation();
                capture_id = e;
                depth = c.depth();
                ev_data = c.data();
            }
        });
        owner.remove_all_components<touchable::capture>();

        if ( !ev_data ) {
            // for mouse_move only
            owner.for_each_component<input_event>(
            [&ev_data](const ecs::const_entity&, const input_event& input) {
                if ( input.data()->type == input_event_type::mouse_move ) {
                    ev_data = input.data();
                }
            });
            if ( !ev_data ) {
                return;
            }

            add_mouse_leave_events(ev_data);
            return;
        }

        E2D_ASSERT(ev_data->type == input_event_type::mouse_move
            || ev_data->type == input_event_type::touch_down);

        auto add_tag = [&ev_data, frame_id](ecs::entity& e) {
            if ( ev_data->type == input_event_type::touch_down ) {
                e.assign_component<touched_tag>();
                e.assign_component<touch_down_event>(touch_down_event{ev_data});
            }
            if ( ev_data->type == input_event_type::mouse_move ) {
                if ( auto* tag = e.find_component<mouse_over_tag>() ) {
                    tag->frame_id = frame_id;
                    e.assign_component<mouse_move_event>(mouse_move_event{ev_data});
                } else {
                    e.assign_component<mouse_over_tag>(mouse_over_tag{frame_id});
                    e.assign_component<mouse_enter_event>(mouse_enter_event{ev_data});
                }
            }
        };

        ecs::entity_id last_touched = capture_id;
        ecs::entity top_e(owner, capture_id);

        add_tag(top_e);
           
        if ( !stop_prop ) {
            auto* act = top_e.find_component<actor>();
            if ( act && act->node() ) {
                for ( auto n = act->node()->parent(); n; n = n->parent() ) {
                    if ( !n->owner() ) {
                        continue;
                    }
                    auto* t_comp = n->owner()->entity().find_component<touchable>();
                    if ( t_comp ) {
                        ecs::entity e = n->owner()->entity();
                        last_touched = e.id();
                        add_tag(e);
                        if ( t_comp->stop_propagation() ) {
                            break;
                        }
                    }
                }
            }
        }
            
        if ( ev_data->type == input_event_type::touch_down ) {
            ecs::entity(owner, last_touched)
                .assign_component<touch_focus_tag>();
        }
            
        if ( ev_data->type == input_event_type::mouse_move ) {
            add_mouse_leave_events(ev_data);
        }
    }
}

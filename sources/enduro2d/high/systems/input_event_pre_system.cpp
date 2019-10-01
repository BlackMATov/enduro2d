/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/input_event_system.hpp>

#include <enduro2d/high/components/input_event.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/touchable.hpp>

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
    
    using input_event_type = input_event::event_type;
}

namespace e2d
{
    //
    // internal_state
    //

    class input_event_pre_system::internal_state final {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;
        
        void process(ecs::registry& owner);
    private:
        v2f mouse_delta_;
        v2f last_cursor_pos_ {-1.0e10f};
        u32 frame_id_ = 0;
    };

    void input_event_pre_system::internal_state::process(ecs::registry& owner) {
        owner.remove_all_components<input_event>();
        owner.remove_all_components<touch_down_event>();
        owner.remove_all_components<touch_up_event>();
        owner.remove_all_components<touch_move_event>();
        owner.remove_all_components<mouse_enter_event>();
        owner.remove_all_components<mouse_leave_event>();
        owner.remove_all_components<mouse_move_event>();
        
        const mouse& m = the<input>().mouse();
            
        input_event_type ev_type = input_event_type(0);
        mouse_delta_ = v2f();

        if ( m.is_button_just_pressed(mouse_button::left) ) {
            ev_type = input_event_type::touch_down;
            mouse_delta_ = v2f();
            last_cursor_pos_ = m.cursor_pos();

        } else if ( m.is_button_just_released(mouse_button::left) ) {
            ev_type = input_event_type::touch_up;

        } else if ( !math::approximately(last_cursor_pos_, m.cursor_pos(), 0.1f) ) {
            if ( m.is_button_pressed(mouse_button::left) ) {
                ev_type = input_event_type::touch_move;
            } else {
                ev_type = input_event_type::mouse_move;
            }
            mouse_delta_ = m.cursor_pos() - last_cursor_pos_;
            last_cursor_pos_ = m.cursor_pos();

        } else {
            return;
        }

        ecs::const_entity cam_e(owner);
        b2f viewport;
        m4f vp;

        owner.for_joined_components<camera::input_handler_tag, camera>(
        [&cam_e, &viewport, &vp, pos = m.cursor_pos()]
        (const ecs::const_entity& e, camera::input_handler_tag, const camera& cam) {
            if ( cam.target() ) {
                return;
            }
            viewport = cam.viewport().cast_to<f32>();

            if ( !math::inside(viewport, pos) ) {
                return;
            }
            cam_e = e;
            vp = std::get<0>(get_vp_and_viewport(e, cam));
        });

        if ( cam_e.valid() ) {
            input_event::data_ptr ev_data(new input_event::event_data{
                vp,
                viewport,
                v2f(m.cursor_pos().x, viewport.size.y - m.cursor_pos().y),
                v2f(mouse_delta_.x, -mouse_delta_.y),
                4.0f,
                ev_type});
            
            ecs::entity(owner, cam_e.id()).assign_component<input_event>(ev_data);

            switch ( ev_type ) {
                case input_event_type::mouse_move:
                case input_event_type::touch_down:
                    owner.enque_event(input_event_system::raycast_evt());
                    break;

                case input_event_type::touch_up:
                    owner.for_each_component<touched_tag>(
                    [&owner, ev_data](ecs::entity e, touched_tag) {
                        e.assign_component<touch_up_event>(touch_up_event{ev_data});
                    });
                    owner.remove_all_components<touch_focus_tag>();
                    owner.remove_all_components<touched_tag>();
                    break;

                case input_event_type::touch_move:
                    owner.for_each_component<touch_focus_tag>(
                    [&owner, ev_data](ecs::entity e, touch_focus_tag) {
                        e.assign_component<touch_move_event>(touch_move_event{ev_data});
                    });
                    break;
            }
        }
        owner.enque_event(input_event_system::post_update_evt());
    }

    //
    // input_event_pre_system
    //

    input_event_pre_system::input_event_pre_system()
    : state_(std::make_unique<internal_state>()) {}

    input_event_pre_system::~input_event_pre_system() = default;

    void input_event_pre_system::process(ecs::registry& owner, ecs::event_ref) {
        state_->process(owner);
    }
}

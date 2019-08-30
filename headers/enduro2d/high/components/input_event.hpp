/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class input_event final {
    public:
        enum class event_type {
            mouse_move,
            touch_down,
            touch_up,
            touch_move,
        };

        class event_data final {
        public:
            m4f view_proj;
            b2f viewport;
            v2f center;
            v2f delta;
            f32 radius;
            event_type type;
        };
        using data_ptr = std::shared_ptr<const event_data>;
    public:
        input_event(const data_ptr&);

        const data_ptr& data() const noexcept;
    private:
        data_ptr data_;
    };
    

    class touch_down_event {
    public:
        input_event::data_ptr data;
    };

    class touch_up_event {
    public:
        input_event::data_ptr data;
    };

    class touch_move_event {
    public:
        input_event::data_ptr data;
    };

    class touched_tag {
    };

    class touch_focus_tag {
    };

    
    class mouse_enter_event {
    public:
        input_event::data_ptr data;
    };

    class mouse_leave_event {
    public:
        input_event::data_ptr data;
    };

    class mouse_move_event {
    public:
        input_event::data_ptr data;
    };
    
    class mouse_over_tag {
    public:
        u32 frame_id = 0;
    };
}

namespace e2d
{
    inline input_event::input_event(const data_ptr& d)
    : data_(d) {}

    inline const input_event::data_ptr& input_event::data() const noexcept {
        return data_;
    }
}

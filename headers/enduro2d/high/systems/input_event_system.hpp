/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class input_event_system final : public ecs::system {
    public:
        void pre_update(ecs::registry& owner);
        void raycast(ecs::registry& owner);
        void post_update(ecs::registry& owner);
    private:
        v2f mouse_delta_;
        v2f last_cursor_pos_ {-1.0e10f};
        u32 frame_id_ = 0;
    };
}

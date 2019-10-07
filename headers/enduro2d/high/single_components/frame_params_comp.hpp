/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class frame_params_comp final {
    public:
        secf delta_time {0.0f};
        secf realtime_time {0.0f}; // from engine start

        f32 animation_time_scale = 1.0f;
    };
}

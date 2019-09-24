/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

namespace e2d::world_ev
{
    struct update_frame {};
    struct render_frame {};

    struct input_event_raycast {};
    struct input_event_post_update {};
}

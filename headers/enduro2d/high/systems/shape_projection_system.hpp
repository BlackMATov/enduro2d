/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class shape_projection_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    private:
        ecs::entity_id camera_entity_ = 0;
        m4f last_vp_matrix_;
        b2f last_viewport_;
    };
}

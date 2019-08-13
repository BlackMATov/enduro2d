/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class spine_render_system final : public ecs::system {
    public:
        spine_render_system();
        ~spine_render_system() noexcept final;
        void process(ecs::registry& owner) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}

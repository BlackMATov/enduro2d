/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class spine_pre_system final : public ecs::system {
    public:
        spine_pre_system();
        ~spine_pre_system() noexcept final;
        void process(ecs::registry& owner) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };

    class spine_post_system final : public ecs::system {
    public:
        spine_post_system();
        ~spine_post_system() noexcept final;
        void process(ecs::registry& owner) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}

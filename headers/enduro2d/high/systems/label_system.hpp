/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class label_system final : public ecs::system {
    public:
        label_system();
        ~label_system() noexcept;
        void process(ecs::registry& owner, ecs::event_ref) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}

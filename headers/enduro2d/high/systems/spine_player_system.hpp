/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class spine_player_system final : public ecs::system {
    public:
        spine_player_system();
        ~spine_player_system() noexcept final;
        void process(ecs::registry& owner) override;
    };
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#pragma once

#include "_high.hpp"

namespace e2d
{
    class world final : public module<world> {
    public:
        world();
        ~world() noexcept final;

        ecs::registry& registry() noexcept;
        const ecs::registry& registry() const noexcept;
    private:
        ecs::registry registry_;
    };
}
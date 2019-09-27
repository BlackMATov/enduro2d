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
        struct pre_update_evt {};
        struct raycast_evt {};
        struct post_update_evt {};
    public:
        void add_systems(ecs::registry& owner) const override;
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };

    class input_event_system_pre_update final : public ecs::system {
    public:
        input_event_system_pre_update();
        ~input_event_system_pre_update() noexcept;
        void process(ecs::registry& owner, ecs::event_ref event) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
    
    class input_event_system_raycast final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
    
    class input_event_system_post_update final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
}

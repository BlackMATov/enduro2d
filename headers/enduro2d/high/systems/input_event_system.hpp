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
        void process(ecs::registry& owner) override;
    };

    class input_event_pre_system final : public ecs::system {
    public:
        input_event_pre_system();
        ~input_event_pre_system() noexcept;
        void process(ecs::registry& owner) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
    
    class input_event_raycast_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };
    
    class input_event_post_system final : public ecs::system {
    public:
        input_event_post_system();
        ~input_event_post_system() noexcept;
        void process(ecs::registry& owner) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}

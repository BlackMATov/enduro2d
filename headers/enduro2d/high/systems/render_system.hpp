/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class render_system final : public ecs::system {
    public:
        struct render_with_camera_evt {
            render_queue::command_encoder_ptr rq;
        };
    public:
        render_system();
        ~render_system() noexcept;

        void add_systems(ecs::registry& owner) const override;
        void process(ecs::registry& owner, ecs::event_ref) override;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
    
    class spine_render_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
    
    class sprite_render_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
    
    class sprite_9p_render_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
    
    class label_render_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };

    class model_render_system final : public ecs::system {
    public:
        void process(ecs::registry& owner, ecs::event_ref event) override;
    };
}

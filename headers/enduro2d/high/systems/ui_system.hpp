/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../components/ui_style.hpp"

namespace e2d
{
    class ui_system final : public ecs::system {
    public:
        struct update_controllers_evt {
            using changed_states = flat_map<ecs::entity_id, ui_style::ui_style_state>;
            mutable changed_states changed;
        };

        struct update_layouts_evt {};
        
        struct update_animation_evt {};
    public:
        void add_systems(ecs::registry& owner) const override;
        void process(ecs::registry& owner) override;
    };

    class ui_style_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };
    
    class ui_layout_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };

    class ui_controller_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };

    class ui_controller_event_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    };
    
    class ui_animation_system final : public ecs::system {
    public:
        void process(ecs::registry& owner) override;
    private:
        f32 time_scale_ = 1.0f;
    };
}

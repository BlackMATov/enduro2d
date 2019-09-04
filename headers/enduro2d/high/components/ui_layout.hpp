/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"

namespace e2d
{
    class ui_layout final {
    public:
        class root_tag final {};
        
        struct layout_state;
        using update_fn_t = void(*)(ecs::entity&, const b2f&, std::vector<layout_state>&);
    public:
        ui_layout& update_fn(update_fn_t fn) noexcept;
        update_fn_t update_fn() const noexcept;

        ui_layout& region(const b2f& value) noexcept;
        const b2f& region() const noexcept;
    private:
        update_fn_t update_ = nullptr;
        b2f region_;
    };
    
    struct ui_layout::layout_state {
        ecs::entity_id id;
        update_fn_t update;
        b2f region; // in: required size or region of the layout
                    // out: region that can be used by layout
    };

    template <>
    class factory_loader<ui_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class fixed_layout final {
    public:
        class dirty_flag final {};
    public:
        fixed_layout() = default;
        fixed_layout(const b2f& r);

        fixed_layout& region(const b2f& value) noexcept;
        const b2f& region() const noexcept;
    private:
        b2f region_;
    };
    
    template <>
    class factory_loader<fixed_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            fixed_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class auto_layout final {
    public:
        class dirty_flag final {};
    private:

    };
    
    template <>
    class factory_loader<auto_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            auto_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class stack_layout final {
    public:
        class dirty_flag final {};
    private:
        std::vector<b2f> child_regions_;
    };
    
    template <>
    class factory_loader<stack_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            stack_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class fill_stack_layout final {
    public:
        class dirty_flag final {};
    private:
        std::vector<b2f> child_regions_;
    };
    
    template <>
    class factory_loader<fill_stack_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            fill_stack_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class dock_layout final {
    public:
        class dirty_flag final {};

        enum class dock_type : u8 {
            none = 0,
            left = 1 << 0,
            right = 1 << 1,
            bottom = 1 << 2,
            top = 1 << 3,
            fill = left | right | bottom | top,
        };
    public:

    private:
        dock_type dock_ = dock_type::none;
        v2f size_;
    };
    
    template <>
    class factory_loader<dock_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            dock_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

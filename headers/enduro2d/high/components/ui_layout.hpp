/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"
#include "../node.hpp"
#include "../factory.hpp"

namespace e2d
{
    class ui_layout final {
    public:
        class root_tag final {};
        
        struct layout_state;
        using update_fn_t = void(*)(ecs::entity&, const b2f&, const node_iptr&, std::vector<layout_state>&);
    public:
        ui_layout& update_fn(update_fn_t fn) noexcept;
        update_fn_t update_fn() const noexcept;

        ui_layout& depends_on_childs(bool enable) noexcept;
        bool depends_on_childs() const noexcept;
        
        ui_layout& depends_on_parent(bool enable) noexcept;
        bool depends_on_parent() const noexcept;
    private:
        update_fn_t update_ = nullptr;
        bool depends_on_childs_ = false; // set true if size depends on child sizes
        bool depends_on_parent_ = false; // set true if size depends on parent size
    };
    
    struct ui_layout::layout_state {
        ecs::entity_id id;
        update_fn_t update;
        node_iptr node; // node contains left bottom coordinate of layout
        b2f parent_rect; // region that can be allocated (in parent space)
        bool is_post_update;
        bool depends_on_childs;
        bool depends_on_parent;
    };
    
    template <>
    class factory_loader<ui_layout::root_tag> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_layout::root_tag& component,
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
        class dirty final {};
    public:
        fixed_layout() = default;
        fixed_layout(const v2f& size);

        fixed_layout& size(const v2f& value) noexcept;
        const v2f& size() const noexcept;
    private:
        v2f size_;
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
    
    template <>
    class factory_loader<fixed_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            fixed_layout::dirty& component,
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
        class dirty final {};
    public:
        auto_layout() = default;

        auto_layout& min_size(const v2f& value) noexcept;
        const v2f& min_size() const noexcept;

        auto_layout& max_size(const v2f& value) noexcept;
        const v2f& max_size() const noexcept;
    private:
        v2f min_size_;
        v2f max_size_ {std::numeric_limits<f32>::max()};
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
    
    template <>
    class factory_loader<auto_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            auto_layout::dirty& component,
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
        class dirty final {};

        enum class stack_origin {
            left,
            top,
            right,
            bottom,
        };
    public:
        stack_layout() = default;
        stack_layout(stack_origin value);

        stack_layout& origin(stack_origin value) noexcept;
        stack_origin origin() const noexcept;
    private:
        stack_origin origin_ = stack_origin::top;
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
    
    template <>
    class factory_loader<stack_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            stack_layout::dirty& component,
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
        class dirty final {};

        enum class dock_type : u8 {
            none = 0,
            left = 1 << 0,
            right = 1 << 1,
            bottom = 1 << 2,
            top = 1 << 3,
            center_x = 1 << 4,
            center_y = 1 << 5,
            fill_x = left | right,
            fill_y = bottom | top,
            fill = fill_x | fill_y,
        };
    public:
        dock_layout() = default;
        dock_layout(dock_type dock);
        
        dock_layout& dock(dock_type value) noexcept;
        dock_type dock() const noexcept;
        bool has_dock(dock_type value) const noexcept;
    private:
        dock_type dock_ = dock_type::none;
    };

    dock_layout::dock_type operator|(dock_layout::dock_type l, dock_layout::dock_type r) noexcept;
    
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
    
    template <>
    class factory_loader<dock_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            dock_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class image_layout final {
    public:
        class dirty final {};
    public:
        image_layout() = default;
        
        image_layout& size(const v2f& value) noexcept;
        image_layout& preserve_aspect(bool value) noexcept;
        
        const v2f& size() const noexcept;
        bool preserve_aspect() const noexcept;
    private:
        v2f size_;
        bool preserve_aspect_ = true;
    };
    
    template <>
    class factory_loader<image_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            image_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };

    template <>
    class factory_loader<image_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            image_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class label_layout final {
    public:
        class dirty final {};
    public:
        label_layout() = default;
    };
    
    template <>
    class factory_loader<label_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            label_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };

    template <>
    class factory_loader<label_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            label_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class margin_layout final {
    public:
        class dirty final {};
    public:
        margin_layout() = default;
        margin_layout(f32 margin);
        margin_layout(f32 left, f32 bottom, f32 right, f32 top);

        margin_layout& left(f32 value) noexcept;
        f32 left() const noexcept;

        margin_layout& top(f32 value) noexcept;
        f32 top() const noexcept;

        margin_layout& right(f32 value) noexcept;
        f32 right() const noexcept;

        margin_layout& bottom(f32 value) noexcept;
        f32 bottom() const noexcept;
    private:
        f32 left_{0.0f}, bottom_{0.0f}, right_{0.0f}, top_{0.0f};
    };
    
    template <>
    class factory_loader<margin_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            margin_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
    
    template <>
    class factory_loader<margin_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            margin_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class padding_layout final {
    public:
        class dirty final {};
    public:
        padding_layout() = default;
        padding_layout(f32 left, f32 bottom, f32 right, f32 top);
        
        padding_layout& left(f32 value) noexcept;
        f32 left() const noexcept;

        padding_layout& top(f32 value) noexcept;
        f32 top() const noexcept;

        padding_layout& right(f32 value) noexcept;
        f32 right() const noexcept;

        padding_layout& bottom(f32 value) noexcept;
        f32 bottom() const noexcept;
    private:
        f32 left_{0.0f}, bottom_{0.0f}, right_{0.0f}, top_{0.0f};
    };
    
    template <>
    class factory_loader<padding_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            padding_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };

    template <>
    class factory_loader<padding_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            padding_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class anchor_layout final {
    public:
        class dirty final {};
        struct anchor {
            v2f position; // in unorm coords
            v2f offset; // in unorm or in local space coords
            bool relative_offset = false;
        };
    public:
        anchor_layout() = default;
        
        anchor_layout& left_bottom(const anchor& value) noexcept;
        const anchor& left_bottom() const noexcept;

        anchor_layout& right_top(const anchor& value) noexcept;
        const anchor& right_top() const noexcept;
    private:
        anchor left_bottom_;
        anchor right_top_;
    };
    
    template <>
    class factory_loader<anchor_layout> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            anchor_layout& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };

    template <>
    class factory_loader<anchor_layout::dirty> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            anchor_layout::dirty& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

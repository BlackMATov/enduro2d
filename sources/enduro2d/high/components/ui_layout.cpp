/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/ui_layout.hpp>

namespace e2d
{
    ui_layout& ui_layout::update_fn(update_fn_t fn) noexcept {
        update_ = fn;
        return *this;
    }

    ui_layout::update_fn_t ui_layout::update_fn() const noexcept {
        return update_;
    }

    ui_layout& ui_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& ui_layout::size() const noexcept {
        return size_;
    }

    ui_layout& ui_layout::post_update(bool enable) noexcept {
        post_update_ = enable;
        return *this;
    }

    bool ui_layout::post_update() const noexcept {
        return post_update_;
    }
}

namespace e2d
{
    const char* factory_loader<ui_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<ui_layout>::operator()(
        ui_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    fixed_layout::fixed_layout(const v2f& size)
    : size_(size) {}

    fixed_layout& fixed_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& fixed_layout::size() const noexcept {
        return size_;
    }
}

namespace e2d
{
    const char* factory_loader<fixed_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<fixed_layout>::operator()(
        fixed_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<fixed_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<auto_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<auto_layout>::operator()(
        auto_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<auto_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    stack_layout::stack_layout(stack_origin value)
    : origin_(value) {}

    stack_layout& stack_layout::origin(stack_origin value) noexcept {
        origin_ = value;
        return *this;
    }

    stack_layout::stack_origin stack_layout::origin() const noexcept {
        return origin_;
    }
}

namespace e2d
{
    const char* factory_loader<stack_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<stack_layout>::operator()(
        stack_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<stack_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<fill_stack_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<fill_stack_layout>::operator()(
        fill_stack_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<fill_stack_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<dock_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<dock_layout>::operator()(
        dock_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<dock_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

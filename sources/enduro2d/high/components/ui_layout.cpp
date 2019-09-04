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

    ui_layout& ui_layout::region(const b2f& value) noexcept {
        region_ = value;
        return *this;
    }

    const b2f& ui_layout::region() const noexcept {
        return region_;
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
    fixed_layout::fixed_layout(const b2f& r)
    : region_(r) {}

    fixed_layout& fixed_layout::region(const b2f& value) noexcept {
        region_ = value;
        return *this;
    }

    const b2f& fixed_layout::region() const noexcept {
        return region_;
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

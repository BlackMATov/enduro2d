/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/ui_controller.hpp>

namespace e2d
{
    const char* factory_loader<ui_button>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_button>::operator()(
        ui_button& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_button>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<ui_selectable>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_selectable>::operator()(
        ui_selectable& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_selectable>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    ui_draggable& ui_draggable::lock_x(bool value) noexcept {
        lock_x_ = value;
        return *this;
    }

    ui_draggable& ui_draggable::lock_y(bool value) noexcept {
        lock_y_ = value;
        return *this;
    }

    bool ui_draggable::lock_x() const noexcept {
        return lock_x_;
    }

    bool ui_draggable::lock_y() const noexcept {
        return lock_y_;
    }

    const char* factory_loader<ui_draggable>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "lock_x" : { "type" : "boolean" },
            "lock_y" : { "type" : "boolean" }
        }
    })json";

    bool factory_loader<ui_draggable>::operator()(
        ui_draggable& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("lock_x") ) {
            component.lock_x(ctx.root["lock_x"].GetBool());
        }
        if ( ctx.root.HasMember("lock_y") ) {
            component.lock_y(ctx.root["lock_y"].GetBool());
        }
        return true;
    }

    bool factory_loader<ui_draggable>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

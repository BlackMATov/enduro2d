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
    ui_selectable& ui_selectable::selected(bool value) noexcept {
        selected_ = value;
        return *this;
    }

    bool ui_selectable::selected() const noexcept {
        return selected_;
    }

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
    const char* factory_loader<ui_draggable>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_draggable>::operator()(
        ui_draggable& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
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

namespace e2d
{
    ui_scrollable& ui_scrollable::separate_axes(bool value) noexcept {
        separate_axes_ = value;
        return *this;
    }

    bool ui_scrollable::separate_axes() const noexcept {
        return separate_axes_;
    }
    
    ui_scrollable& ui_scrollable::allowed_directions(direction value) noexcept {
        allowed_directions_ = value;
        return *this;
    }

    ui_scrollable::direction ui_scrollable::allowed_directions() const noexcept {
        return allowed_directions_;
    }
    
    bool ui_scrollable::is_vertical() const noexcept {
        return u32(allowed_directions_) & u32(direction::vertical);
    }

    bool ui_scrollable::is_horizontal() const noexcept {
        return u32(allowed_directions_) & u32(direction::horizontal);
    }

    const char* factory_loader<ui_scrollable>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "separate_axes" : { "type" : "boolean" },
            "allowed_directions" : { "$ref": "#/definitions/direction" }
        },
        "definitions" : {
            "direction" : {
                "type" : "string",
                "enum" : [
                    "vertical",
                    "horizontal",
                    "all"
                ]
            }
        }
    })json";

    bool factory_loader<ui_scrollable>::operator()(
        ui_scrollable& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("separate_axes") ) {
            component.separate_axes(ctx.root["separate_axes"].GetBool());
        }
        
        if ( ctx.root.HasMember("allowed_directions") ) {
            auto& json_dir = ctx.root["allowed_directions"];
            E2D_ASSERT(json_dir.IsString());
            str_view value = json_dir.GetString();
            if ( value == "vertical" ) {
                component.allowed_directions(ui_scrollable::direction::vertical);
            } else if ( value == "horizontal" ) {
                component.allowed_directions(ui_scrollable::direction::horizontal);
            } else if ( value == "all" ) {
                component.allowed_directions(ui_scrollable::direction::all);
            } else {
                the<debug>().error("UI_SCROLLABLE: Incorrect formatting of 'allowed_directions' property");
                return false;
            }
        }

        return true;
    }

    bool factory_loader<ui_scrollable>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    ui_controller_event_name& ui_controller_event_name::set_name(str value) {
        name_ = std::move(value);
        return *this;
    }

    const str& ui_controller_event_name::name() const noexcept {
        return name_;
    }

    const char* factory_loader<ui_controller_event_name>::schema_source = R"json({
        "type" : "object",
        "required" : [ "name" ],
        "additionalProperties" : false,
        "properties" : {
            "name" : { "$ref": "#/common_definitions/name" }
        }
    })json";

    bool factory_loader<ui_controller_event_name>::operator()(
        ui_controller_event_name& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("name"));
        component.set_name(ctx.root["name"].GetString());
        return true;
    }

    bool factory_loader<ui_controller_event_name>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

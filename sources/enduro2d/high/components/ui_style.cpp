/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/ui_style.hpp>

namespace
{
    using namespace e2d;

    bool parse_bit_name(str_view str, ui_style::type& t) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { t = ui_style::type::x; return true; }
        DEFINE_IF(disabled);
        DEFINE_IF(mouse_over);
        DEFINE_IF(touched);
        DEFINE_IF(selected);
        DEFINE_IF(dragging);
    #undef DEFINE_IF
        return false;
    }

    bool parse_style_bits(str_view str, ui_style::bits& r) noexcept {
        size_t begin = 0;
        for ( size_t i = 0; i < str.length(); ++i ) {
            const char c = str[i];
            if ( c == ',' || c == '|' || c == ' ' || c == '\t' ) {
                if ( i == begin+1 ) {
                    begin = i;
                    continue;
                }

                ui_style::type t;
                if ( !parse_bit_name(str.substr(), t) ) {
                    return false;
                }
                r.set(t);
                begin = i;
            }
        }
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<ui_style>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "propagate" : { "type" : "string" }
        }
    })json";

    bool factory_loader<ui_style>::operator()(
        ui_style& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("propagate") ) {
            ui_style::bits propagate;
            if ( !parse_style_bits(ctx.root["propagate"].GetString(), propagate) ) {
                the<debug>().error("UI_STYLE: Incorrect formatting of 'propagate' property");
                return false;
            }
            component.propagate(propagate);
        }
        return true;
    }

    bool factory_loader<ui_style>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<ui_style::style_changed_tag>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_style::style_changed_tag>::operator()(
        ui_style::style_changed_tag& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_style::style_changed_tag>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<ui_color_style_comp>::schema_source = R"json({
        "type" : "object",
        "required" : [ "style" ],
        "additionalProperties" : false,
        "properties" : {
            "style" : { "$ref": "#/common_definitions/address" }
        }
    })json";

    bool factory_loader<ui_color_style_comp>::operator()(
        ui_color_style_comp& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("style") ) {
            auto style = ctx.dependencies.find_asset<ui_color_style_asset>(
                path::combine(ctx.parent_address, ctx.root["style"].GetString()));
            if ( !style ) {
                the<debug>().error("UI_COLOR_STYLE_COMP: Dependency 'style' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["style"].GetString());
                return false;
            }
            component.style(style);
        }

        return true;
    }

    bool factory_loader<ui_color_style_comp>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        if ( ctx.root.HasMember("style") ) {
            dependencies.add_dependency<ui_color_style_asset>(
                path::combine(ctx.parent_address, ctx.root["style"].GetString()));
        }
        return true;
    }
}

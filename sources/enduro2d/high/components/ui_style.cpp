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

    bool parse_style_bits(str_view str, ui_style::ui_style_state& r) noexcept {
        if ( str == "all" ) {
            r.set_all();
            return true;
        }
        size_t begin = 0;
        size_t i = 0;
        ui_style::type t;
        for (; i < str.length(); ++i ) {
            const char c = str[i];
            if ( c == ',' || c == '|' || c == ' ' || c == '\t' ) {
                if ( i == begin ) {
                    begin = i+1;
                    continue;
                }
                if ( !parse_bit_name(str.substr(begin, i-begin), t) ) {
                    return false;
                }
                r.set(t);
                begin = i+1;
            }
        }
        if ( begin < str.length() ) {
            if ( !parse_bit_name(str.substr(begin), t) ) {
                return false;
            }
            r.set(t);
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
            ui_style::ui_style_state propagate;
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
        "required" : [ "address" ],
        "additionalProperties" : false,
        "properties" : {
            "address" : { "$ref": "#/common_definitions/address" }
        }
    })json";

    bool factory_loader<ui_color_style_comp>::operator()(
        ui_color_style_comp& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("address") ) {
            auto style = ctx.dependencies.find_asset<ui_color_style_asset>(
                path::combine(ctx.parent_address, ctx.root["address"].GetString()));
            if ( !style ) {
                the<debug>().error("UI_COLOR_STYLE_COMP: Dependency 'address' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["address"].GetString());
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
        if ( ctx.root.HasMember("address") ) {
            dependencies.add_dependency<ui_color_style_asset>(
                path::combine(ctx.parent_address, ctx.root["address"].GetString()));
        }
        return true;
    }
}

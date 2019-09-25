/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/sprite_9p_renderer.hpp>

namespace e2d
{
    const char* factory_loader<sprite_9p_renderer>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "tint" : { "$ref": "#/common_definitions/color" },
            "filtering" : { "type" : "boolean" },
            "atlas" : { "$ref": "#/common_definitions/address" },
            "sprite" : { "$ref": "#/common_definitions/address" }
        }
    })json";

    bool factory_loader<sprite_9p_renderer>::operator()(
        sprite_9p_renderer& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("tint") ) {
            auto tint = component.tint();
            if ( !json_utils::try_parse_value(ctx.root["tint"], tint) ) {
                the<debug>().error("SPRITE_9P_RENDERER: Incorrect formatting of 'tint' property");
                return false;
            }
            component.tint(tint);
        }

        if ( ctx.root.HasMember("filtering") ) {
            auto filtering = component.filtering();
            if ( !json_utils::try_parse_value(ctx.root["filtering"], filtering) ) {
                the<debug>().error("SPRITE_9P_RENDERER: Incorrect formatting of 'filtering' property");
                return false;
            }
            component.filtering(filtering);
        }

        if ( ctx.root.HasMember("atlas") ) {
            auto sprite = ctx.dependencies.find_asset<atlas_asset, sprite_9p_asset>(
                path::combine(ctx.parent_address, ctx.root["atlas"].GetString()));
            if ( !sprite ) {
                the<debug>().error("SPRITE_9P_RENDERER: Dependency 'atlas' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["atlas"].GetString());
                return false;
            }
            component.sprite(sprite);
        }

        if ( ctx.root.HasMember("sprite") ) {
            auto sprite = ctx.dependencies.find_asset<sprite_9p_asset>(
                path::combine(ctx.parent_address, ctx.root["sprite"].GetString()));
            if ( !sprite ) {
                the<debug>().error("SPRITE_9P_RENDERER: Dependency 'sprite' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["sprite"].GetString());
                return false;
            }
            component.sprite(sprite);
        }

        return true;
    }

    bool factory_loader<sprite_9p_renderer>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        if ( ctx.root.HasMember("atlas") ) {
            dependencies.add_dependency<atlas_asset, sprite_9p_asset>(
                path::combine(ctx.parent_address, ctx.root["atlas"].GetString()));
        }
        
        if ( ctx.root.HasMember("sprite") ) {
            dependencies.add_dependency<sprite_9p_asset>(
                path::combine(ctx.parent_address, ctx.root["sprite"].GetString()));
        }

        return true;
    }
}

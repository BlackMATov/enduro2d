/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/flipbook_player.hpp>

namespace e2d
{
    const char* factory_loader<flipbook_player>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "time" : { "type" : "number" },
            "speed" : { "type" : "number" },
            "looped" : { "type" : "boolean" },
            "stopped" : { "type" : "boolean" },
            "playing" : { "type" : "boolean" },
            "sequence" : { "$ref": "#/common_definitions/name" },
            "flipbook" : { "$ref": "#/common_definitions/address" }
        }
    })json";

    bool factory_loader<flipbook_player>::operator()(
        flipbook_player& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("time") ) {
            f32 time = component.time();
            if ( !json_utils::try_parse_value(ctx.root["time"], time) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'time' property");
                return false;
            }
            component.time(time);
        }

        if ( ctx.root.HasMember("speed") ) {
            f32 speed = component.speed();
            if ( !json_utils::try_parse_value(ctx.root["speed"], speed) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'speed' property");
                return false;
            }
            component.speed(speed);
        }

        if ( ctx.root.HasMember("looped") ) {
            bool looped = component.looped();
            if ( !json_utils::try_parse_value(ctx.root["looped"], looped) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'looped' property");
                return false;
            }
            component.looped(looped);
        }

        if ( ctx.root.HasMember("stopped") ) {
            bool stopped = component.stopped();
            if ( !json_utils::try_parse_value(ctx.root["stopped"], stopped) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'stopped' property");
                return false;
            }
            component.stopped(stopped);
        }

        if ( ctx.root.HasMember("playing") ) {
            bool playing = component.playing();
            if ( !json_utils::try_parse_value(ctx.root["playing"], playing) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'playing' property");
                return false;
            }
            component.playing(playing);
        }

        if ( ctx.root.HasMember("sequence") ) {
            str_hash sequence = component.sequence();
            if ( !json_utils::try_parse_value(ctx.root["sequence"], sequence) ) {
                the<debug>().error("FLIPBOOK_PLAYER: Incorrect formatting of 'sequence' property");
                return false;
            }
            component.sequence(sequence);
        }

        if ( ctx.root.HasMember("flipbook") ) {
            auto flipbook = ctx.dependencies.find_asset<flipbook_asset>(
                path::combine(ctx.parent_address, ctx.root["flipbook"].GetString()));
            if ( !flipbook ) {
                the<debug>().error("FLIPBOOK_PLAYER: Dependency 'flipbook' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["flipbook"].GetString());
                return false;
            }
            component.flipbook(flipbook);
        }

        return true;
    }

    bool factory_loader<flipbook_player>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        if ( ctx.root.HasMember("flipbook") ) {
            dependencies.add_dependency<flipbook_asset>(
                path::combine(ctx.parent_address, ctx.root["flipbook"].GetString()));
        }

        return true;
    }
}

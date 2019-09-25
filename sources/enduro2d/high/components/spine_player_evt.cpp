/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/spine_player_evt.hpp>

namespace
{
    using namespace e2d;

    std::optional<spine_player_evt::custom_evt> parse_custom_evt(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        str_hash evt_name;
        E2D_ASSERT(root.HasMember("name"));
        if ( !json_utils::try_parse_value(root["name"], evt_name) ) {
            the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'custom_evt.name' property");
            return std::nullopt;
        }

        spine_player_evt::custom_evt evt(evt_name);

        if ( root.HasMember("int_value") ) {
            i32 evt_int_value = evt.int_value();
            if ( !json_utils::try_parse_value(root["int_value"], evt_int_value) ) {
                the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'custom_evt.int_value' property");
                return std::nullopt;
            }
            evt.int_value(evt_int_value);
        }

        if ( root.HasMember("float_value") ) {
            f32 evt_float_value = evt.float_value();
            if ( !json_utils::try_parse_value(root["float_value"], evt_float_value) ) {
                the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'custom_evt.float_value' property");
                return std::nullopt;
            }
            evt.float_value(evt_float_value);
        }

        if ( root.HasMember("string_value") ) {
            str evt_string_value = evt.string_value();
            if ( !json_utils::try_parse_value(root["string_value"], evt_string_value) ) {
                the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'custom_evt.string_value' property");
                return std::nullopt;
            }
            evt.string_value(evt_string_value);
        }

        return evt;
    }

    std::optional<spine_player_evt::end_evt> parse_end_evt(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        str evt_message;
        E2D_ASSERT(root.HasMember("message"));
        if ( !json_utils::try_parse_value(root["message"], evt_message) ) {
            the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'end_evt.message' property");
            return std::nullopt;
        }

        return spine_player_evt::end_evt(evt_message);
    }

    std::optional<spine_player_evt::complete_evt> parse_complete_evt(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        str evt_message;
        E2D_ASSERT(root.HasMember("message"));
        if ( !json_utils::try_parse_value(root["message"], evt_message) ) {
            the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'complete_evt.message' property");
            return std::nullopt;
        }

        return spine_player_evt::complete_evt(evt_message);
    }

    std::optional<spine_player_evt::event> parse_event(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        str_hash event_type;
        E2D_ASSERT(root.HasMember("type") && root["type"].IsString());
        if ( !json_utils::try_parse_value(root["type"], event_type) ) {
            the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'event.type' property");
            return std::nullopt;
        }

        E2D_ASSERT(root.HasMember("desc") && root["desc"].IsObject());
        const auto& event_desc = root["desc"];

        if ( event_type == make_hash("custom_evt") ) {
            auto evt = parse_custom_evt(event_desc);
            return evt ? evt : std::nullopt;
        } else if ( event_type == make_hash("end_evt") ) {
            auto evt = parse_end_evt(event_desc);
            return evt ? evt : std::nullopt;
        } else if ( event_type == make_hash("complete_evt") ) {
            auto evt = parse_complete_evt(event_desc);
            return evt ? evt : std::nullopt;
        } else {
            the<debug>().error("SPINE_PLAYER_EVT: Incorrect formatting of 'event.type' property");
            return std::nullopt;
        }
    }

    std::optional<vector<spine_player_evt::event>> parse_events(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsArray());

        vector<spine_player_evt::event> evts;
        evts.reserve(root.Size());

        for ( rapidjson::SizeType i = 0; i < root.Size(); ++i ) {
            auto evt = parse_event(root[i]);
            if ( !evt ) {
                return std::nullopt;
            }
            evts.push_back(std::move(evt.value()));
        }

        return evts;
    }
}

namespace e2d
{
    const char* factory_loader<spine_player_evt>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "events" : { "$ref": "#/definitions/events" }
        },
        "definitions" : {
            "events" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/event" }
            },
            "event" : {
                "type" : "object",
                "required" : [ "type", "desc" ],
                "additionalProperties" : false,
                "properties" : {
                    "type" : { "$ref": "#/common_definitions/name" },
                    "desc" : {
                        "anyOf" : [{
                            "$ref" : "#/definitions/custom_evt"
                        },{
                            "$ref" : "#/definitions/end_evt"
                        },{
                            "$ref" : "#/definitions/complete_evt"
                        }]
                    }
                }
            },
            "custom_evt" : {
                "type" : "object",
                "required" : [ "name" ],
                "additionalProperties" : false,
                "properties" : {
                    "name" : { "$ref": "#/common_definitions/name" },
                    "int_value" : { "type" : "integer" },
                    "float_value" : { "type" : "number" },
                    "string_value" : { "type" : "string" }
                }
            },
            "end_evt" : {
                "type" : "object",
                "required" : [ "message" ],
                "additionalProperties" : false,
                "properties" : {
                    "message" : { "$ref": "#/common_definitions/name" }
                }
            },
            "complete_evt" : {
                "type" : "object",
                "required" : [ "message" ],
                "additionalProperties" : false,
                "properties" : {
                    "message" : { "$ref": "#/common_definitions/name" }
                }
            }
        }
    })json";

    bool factory_loader<spine_player_evt>::operator()(
        spine_player_evt& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("events") ) {
            auto evts = parse_events(ctx.root["events"]);
            if ( !evts ) {
                return false;
            }
            for ( auto& evt : evts.value() ) {
                if ( evt.valueless_by_exception() ) {
                    return false;
                }
                component.add_event(std::move(evt));
            }
        }

        return true;
    }

    bool factory_loader<spine_player_evt>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

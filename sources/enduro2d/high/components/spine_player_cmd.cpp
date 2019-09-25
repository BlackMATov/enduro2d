/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/spine_player_cmd.hpp>

namespace
{
    using namespace e2d;

    std::optional<spine_player_cmd::clear_track_cmd> parse_clear_track_cmd(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        u32 cmd_track{0u};
        if ( !json_utils::try_parse_value(root["track"], cmd_track) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'clear_track_cmd.track' property");
            return std::nullopt;
        }

        return spine_player_cmd::clear_track_cmd(cmd_track);
    }

    std::optional<spine_player_cmd::set_anim_cmd> parse_set_anim_cmd(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        u32 cmd_track{0u};
        E2D_ASSERT(root.HasMember("track"));
        if ( !json_utils::try_parse_value(root["track"], cmd_track) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_anim_cmd.track' property");
            return std::nullopt;
        }

        str cmd_name;
        E2D_ASSERT(root.HasMember("name"));
        if ( !json_utils::try_parse_value(root["name"], cmd_name) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_anim_cmd.name' property");
            return std::nullopt;
        }

        spine_player_cmd::set_anim_cmd cmd(cmd_track, std::move(cmd_name));

        if ( root.HasMember("loop") ) {
            bool cmd_loop = cmd.loop();
            if ( !json_utils::try_parse_value(root["loop"], cmd_loop) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_anim_cmd.loop' property");
                return std::nullopt;
            }
            cmd.loop(cmd_loop);
        }

        if ( root.HasMember("end_message") ) {
            str cmd_end_message = cmd.end_message();
            if ( !json_utils::try_parse_value(root["end_message"], cmd_end_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_anim_cmd.end_message' property");
                return std::nullopt;
            }
            cmd.end_message(std::move(cmd_end_message));
        }

        if ( root.HasMember("complete_message") ) {
            str cmd_complete_message = cmd.complete_message();
            if ( !json_utils::try_parse_value(root["complete_message"], cmd_complete_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_anim_cmd.complete_message' property");
                return std::nullopt;
            }
            cmd.complete_message(std::move(cmd_complete_message));
        }

        return cmd;
    }

    std::optional<spine_player_cmd::add_anim_cmd> parse_add_anim_cmd(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        u32 cmd_track{0u};
        E2D_ASSERT(root.HasMember("track"));
        if ( !json_utils::try_parse_value(root["track"], cmd_track) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.track' property");
            return std::nullopt;
        }

        str cmd_name;
        E2D_ASSERT(root.HasMember("name"));
        if ( !json_utils::try_parse_value(root["name"], cmd_name) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.name' property");
            return std::nullopt;
        }

        spine_player_cmd::add_anim_cmd cmd(cmd_track, std::move(cmd_name));

        if ( root.HasMember("loop") ) {
            bool cmd_loop = cmd.loop();
            if ( !json_utils::try_parse_value(root["loop"], cmd_loop) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.loop' property");
                return std::nullopt;
            }
            cmd.loop(cmd_loop);
        }

        if ( root.HasMember("delay") ) {
            secf cmd_delay = cmd.delay();
            if ( !json_utils::try_parse_value(root["delay"], cmd_delay) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.delay' property");
                return std::nullopt;
            }
            cmd.delay(cmd_delay);
        }

        if ( root.HasMember("end_message") ) {
            str cmd_end_message = cmd.end_message();
            if ( !json_utils::try_parse_value(root["end_message"], cmd_end_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.end_message' property");
                return std::nullopt;
            }
            cmd.end_message(std::move(cmd_end_message));
        }

        if ( root.HasMember("complete_message") ) {
            str cmd_complete_message = cmd.complete_message();
            if ( !json_utils::try_parse_value(root["complete_message"], cmd_complete_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_anim_cmd.complete_message' property");
                return std::nullopt;
            }
            cmd.complete_message(std::move(cmd_complete_message));
        }

        return cmd;
    }

    std::optional<spine_player_cmd::set_empty_anim_cmd> parse_set_empty_anim_cmd(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        u32 cmd_track{0u};
        E2D_ASSERT(root.HasMember("track"));
        if ( !json_utils::try_parse_value(root["track"], cmd_track) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_empty_anim_cmd.track' property");
            return std::nullopt;
        }

        spine_player_cmd::set_empty_anim_cmd cmd(cmd_track);

        if ( root.HasMember("mix_duration") ) {
            secf cmd_mix_duration = cmd.mix_duration();
            if ( !json_utils::try_parse_value(root["mix_duration"], cmd_mix_duration) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_empty_anim_cmd.mix_duration' property");
                return std::nullopt;
            }
            cmd.mix_duration(cmd_mix_duration);
        }

        if ( root.HasMember("end_message") ) {
            str cmd_end_message = cmd.end_message();
            if ( !json_utils::try_parse_value(root["end_message"], cmd_end_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_empty_anim_cmd.end_message' property");
                return std::nullopt;
            }
            cmd.end_message(std::move(cmd_end_message));
        }

        if ( root.HasMember("complete_message") ) {
            str cmd_complete_message = cmd.complete_message();
            if ( !json_utils::try_parse_value(root["complete_message"], cmd_complete_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'set_empty_anim_cmd.complete_message' property");
                return std::nullopt;
            }
            cmd.complete_message(std::move(cmd_complete_message));
        }

        return cmd;
    }

    std::optional<spine_player_cmd::add_empty_anim_cmd> parse_add_empty_anim_cmd(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        u32 cmd_track{0u};
        E2D_ASSERT(root.HasMember("track"));
        if ( !json_utils::try_parse_value(root["track"], cmd_track) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_empty_anim_cmd.track' property");
            return std::nullopt;
        }

        spine_player_cmd::add_empty_anim_cmd cmd(cmd_track);

        if ( root.HasMember("delay") ) {
            secf cmd_delay = cmd.delay();
            if ( !json_utils::try_parse_value(root["delay"], cmd_delay) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_empty_anim_cmd.delay' property");
                return std::nullopt;
            }
            cmd.delay(cmd_delay);
        }

        if ( root.HasMember("mix_duration") ) {
            secf cmd_mix_duration = cmd.mix_duration();
            if ( !json_utils::try_parse_value(root["mix_duration"], cmd_mix_duration) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_empty_anim_cmd.mix_duration' property");
                return std::nullopt;
            }
            cmd.mix_duration(cmd_mix_duration);
        }

        if ( root.HasMember("end_message") ) {
            str cmd_end_message = cmd.end_message();
            if ( !json_utils::try_parse_value(root["end_message"], cmd_end_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_empty_anim_cmd.end_message' property");
                return std::nullopt;
            }
            cmd.end_message(std::move(cmd_end_message));
        }

        if ( root.HasMember("complete_message") ) {
            str cmd_complete_message = cmd.complete_message();
            if ( !json_utils::try_parse_value(root["complete_message"], cmd_complete_message) ) {
                the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'add_empty_anim_cmd.complete_message' property");
                return std::nullopt;
            }
            cmd.complete_message(std::move(cmd_complete_message));
        }

        return cmd;
    }

    std::optional<spine_player_cmd::command> parse_command(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsObject());

        str_hash command_type;
        E2D_ASSERT(root.HasMember("type") && root["type"].IsString());
        if ( !json_utils::try_parse_value(root["type"], command_type) ) {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'command.type' property");
            return std::nullopt;
        }

        E2D_ASSERT(root.HasMember("desc") && root["desc"].IsObject());
        const auto& command_desc = root["desc"];

        if ( command_type == make_hash("clear_track_cmd") ) {
            auto cmd = parse_clear_track_cmd(command_desc);
            return cmd ? cmd : std::nullopt;
        } else if ( command_type == make_hash("set_anim_cmd") ) {
            auto cmd = parse_set_anim_cmd(command_desc);
            return cmd ? cmd : std::nullopt;
        } else if ( command_type == make_hash("add_anim_cmd") ) {
            auto cmd = parse_add_anim_cmd(command_desc);
            return cmd ? cmd : std::nullopt;
        } else if ( command_type == make_hash("set_empty_anim_cmd") ) {
            auto cmd = parse_set_empty_anim_cmd(command_desc);
            return cmd ? cmd : std::nullopt;
        } else if ( command_type == make_hash("add_empty_anim_cmd") ) {
            auto cmd = parse_add_empty_anim_cmd(command_desc);
            return cmd ? cmd : std::nullopt;
        } else {
            the<debug>().error("SPINE_PLAYER_CMD: Incorrect formatting of 'command.type' property");
            return std::nullopt;
        }
    }

    std::optional<vector<spine_player_cmd::command>> parse_commands(const rapidjson::Value& root) {
        E2D_ASSERT(root.IsArray());

        vector<spine_player_cmd::command> cmds;
        cmds.reserve(root.Size());

        for ( rapidjson::SizeType i = 0; i < root.Size(); ++i ) {
            auto cmd = parse_command(root[i]);
            if ( !cmd ) {
                return std::nullopt;
            }
            cmds.push_back(std::move(cmd.value()));
        }

        return cmds;
    }
}

namespace e2d
{
    const char* factory_loader<spine_player_cmd>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "commands" : { "$ref": "#/definitions/commands" }
        },
        "definitions" : {
            "commands" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/command" }
            },
            "command" : {
                "type" : "object",
                "required" : [ "type", "desc" ],
                "additionalProperties" : false,
                "properties" : {
                    "type" : { "$ref": "#/common_definitions/name" },
                    "desc" : {
                        "anyOf" : [{
                            "$ref" : "#/definitions/clear_track_cmd"
                        }, {
                            "$ref" : "#/definitions/set_anim_cmd"
                        }, {
                            "$ref" : "#/definitions/add_anim_cmd"
                        }, {
                            "$ref" : "#/definitions/set_empty_anim_cmd"
                        }, {
                            "$ref" : "#/definitions/add_empty_anim_cmd"
                        }]
                    }
                }
            },
            "clear_track_cmd" : {
                "type" : "object",
                "required" : [ "track" ],
                "additionalProperties" : false,
                "properties" : {
                    "track" : { "type" : "integer", "minimum" : 0 }
                }
            },
            "set_anim_cmd" : {
                "type" : "object",
                "required" : [ "track", "name" ],
                "additionalProperties" : false,
                "properties" : {
                    "track" : { "type" : "integer", "minimum" : 0 },
                    "name" : { "$ref": "#/common_definitions/name" },
                    "loop" : { "type" : "boolean" },
                    "end_message" : { "type" : "string" },
                    "complete_message" : { "type" : "string" }
                }
            },
            "add_anim_cmd" : {
                "type" : "object",
                "required" : [ "track", "name" ],
                "additionalProperties" : false,
                "properties" : {
                    "track" : { "type" : "integer", "minimum" : 0 },
                    "name" : { "$ref": "#/common_definitions/name" },
                    "loop" : { "type" : "boolean" },
                    "delay" : { "type" : "number" },
                    "end_message" : { "type" : "string" },
                    "complete_message" : { "type" : "string" }
                }
            },
            "set_empty_anim_cmd" : {
                "type" : "object",
                "required" : [ "track" ],
                "additionalProperties" : false,
                "properties" : {
                    "track" : { "type" : "integer", "minimum" : 0 },
                    "mix_duration" : { "type" : "number" },
                    "end_message" : { "type" : "string" },
                    "complete_message" : { "type" : "string" }
                }
            },
            "add_empty_anim_cmd" : {
                "type" : "object",
                "required" : [ "track" ],
                "additionalProperties" : false,
                "properties" : {
                    "track" : { "type" : "integer", "minimum" : 0 },
                    "delay" : { "type" : "number" },
                    "mix_duration" : { "type" : "number" },
                    "end_message" : { "type" : "string" },
                    "complete_message" : { "type" : "string" }
                }
            }
        }
    })json";

    bool factory_loader<spine_player_cmd>::operator()(
        spine_player_cmd& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("commands") ) {
            auto cmds = parse_commands(ctx.root["commands"]);
            if ( !cmds ) {
                return false;
            }
            for ( auto& cmd : cmds.value() ) {
                if ( cmd.valueless_by_exception() ) {
                    return false;
                }
                component.add_command(std::move(cmd));
            }
        }

        return true;
    }

    bool factory_loader<spine_player_cmd>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

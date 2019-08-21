/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/render_technique.hpp>
#include <enduro2d/high/assets/cbuffer_template_asset.hpp>

namespace
{
    using namespace e2d;

    bool parse_render_pass(
        const rapidjson::Value& root,
        str_view parent_address,
        const asset_group& dependencies,
        const render::property_map& shared_props,
        render_technique::pass& content)
    {
        if ( !json_utils::try_parse_value(root, content.desc) ) {
            return false;
        }

        if ( root.HasMember("constants") ) {
            if ( !json_utils::try_parse_value(root["constants"], content.properties) ) {
                return false;
            }

            shared_props.foreach([&content](str_hash key, const auto& value) {
                content.properties.assign(key, value);
            });
        }

        if ( root.HasMember("block") ) {
            str address;
            if ( !json_utils::try_parse_value(root["block"], address) ) {
                return false;
            }

            address = path::combine(parent_address, address);
            auto templ_a = dependencies.find_asset<cbuffer_template_asset>(address);
            if ( !templ_a ) {
                return false;
            }

            content.templ = templ_a->content();
        }
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<render_technique>::schema_source = R"json({
        "type" : "object",
        "required" : [ "model" ],
        "additionalProperties" : false,
        "properties" : {
            "constants" : { $ref : "#/render_definitions/property_map" },
            "passes" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/render_pass" }
            }
        },
        "definitions" : {
            "render_pass" : {
                "type" : "object",
                "required" : [],
                "additionalProperties" : false,
                "properties" : {
                    "block" : { "$ref": "#/common_definitions/address" },
                    "constants" : { $ref : "#/render_definitions/property_map" },

                    "depth_range" : {
                        "type" : "object",
                        "additionalProperties" : false,
                        "properties" : {
                            "near" : { "type" : "number" },
                            "far" : { "type" : "number" }
                        }
                    },
                    "state_block" : { "$ref": "#/render_definitions/state_block" },

                    "color_clear_value" : { "$ref" : "#/common_definitions/color" },
                    "color_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                    "color_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" },

                    "depth_clear_value" : { "type" : "number" },
                    "depth_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                    "depth_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" },

                    "stencil_clear_value" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                    "stencil_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                    "stencil_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" }
                }
            }
        }
    })json";

    bool factory_loader<render_technique>::operator()(
        render_technique& component,
        const fill_context& ctx) const
    {
        render::property_map props;
        if ( ctx.root.HasMember("constants") ) {
            if ( !json_utils::try_parse_value(ctx.root["constants"], props) ) {
                return false;
            }
        }

        if ( ctx.root.HasMember("passes") ) {
            E2D_ASSERT(ctx.root["passes"].IsArray());
            const auto& passes_json = ctx.root["passes"];

            for ( rapidjson::SizeType i = 0; i < passes_json.Size(); ++i ) {
                E2D_ASSERT(passes_json[i].IsObject());
                const auto& pass_json = passes_json[i];
                render_technique::pass pass;
                if ( !parse_render_pass(pass_json, ctx.parent_address, ctx.dependencies, props, pass) ) {
                    return false;
                }
                component.add_pass(pass);
            }
        }

        return true;
    }

    bool factory_loader<render_technique>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

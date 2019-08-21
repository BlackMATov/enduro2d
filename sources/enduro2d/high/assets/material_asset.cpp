/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/material_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/assets/shader_asset.hpp>
#include <enduro2d/high/assets/texture_asset.hpp>

namespace
{
    using namespace e2d;

    class material_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "material asset loading exception";
        }
    };

    const char* material_asset_schema_source = R"json({
        "type" : "object",
        "additionalProperties" : false,
        "properties" : {
            "constants" : {
                "type" : "array",
                "items" : { "$ref" : "#/render_definitions/property" }
            },
            "samplers" : {
                "type" : "array",
                "items" : { "$ref" : "#/render_definitions/sampler" }
            },
            "shader" : { "$ref": "#/common_definitions/address" },
            "blending_state" : { "$ref": "#/render_definitions/blending_state" },
            "depth_state" : { "$ref": "#/render_definitions/depth_dynamic_state" },
            "culling_state" : { "$ref": "#/render_definitions/culling_state" }
        },
        "definitions" : {
            "sampler" : {
                "type" : "object",
                "required" : [ "name" ],
                "additionalProperties" : false,
                "properties" : {
                    "name" : {
                        "$ref" : "#/common_definitions/name"
                    },
                    "texture" : {
                        "$ref" : "#/common_definitions/address"
                    },
                    "wrap" : {
                        "anyOf" : [{
                            "type" : "object",
                            "additionalProperties" : false,
                            "properties" : {
                                "s" : { "$ref" : "#/render_definitions/sampler_wrap" },
                                "t" : { "$ref" : "#/render_definitions/sampler_wrap" }
                            }
                        }, {
                            "$ref" : "#/render_definitions/sampler_wrap"
                        }]
                    },
                    "filter" : {
                        "anyOf" : [{
                            "type" : "object",
                            "additionalProperties" : false,
                            "properties" : {
                                "min" : { "$ref" : "#/render_definitions/sampler_filter" },
                                "mag" : { "$ref" : "#/render_definitions/sampler_filter" }
                            }
                        }, {
                            "$ref" : "#/render_definitions/sampler_filter"
                        }]
                    }
                }
            }
        }
    })json";

    const rapidjson::SchemaDocument& material_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(material_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse material asset schema");
                throw material_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            json_utils::add_render_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }
    
    stdex::promise<texture_ptr> parse_texture_block(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        E2D_ASSERT(root.IsString());
        const auto texture_address =
            path::combine(parent_address, root.GetString());
        return library.load_asset_async<texture_asset>(texture_address)
            .then([](const texture_asset::load_result& texture){
                return texture->content();
            });
    }

    stdex::promise<std::pair<str_hash,render::sampler_state>> parse_sampler_state(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        render::sampler_state content;

        E2D_ASSERT(root.HasMember("name") && root["name"].IsString());
        auto name_hash = make_hash(root["name"].GetString());

        auto texture_p = root.HasMember("texture")
            ? parse_texture_block(library, parent_address, root["texture"])
            : stdex::make_resolved_promise<texture_ptr>(nullptr);

        if ( root.HasMember("wrap") ) {
            const auto& wrap_json = root["wrap"];
            if ( wrap_json.IsObject() ) {
                if ( wrap_json.HasMember("s") ) {
                    auto wrap = content.s_wrap();
                    if ( json_utils::try_parse_value(wrap_json["s"], wrap) ) {
                        content.s_wrap(wrap);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                    }
                }

                if ( wrap_json.HasMember("t") ) {
                    auto wrap = content.t_wrap();
                    if ( json_utils::try_parse_value(wrap_json["t"], wrap) ) {
                        content.t_wrap(wrap);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                    }
                }
            } else if ( wrap_json.IsString() ) {
                auto wrap = content.s_wrap();
                if ( json_utils::try_parse_value(wrap_json, wrap) ) {
                    content.wrap(wrap);
                } else {
                    E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                }
            }
        }

        if ( root.HasMember("filter") ) {
            const auto& filter_json = root["filter"];
            if ( filter_json.IsObject() ) {
                if ( filter_json.HasMember("min") ) {
                    auto filter = content.min_filter();
                    if ( json_utils::try_parse_value(filter_json["min"], filter) ) {
                        content.min_filter(filter);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler min filter");
                    }
                }

                if ( filter_json.HasMember("mag") ) {
                    auto filter = content.mag_filter();
                    if ( json_utils::try_parse_value(filter_json["mag"], filter) ) {
                        content.mag_filter(filter);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler mag filter");
                    }
                }
            } else if ( filter_json.IsString() ) {
                auto min_filter = content.min_filter();
                if ( json_utils::try_parse_value(filter_json, min_filter) ) {
                    content.min_filter(min_filter);
                } else {
                    E2D_ASSERT_MSG(false, "unexpected sampler filter");
                }

                auto mag_filter = content.mag_filter();
                if ( json_utils::try_parse_value(filter_json, mag_filter) ) {
                    content.mag_filter(mag_filter);
                } else {
                    E2D_ASSERT_MSG(false, "unexpected sampler filter");
                }
            }
        }

        return texture_p
            .then([name_hash, content](const texture_ptr& texture) mutable {
                content.texture(texture);
                return std::make_pair(name_hash, content);
            });
    }

    render::culling_state_opt parse_culling_state(const rapidjson::Value& root) {
        render::culling_state state;
        if ( json_utils::try_parse_value(root, state) ) {
            return state;
        } else {
            return std::nullopt;
        }
    }

    render::blending_state_opt parse_blending_state(const rapidjson::Value& root) {
        render::blending_state state;
        if ( json_utils::try_parse_value(root, state) ) {
            return state;
        } else {
            return std::nullopt;
        }
    }

    render::depth_dynamic_state_opt parse_depth_dynamic_state(const rapidjson::Value& root) {
        render::depth_dynamic_state state;
        if ( json_utils::try_parse_value(root, state) ) {
            return state;
        } else {
            return std::nullopt;
        }
    }

    stdex::promise<render::sampler_block> parse_sampler_block(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        vector<stdex::promise<std::pair<str_hash, render::sampler_state>>> samplers_p;

        if ( root.HasMember("samplers") ) {
            E2D_ASSERT(root["samplers"].IsArray());
            const auto& samplers_json = root["samplers"];

            samplers_p.reserve(samplers_json.Size());
            for ( rapidjson::SizeType i = 0; i < samplers_json.Size(); ++i ) {
                E2D_ASSERT(samplers_json[i].IsObject());
                const auto& sampler_json = samplers_json[i];
                samplers_p.emplace_back(
                    parse_sampler_state(library, parent_address, sampler_json));
            }
        }
        return stdex::make_all_promise(samplers_p)
            .then([](auto&& results) {
                render::sampler_block content;
                for ( auto& result : results ) {
                    content.bind(
                        std::move(result.first),
                        std::move(result.second));
                }
                return content;
            });
    }

    stdex::promise<shader_ptr> parse_shader_block(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        E2D_ASSERT(root.IsString());
        const auto shader_address =
            path::combine(parent_address, root.GetString());
        return library.load_asset_async<shader_asset>(shader_address)
            .then([](const shader_asset::load_result& shader){
                return shader->content();
            });
    }

    stdex::promise<const_buffer_ptr> create_const_buffer(
        const rapidjson::Value& root,
        stdex::promise<shader_ptr>& shader_p)
    {
        render::property_map props;
        if ( !json_utils::try_parse_value(root, props) ) {
            return stdex::make_rejected_promise<const_buffer_ptr>(material_asset_loading_exception());
        }

        return shader_p.then([props = std::move(props)](const shader_ptr& shader) {
            return the<deferrer>().do_in_main_thread([props = std::move(props), shader]() {
                auto content = the<render>().create_const_buffer(shader, const_buffer::scope::material);
                the<render>().update_buffer(content, props);
                return content;
            });
        });
    }

    stdex::promise<render::material> parse_material(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        auto shader_p = root.HasMember("shader")
            ? parse_shader_block(library, parent_address, root["shader"])
            : stdex::make_resolved_promise<shader_ptr>(nullptr);

        auto constants_p = root.HasMember("constants")
            ? create_const_buffer(root["constants"], shader_p)
            : stdex::make_resolved_promise<const_buffer_ptr>(nullptr);

        auto samplers_p = parse_sampler_block(library, parent_address, root);

        auto blending = root.HasMember("blending_state")
            ? parse_blending_state(root["blending_state"])
            : std::nullopt;
        
        auto depth = root.HasMember("depth_state")
            ? parse_depth_dynamic_state(root["depth_state"])
            : std::nullopt;

        auto culling = root.HasMember("culling_state")
            ? parse_culling_state(root["culling_state"])
            : std::nullopt;
        
        return stdex::make_tuple_promise(std::make_tuple(
            shader_p, constants_p, samplers_p))
            .then([blending, depth, culling](const std::tuple<
                shader_ptr,
                const_buffer_ptr,
                render::sampler_block
            >& results) {
                render::material content;
                content.shader(std::get<0>(results));
                content.constants(std::get<1>(results));
                content.samplers(std::get<2>(results));
                if ( blending ) {
                    content.blending(*blending);
                }
                if ( depth ) {
                    content.depth(*depth);
                }
                if ( culling ) {
                    content.culling(*culling);
                }
                return content;
            });
    }
}

namespace e2d
{
    material_asset::load_async_result material_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& material_data){
            return the<deferrer>().do_in_worker_thread([address, material_data](){
                const rapidjson::Document& doc = *material_data->content();
                rapidjson::SchemaValidator validator(material_asset_schema());

                if ( doc.Accept(validator) ) {
                    return;
                }

                rapidjson::StringBuffer sb;
                if ( validator.GetInvalidDocumentPointer().StringifyUriFragment(sb) ) {
                    the<debug>().error("ASSET: Failed to validate asset json:\n"
                        "--> Address: %0\n"
                        "--> Invalid schema keyword: %1\n"
                        "--> Invalid document pointer: %2",
                        address,
                        validator.GetInvalidSchemaKeyword(),
                        sb.GetString());
                } else {
                    the<debug>().error("ASSET: Failed to validate asset json");
                }

                throw material_asset_loading_exception();
            })
            .then([&library, parent_address, material_data](){
                return parse_material(
                    library, parent_address, *material_data->content());
            })
            .then([](const render::material& material){
                return material_asset::create(material);
            });
        });
    }
}

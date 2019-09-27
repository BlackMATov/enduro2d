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

    const char* material_asset_schema_source = R"json(
        {
            "type" : "object",
            "additionalProperties" : false,
            "properties" : {
                "properties" : {
                    "type" : "array",
                    "items" : { "$ref" : "#/definitions/property" }
                },
                "samplers" : {
                    "type" : "array",
                    "items" : { "$ref" : "#/definitions/sampler" }
                },
                "shader" : { "$ref": "#/common_definitions/address" },
                "depth_state" : { "$ref": "#/definitions/depth_state" },
                "stencil_state" : { "$ref": "#/definitions/stencil_state" },
                "culling_state" : { "$ref": "#/definitions/culling_state" },
                "blending_state" : { "$ref": "#/definitions/blending_state" }
            },
            "definitions" : {
                "depth_state" : {
                    "type" : "object",
                    "additionalProperties" : false,
                    "properties" : {
                        "test" : { "type" : "boolean" },
                        "write" : { "type" : "boolean" },
                        "func" : { "$ref" : "#/definitions/compare_func" }
                    }
                },
                "stencil_state" : {
                    "type" : "object",
                    "additionalProperties" : false,
                    "properties" : {
                        "test" : { "type" : "boolean" },
                        "write" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                        "func" : { "$ref" : "#/definitions/compare_func" },
                        "ref" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                        "mask" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                        "pass" : { "$ref" : "#/definitions/stencil_op" },
                        "sfail" : { "$ref" : "#/definitions/stencil_op" },
                        "zfail" : { "$ref" : "#/definitions/stencil_op" }
                    }
                },
                "culling_state" : {
                    "type" : "object",
                    "additionalProperties" : false,
                    "properties" : {
                        "enable" : { "type" : "boolean" },
                        "mode" : { "$ref" : "#/definitions/culling_mode" },
                        "face" : { "$ref" : "#/definitions/culling_face" }
                    }
                },
                "blending_state" : {
                    "type" : "object",
                    "additionalProperties" : false,
                    "properties" : {
                        "enable" : { 
                            "type" : "boolean"
                        },
                        "color_mask" : {
                            "$ref" : "#/definitions/color_mask"
                        },
                        "src_factor" : {
                            "anyOf" : [{
                                "type" : "object",
                                "additionalProperties" : false,
                                "properties" : {
                                    "rgb" : { "$ref" : "#/definitions/blending_factor" },
                                    "alpha" : { "$ref" : "#/definitions/blending_factor" }
                                }
                            }, {
                                "$ref" : "#/definitions/blending_factor"
                            }]
                        },
                        "dst_factor" : {
                            "anyOf" : [{
                                "type" : "object",
                                "additionalProperties" : false,
                                "properties" : {
                                    "rgb" : { "$ref" : "#/definitions/blending_factor" },
                                    "alpha" : { "$ref" : "#/definitions/blending_factor" }
                                }
                            }, {
                                "$ref" : "#/definitions/blending_factor"
                            }]
                        },
                        "equation" : {
                            "anyOf" : [{
                                "type" : "object",
                                "additionalProperties" : false,
                                "properties" : {
                                    "rgb" : { "$ref" : "#/definitions/blending_equation" },
                                    "alpha" : { "$ref" : "#/definitions/blending_equation" }
                                }
                            }, {
                                "$ref" : "#/definitions/blending_equation"
                            }]
                        }
                    }
                },
                "sampler" : {
                    "type" : "object",
                    "required" : [ "name", "texture" ],
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
                                    "s" : { "$ref" : "#/definitions/sampler_wrap" },
                                    "t" : { "$ref" : "#/definitions/sampler_wrap" }
                                }
                            }, {
                                "$ref" : "#/definitions/sampler_wrap"
                            }]
                        },
                        "filter" : {
                            "anyOf" : [{
                                "type" : "object",
                                "additionalProperties" : false,
                                "properties" : {
                                    "min" : { "$ref" : "#/definitions/sampler_filter" },
                                    "mag" : { "$ref" : "#/definitions/sampler_filter" }
                                }
                            }, {
                                "$ref" : "#/definitions/sampler_filter"
                            }]
                        }
                    }
                },
                "property" : {
                    "type" : "object",
                    "required" : [ "name", "type", "value" ],
                    "additionalProperties" : false,
                    "properties" : {
                        "name" : { "$ref" : "#/common_definitions/name" },
                        "type" : { "$ref" : "#/definitions/property_type" },
                        "value" : { "$ref" : "#/definitions/property_value" }
                    }
                },
                "property_type" : {
                    "type" : "string",
                    "enum" : [
                        "i32", "f32",
                        "v2f", "v3f", "v4f",
                        "m2f", "m3f", "m4f"
                    ]
                },
                "property_value" : {
                    "anyOf": [
                        { "type" : "number" },
                        { "$ref" : "#/common_definitions/v2" },
                        { "$ref" : "#/common_definitions/v3" },
                        { "$ref" : "#/common_definitions/v4" },
                        { "$ref" : "#/common_definitions/m2" },
                        { "$ref" : "#/common_definitions/m3" },
                        { "$ref" : "#/common_definitions/m4" }
                    ]
                },
                "stencil_op" : {
                    "type" : "string",
                    "enum" : [
                        "keep",
                        "zero",
                        "replace",
                        "incr",
                        "incr_wrap",
                        "decr",
                        "decr_wrap",
                        "invert"
                    ]
                },
                "compare_func" : {
                    "type" : "string",
                    "enum" : [
                        "never",
                        "less",
                        "lequal",
                        "greater",
                        "gequal",
                        "equal",
                        "notequal",
                        "always"
                    ]
                },
                "culling_mode" : {
                    "type" : "string",
                    "enum" : [
                        "cw",
                        "ccw"
                    ]
                },
                "culling_face" : {
                    "type" : "string",
                    "enum" : [
                        "back",
                        "front",
                        "back_and_front"
                    ]
                },
                "blending_factor" : {
                    "type" : "string",
                    "enum" : [
                        "zero",
                        "one",
                        "src_color",
                        "one_minus_src_color",
                        "dst_color",
                        "one_minus_dst_color",
                        "src_alpha",
                        "one_minus_src_alpha",
                        "dst_alpha",
                        "one_minus_dst_alpha",
                        "constant_color",
                        "one_minus_constant_color",
                        "constant_alpha",
                        "one_minus_constant_alpha",
                        "src_alpha_saturate"
                    ]
                },
                "blending_equation" : {
                    "type" : "string",
                    "enum" : [
                        "add",
                        "subtract",
                        "reverse_subtract"
                    ]
                },
                "blending_color_mask" : {
                    "type" : "string",
                    "enum" : [
                        "none",
                        "r",
                        "g",
                        "b",
                        "a",
                        "rg",
                        "rb",
                        "ra",
                        "gb",
                        "ga",
                        "ba",
                        "rgb",
                        "rga",
                        "rba",
                        "gba",
                        "rgba"
                    ]
                },
                "sampler_wrap" : {
                    "type" : "string",
                    "enum" : [
                        "clamp",
                        "repeat",
                        "mirror"
                    ]
                },
                "sampler_filter" : {
                    "type" : "string",
                    "enum" : [
                        "nearest",
                        "linear"
                    ]
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
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }
    /*
    bool parse_stencil_op(str_view str, render::stencil_op& op) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { op = render::stencil_op::x; return true; }
        DEFINE_IF(keep);
        DEFINE_IF(zero);
        DEFINE_IF(replace);
        DEFINE_IF(incr);
        DEFINE_IF(incr_wrap);
        DEFINE_IF(decr);
        DEFINE_IF(decr_wrap);
        DEFINE_IF(invert);
    #undef DEFINE_IF
        return false;
    }

    bool parse_compare_func(str_view str, render::compare_func& func) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { func = render::compare_func::x; return true; }
        DEFINE_IF(never);
        DEFINE_IF(less);
        DEFINE_IF(lequal);
        DEFINE_IF(greater);
        DEFINE_IF(gequal);
        DEFINE_IF(equal);
        DEFINE_IF(notequal);
        DEFINE_IF(always);
    #undef DEFINE_IF
        return false;
    }*/
    
    bool parse_culling_mode(str_view str, render::culling_mode& mode) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { mode = render::culling_mode::x; return true; }
        DEFINE_IF(cw);
        DEFINE_IF(ccw);
    #undef DEFINE_IF
        return false;
    }

    bool parse_culling_face(str_view str, render::culling_face& face) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { face = render::culling_face::x; return true; }
        DEFINE_IF(back);
        DEFINE_IF(front);
        DEFINE_IF(back_and_front);
    #undef DEFINE_IF
        return false;
    }

    bool parse_blending_factor(str_view str, render::blending_factor& factor) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { factor = render::blending_factor::x; return true; }
        DEFINE_IF(zero);
        DEFINE_IF(one);
        DEFINE_IF(src_color);
        DEFINE_IF(one_minus_src_color);
        DEFINE_IF(dst_color);
        DEFINE_IF(one_minus_dst_color);
        DEFINE_IF(src_alpha);
        DEFINE_IF(one_minus_src_alpha);
        DEFINE_IF(dst_alpha);
        DEFINE_IF(one_minus_dst_alpha);
        DEFINE_IF(constant_color);
        DEFINE_IF(one_minus_constant_color);
        DEFINE_IF(constant_alpha);
        DEFINE_IF(one_minus_constant_alpha);
        DEFINE_IF(src_alpha_saturate);
    #undef DEFINE_IF
        return false;
    }

    bool parse_blending_equation(str_view str, render::blending_equation& equation) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { equation = render::blending_equation::x; return true; }
        DEFINE_IF(add);
        DEFINE_IF(subtract);
        DEFINE_IF(reverse_subtract);
    #undef DEFINE_IF
        return false;
    }

    bool parse_blending_color_mask(str_view str, render::blending_color_mask& mask) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { mask = render::blending_color_mask::x; return true; }
        DEFINE_IF(none);
        DEFINE_IF(r);
        DEFINE_IF(g);
        DEFINE_IF(b);
        DEFINE_IF(a);
        DEFINE_IF(rg);
        DEFINE_IF(rb);
        DEFINE_IF(ra);
        DEFINE_IF(gb);
        DEFINE_IF(ga);
        DEFINE_IF(ba);
        DEFINE_IF(rgb);
        DEFINE_IF(rga);
        DEFINE_IF(rba);
        DEFINE_IF(gba);
        DEFINE_IF(rgba);
    #undef DEFINE_IF
        return false;
    }
    
    bool parse_sampler_wrap(str_view str, render::sampler_wrap& wrap) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { wrap = render::sampler_wrap::x; return true; }
        DEFINE_IF(clamp);
        DEFINE_IF(repeat);
        DEFINE_IF(mirror);
    #undef DEFINE_IF
        return false;
    }

    bool parse_sampler_min_filter(str_view str, render::sampler_min_filter& filter) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { filter = render::sampler_min_filter::x; return true; }
        DEFINE_IF(nearest);
        DEFINE_IF(linear);
    #undef DEFINE_IF
        return false;
    }

    bool parse_sampler_mag_filter(str_view str, render::sampler_mag_filter& filter) noexcept {
    #define DEFINE_IF(x,y) if ( str == #x ) { filter = render::sampler_mag_filter::y; return true; }
        DEFINE_IF(nearest, nearest);
        DEFINE_IF(linear, linear);
    #undef DEFINE_IF
        return false;
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
        E2D_ASSERT(root.HasMember("texture") && root["texture"].IsString());

        auto name_hash = make_hash(root["name"].GetString());
        auto texture_p = parse_texture_block(library, parent_address, root["texture"]);

        if ( root.HasMember("wrap") ) {
            const auto& wrap_json = root["wrap"];
            if ( wrap_json.IsObject() ) {
                if ( wrap_json.HasMember("s") ) {
                    E2D_ASSERT(wrap_json["s"].IsString());
                    auto wrap = content.s_wrap();
                    if ( parse_sampler_wrap(wrap_json["s"].GetString(), wrap) ) {
                        content.s_wrap(wrap);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                    }
                }

                if ( wrap_json.HasMember("t") ) {
                    E2D_ASSERT(wrap_json["t"].IsString());
                    auto wrap = content.t_wrap();
                    if ( parse_sampler_wrap(wrap_json["t"].GetString(), wrap) ) {
                        content.t_wrap(wrap);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                    }
                }
            } else if ( wrap_json.IsString() ) {
                auto wrap = content.s_wrap();
                if ( parse_sampler_wrap(wrap_json.GetString(), wrap) ) {
                    content.wrap(wrap, wrap);
                } else {
                    E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                }
            }
        }

        if ( root.HasMember("filter") ) {
            const auto& filter_json = root["filter"];
            if ( filter_json.IsObject() ) {
                if ( filter_json.HasMember("min") ) {
                    E2D_ASSERT(filter_json["min"].IsString());
                    auto filter = content.min_filter();
                    if ( parse_sampler_min_filter(filter_json["min"].GetString(), filter) ) {
                        content.min_filter(filter);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler min filter");
                    }
                }

                if ( filter_json.HasMember("mag") ) {
                    E2D_ASSERT(filter_json["mag"].IsString());
                    auto filter = content.mag_filter();
                    if ( parse_sampler_mag_filter(filter_json["mag"].GetString(), filter) ) {
                        content.mag_filter(filter);
                    } else {
                        E2D_ASSERT_MSG(false, "unexpected sampler mag filter");
                    }
                }
            } else if ( filter_json.IsString() ) {
                auto min_filter = content.min_filter();
                if ( parse_sampler_min_filter(filter_json.GetString(), min_filter) ) {
                    content.min_filter(min_filter);
                } else {
                    E2D_ASSERT_MSG(false, "unexpected sampler filter");
                }

                auto mag_filter = content.mag_filter();
                if ( parse_sampler_mag_filter(filter_json.GetString(), mag_filter) ) {
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

    bool parse_property_map(
        const rapidjson::Value& root,
        render::property_map& props)
    {
        E2D_ASSERT(root.IsArray());
        for ( rapidjson::SizeType i = 0; i < root.Size(); ++i ) {
            E2D_ASSERT(root[i].IsObject());
            const auto& property_json = root[i];

            E2D_ASSERT(property_json.HasMember("name") && property_json["name"].IsString());
            E2D_ASSERT(property_json.HasMember("type") && property_json["type"].IsString());
            E2D_ASSERT(property_json.HasMember("value"));

        #define DEFINE_CASE(x)\
            if ( 0 == std::strcmp(property_json["type"].GetString(), #x) ) {\
                x value;\
                if ( !json_utils::try_parse_value(property_json["value"], value) ) {\
                    E2D_ASSERT_MSG(false, "unexpected property value");\
                    return false;\
                }\
                props.assign(property_json["name"].GetString(), value);\
                continue;\
            }

            DEFINE_CASE(i32);
            DEFINE_CASE(f32);

            DEFINE_CASE(v2f);
            DEFINE_CASE(v3f);
            DEFINE_CASE(v4f);

            DEFINE_CASE(m2f);
            DEFINE_CASE(m3f);
            DEFINE_CASE(m4f);
        #undef DEFINE_CASE

            E2D_ASSERT_MSG(false, "unexpected property type");
            return false;
        }
        return true;
    }
    
    stdex::promise<const_buffer_ptr> create_const_buffer(
        const rapidjson::Value& root,
        stdex::promise<shader_ptr>& shader_p)
    {
        render::property_map props;
        if ( !parse_property_map(root, props) ) {
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

    /*
    bool parse_depth_state(
        const rapidjson::Value& root,
        render::depth_state& depth)
    {
        if ( root.HasMember("range") ) {
            E2D_ASSERT(root["range"].IsObject());
            const auto& root_range = root["range"];

            if ( root_range.HasMember("near") ) {
                E2D_ASSERT(root_range["near"].IsNumber());
                depth.range(
                    root_range["near"].GetFloat(),
                    depth.range_far());
            }

            if ( root_range.HasMember("far") ) {
                E2D_ASSERT(root_range["far"].IsNumber());
                depth.range(
                    depth.range_near(),
                    root_range["far"].GetFloat());
            }
        }

        if ( root.HasMember("write") ) {
            E2D_ASSERT(root["write"].IsBool());
            depth.write(
                root["write"].GetBool());
        }

        if ( root.HasMember("func") ) {
            E2D_ASSERT(root["func"].IsString());

            render::compare_func func = depth.func();
            if ( !parse_compare_func(root["func"].GetString(), func) ) {
                E2D_ASSERT_MSG(false, "unexpected depth state func");
                return false;
            }

            depth.func(func);
        }

        return true;
    }

    bool parse_stencil_state(
        const rapidjson::Value& root,
        render::stencil_state& stencil)
    {
        if ( root.HasMember("write") ) {
            E2D_ASSERT(root["write"].IsUint() && root["write"].GetUint() <= 255);
            stencil.write(math::numeric_cast<u8>(
                root["write"].GetUint()));
        }

        if ( root.HasMember("func") ) {
            E2D_ASSERT(root["func"].IsString());

            render::compare_func func = stencil.func();
            if ( !parse_compare_func(root["func"].GetString(), func) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state func");
                return false;
            }

            stencil.func(
                func,
                stencil.ref(),
                stencil.mask());
        }

        if ( root.HasMember("ref") ) {
            E2D_ASSERT(root["ref"].IsUint() && root["ref"].GetUint() <= 255);

            stencil.func(
                stencil.func(),
                math::numeric_cast<u8>(root["ref"].GetUint()),
                stencil.mask());
        }

        if ( root.HasMember("mask") ) {
            E2D_ASSERT(root["mask"].IsUint() && root["mask"].GetUint() <= 255);

            stencil.func(
                stencil.func(),
                stencil.ref(),
                math::numeric_cast<u8>(root["mask"].GetUint()));
        }

        if ( root.HasMember("pass") ) {
            E2D_ASSERT(root["pass"].IsString());

            render::stencil_op op = stencil.pass();
            if ( !parse_stencil_op(root["pass"].GetString(), op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state pass");
                return false;
            }

            stencil.op(
                op,
                stencil.sfail(),
                stencil.zfail());
        }

        if ( root.HasMember("sfail") ) {
            E2D_ASSERT(root["sfail"].IsString());

            render::stencil_op op = stencil.sfail();
            if ( !parse_stencil_op(root["sfail"].GetString(), op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state sfail");
                return false;
            }

            stencil.op(
                stencil.pass(),
                op,
                stencil.zfail());
        }

        if ( root.HasMember("zfail") ) {
            E2D_ASSERT(root["zfail"].IsString());

            render::stencil_op op = stencil.zfail();
            if ( !parse_stencil_op(root["zfail"].GetString(), op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state zfail");
                return false;
            }

            stencil.op(
                stencil.pass(),
                stencil.sfail(),
                op);
        }

        return true;
    }*/

    stdex::promise<render::culling_state_opt> parse_culling_state(const rapidjson::Value& root) {
        render::culling_state culling;

        if ( root.HasMember("enable") ) {
            E2D_ASSERT(root["enable"].IsBool());
            culling.enable(root["enable"].GetBool());
        }

        if ( root.HasMember("mode") ) {
            E2D_ASSERT(root["mode"].IsString());

            render::culling_mode mode = culling.mode();
            if ( !parse_culling_mode(root["mode"].GetString(), mode) ) {
                E2D_ASSERT_MSG(false, "unexpected culling state mode");
                return stdex::make_rejected_promise<render::culling_state_opt>(
                    material_asset_loading_exception());
            }

            culling.mode(mode);
        }

        if ( culling.enabled() && root.HasMember("face") ) {
            E2D_ASSERT(root["face"].IsString());

            render::culling_face face = culling.face();
            if ( !parse_culling_face(root["face"].GetString(), face) ) {
                E2D_ASSERT_MSG(false, "unexpected culling state face");
                return stdex::make_rejected_promise<render::culling_state_opt>(
                    material_asset_loading_exception());
            }

            culling.face(face);
        }

        return stdex::make_resolved_promise(
            render::culling_state_opt(culling));
    }
    
    stdex::promise<render::blending_state_opt> parse_blending_state(const rapidjson::Value& root)
    {
        E2D_ASSERT(root.IsObject());

        render::blending_state blending;

        if ( root.HasMember("enable") ) {
            E2D_ASSERT(root["enable"].IsBool());
            blending.enable(root["enable"].GetBool());
        }

        if ( root.HasMember("color_mask") ) {
            E2D_ASSERT(root["color_mask"].IsString());

            render::blending_color_mask mask = blending.color_mask();
            if ( !parse_blending_color_mask(root["color_mask"].GetString(), mask) ) {
                E2D_ASSERT_MSG(false, "unexpected blending state color mask");
                return stdex::make_rejected_promise<render::blending_state_opt>(
                    material_asset_loading_exception());
            }

            blending.color_mask(mask);
        }

        if ( root.HasMember("src_factor") ) {
            if ( root["src_factor"].IsString() ) {
                render::blending_factor factor = blending.src_rgb_factor();
                if ( !parse_blending_factor(root["src_factor"].GetString(), factor) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                    return stdex::make_rejected_promise<render::blending_state_opt>(
                        material_asset_loading_exception());
                }
                blending.src_factor(factor);
            } else if ( root["src_factor"].IsObject() ) {
                const auto& root_src_factor = root["src_factor"];

                if ( root_src_factor.HasMember("rgb") ) {
                    E2D_ASSERT(root_src_factor["rgb"].IsString());
                    render::blending_factor factor = blending.src_rgb_factor();
                    if ( !parse_blending_factor(root_src_factor["rgb"].GetString(), factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.src_rgb_factor(factor);
                }

                if ( root_src_factor.HasMember("alpha") ) {
                    E2D_ASSERT(root_src_factor["alpha"].IsString());
                    render::blending_factor factor = blending.src_alpha_factor();
                    if ( !parse_blending_factor(root_src_factor["alpha"].GetString(), factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.src_alpha_factor(factor);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state src factor");
            }
        }

        if ( root.HasMember("dst_factor") ) {
            if ( root["dst_factor"].IsString() ) {
                render::blending_factor factor = blending.dst_rgb_factor();
                if ( !parse_blending_factor(root["dst_factor"].GetString(), factor) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                    return stdex::make_rejected_promise<render::blending_state_opt>(
                        material_asset_loading_exception());
                }
                blending.dst_factor(factor);
            } else if ( root["dst_factor"].IsObject() ) {
                const auto& root_dst_factor = root["dst_factor"];

                if ( root_dst_factor.HasMember("rgb") ) {
                    E2D_ASSERT(root_dst_factor["rgb"].IsString());
                    render::blending_factor factor = blending.dst_rgb_factor();
                    if ( !parse_blending_factor(root_dst_factor["rgb"].GetString(), factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.dst_rgb_factor(factor);
                }

                if ( root_dst_factor.HasMember("alpha") ) {
                    E2D_ASSERT(root_dst_factor["alpha"].IsString());
                    render::blending_factor factor = blending.dst_alpha_factor();
                    if ( !parse_blending_factor(root_dst_factor["alpha"].GetString(), factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.dst_alpha_factor(factor);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
            }
        }

        if ( root.HasMember("equation") ) {
            if ( root["equation"].IsString() ) {
                render::blending_equation equation = blending.rgb_equation();
                if ( !parse_blending_equation(root["equation"].GetString(), equation) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state equation");
                    return stdex::make_rejected_promise<render::blending_state_opt>(
                        material_asset_loading_exception());
                }
                blending.equation(equation);
            } else if ( root["equation"].IsObject() ) {
                const auto& root_equation = root["equation"];

                if ( root_equation.HasMember("rgb") ) {
                    E2D_ASSERT(root_equation["rgb"].IsString());
                    render::blending_equation equation = blending.rgb_equation();
                    if ( !parse_blending_equation(root_equation["rgb"].GetString(), equation) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state equation");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.rgb_equation(equation);
                }

                if ( root_equation.HasMember("alpha") ) {
                    E2D_ASSERT(root_equation["alpha"].IsString());
                    render::blending_equation equation = blending.alpha_equation();
                    if ( !parse_blending_equation(root_equation["alpha"].GetString(), equation) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state equation");
                        return stdex::make_rejected_promise<render::blending_state_opt>(
                            material_asset_loading_exception());
                    }
                    blending.alpha_equation(equation);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state equation");
            }
        }

        return stdex::make_resolved_promise(
            render::blending_state_opt(blending));
    }

    stdex::promise<render::material> parse_material(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        auto shader_p = root.HasMember("shader")
            ? parse_shader_block(library, parent_address, root["shader"])
            : stdex::make_resolved_promise<shader_ptr>(nullptr);

        auto constants_p = root.HasMember("properties")
            ? create_const_buffer(root["properties"], shader_p)
            : stdex::make_resolved_promise<const_buffer_ptr>(nullptr);

        auto samplers_p = parse_sampler_block(library, parent_address, root);

        auto blending_p = root.HasMember("blending_state")
            ? parse_blending_state(root["blending_state"])
            : stdex::make_resolved_promise<render::blending_state_opt>({});
        
        auto culling_p = parse_culling_state(root);
        
        return stdex::make_tuple_promise(std::make_tuple(
            shader_p, constants_p, samplers_p, blending_p, culling_p))
            .then([](const std::tuple<
                shader_ptr,
                const_buffer_ptr,
                render::sampler_block,
                render::blending_state_opt,
                render::culling_state_opt
            >& results) {
                render::material content;
                content.shader(std::get<0>(results));
                content.constants(std::get<1>(results));
                content.samplers(std::get<2>(results));
                if ( auto& blending = std::get<3>(results); blending.has_value() ) {
                    content.blending(*blending);
                }
                if ( auto& culling = std::get<4>(results); culling.has_value() ) {
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

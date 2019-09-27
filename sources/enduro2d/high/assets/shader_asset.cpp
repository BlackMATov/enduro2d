/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/shader_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/assets/text_asset.hpp>
#include <enduro2d/high/assets/cbuffer_template_asset.hpp>

namespace
{
    using namespace e2d;

    class shader_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "shader asset loading exception";
        }
    };

    const char* shader_asset_schema_source = R"json(
        {
            "type" : "object",
            "required" : [],
            "additionalProperties" : false,
            "properties" : {
                "attributes" : {
                    "type" : "array",
                    "items" : { "$ref": "#/definitions/attribute" }
                },
                "samplers" : {
                    "type" : "array",
                    "items" : { "$ref": "#/definitions/sampler" }
                },
                "render_pass_block" : { "$ref": "#/common_definitions/address" },
                "material_block" : { "$ref": "#/common_definitions/address" },
                "draw_command_block" : { "$ref": "#/common_definitions/address" },
                "opengl" : {
                    "type" : "array",
                    "items" : { "$ref": "#/definitions/shader_src" }
                }
            },
            "definitions" : {
                "shader_src" : {
                    "type" : "object",
                    "required" : [ "vertex", "fragment" ],
                    "additionalProperties" : false,
                    "properties" : {
                        "vertex" : { "$ref": "#/common_definitions/address" },
                        "fragment" : { "$ref": "#/common_definitions/address" },
                        "requires" : {
                            "type" : "array",
                            "items" : { "$ref": "#/common_definitions/name" }
                        }
                    }
                },
                "sampler" : {
                    "type" : "object",
                    "required" : [ "name", "unit" ],
                    "additionalProperties" : false,
                    "properties" : {
                        "name" : { "$ref": "#/common_definitions/name" },
                        "unit" : { "type" : "integer", "minimum" : 0, "maximum" : 8 },
                        "type" : { "$ref": "#/definitions/sampler_type" },
                        "scope" : { "$ref": "#/definitions/scope_type" }
                    }
                },
                "attribute" : {
                    "type" : "object",
                    "required" : [ "name", "index", "type" ],
                    "additionalProperties" : false,
                    "properties" : {
                        "name" : { "$ref": "#/common_definitions/name" },
                        "index" : { "type" : "integer", "minimum" : 0, "maximum" : 16 },
                        "type" : { "$ref": "#/definitions/attribute_type" }
                    }
                },
                "attribute_type" : {
                    "type" : "string",
                    "enum" : [
                        "f32",
                        "v2f",
                        "v3f",
                        "v4f",
                        "m2f",
                        "m3f",
                        "m4f"
                    ]
                },
                "scope_type" : {
                    "type" : "string",
                    "enum" : [
                        "render_pass",
                        "material",
                        "draw_command"
                    ]
                },
                "sampler_type" : {
                    "type" : "string",
                    "enum" : [
                        "_2d",
                        "cube_map"
                    ]
                }
            }
        })json";

    const rapidjson::SchemaDocument& shader_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(shader_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse shader asset schema");
                throw shader_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    bool parse_attribute_type(str_view str, shader_source::value_type& value) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { value = shader_source::value_type::x; return true; }
        DEFINE_IF(f32);
        DEFINE_IF(v2f);
        DEFINE_IF(v3f);
        DEFINE_IF(v4f);
        DEFINE_IF(m2f);
        DEFINE_IF(m3f);
        DEFINE_IF(m4f);
    #undef DEFINE_IF
        return false;
    }

    bool parse_scope_type(str_view str, shader_source::scope_type& value) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { value = shader_source::scope_type::x; return true; }
        DEFINE_IF(render_pass);
        DEFINE_IF(material);
        DEFINE_IF(draw_command);
    #undef DEFINE_IF
        return false;
    }

    bool parse_sampler_type(str_view str, shader_source::sampler_type& value) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { value = shader_source::sampler_type::x; return true; }
        DEFINE_IF(_2d);
        DEFINE_IF(cube_map);
    #undef DEFINE_IF
        return false;
    }

    bool parse_attribute(const rapidjson::Value& root, shader_source& shader_src) {
        E2D_ASSERT(root.IsObject());
        E2D_ASSERT(root.HasMember("name"));
        E2D_ASSERT(root.HasMember("index"));
        E2D_ASSERT(root.HasMember("type"));

        u32 index;
        if ( !json_utils::try_parse_value(root["index"], index) ) {
            the<debug>().error("SHADER: Incorrect formatting of 'attribute.index' property");
            return false;
        }

        shader_source::value_type type;
        if ( !parse_attribute_type(root["type"].GetString(), type) ) {
            the<debug>().error("SHADER: Incorrect formatting of 'attribute.type' property");
            return false;
        }

        E2D_ASSERT(root["name"].IsString());
        shader_src.add_attribute(root["name"].GetString(), index, type);
        return true;
    }

    bool parse_sampler(const rapidjson::Value& root, shader_source& shader_src) {
        E2D_ASSERT(root.IsObject());
        E2D_ASSERT(root.HasMember("name"));
        E2D_ASSERT(root.HasMember("unit"));

        u32 unit;
        if ( !json_utils::try_parse_value(root["unit"], unit) ) {
            the<debug>().error("SHADER: Incorrect formatting of 'sampler.unit' property");
            return false;
        }

        auto scope = shader_source::scope_type::material;
        if ( root.HasMember("scope") && !parse_scope_type(root["scope"].GetString(), scope) ) {
            the<debug>().error("SHADER: Incorrect formatting of 'sampler.scope' property");
            return false;
        }

        auto type = shader_source::sampler_type::_2d;
        if ( root.HasMember("type") && !parse_sampler_type(root["type"].GetString(), type) ) {
            the<debug>().warning("SHADER: Incorrect formatting of 'sampler.type' property");
        }

        E2D_ASSERT(root["name"].IsString());
        shader_src.add_sampler(root["name"].GetString(), unit, type, scope);
        return true;
    }

    stdex::promise<cbuffer_template_asset::load_result> parse_const_block(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root,
        const char* name)
    {
        if ( !root.HasMember(name) ) {
            return stdex::make_resolved_promise<cbuffer_template_asset::load_result>(nullptr);
        }
        return library.load_asset_async<cbuffer_template_asset>(
            path::combine(parent_address, root[name].GetString()));
    }

    auto parse_shader_src(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        E2D_ASSERT(root.IsObject());

        E2D_ASSERT(root.HasMember("vertex") && root["vertex"].IsString());
        auto vertex_p = library.load_asset_async<text_asset>(
            path::combine(parent_address, root["vertex"].GetString()));

        E2D_ASSERT(root.HasMember("fragment") && root["fragment"].IsString());
        auto fragment_p = library.load_asset_async<text_asset>(
            path::combine(parent_address, root["fragment"].GetString()));

        return stdex::make_tuple_promise(std::make_tuple(
            std::move(vertex_p),
            std::move(fragment_p)));
    }

    auto choose_shader_version(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        if ( root.HasMember("opengl") ) {
            auto& gl_shaders = root["opengl"];
            E2D_ASSERT(gl_shaders.IsArray());
            for ( rapidjson::SizeType i = 0; i < gl_shaders.Size(); ++i ) {
                auto& item = gl_shaders[i];
                bool supported = true;
                if ( item.HasMember("requires") ) {
                    auto& requires = item["requires"];
                    for ( rapidjson::SizeType j = 0; j < requires.Size(); ++j ) {
                        //supported |= library.environment(requires[j]);  // TODO
                    }
                } 
                if ( supported ) {
                    return parse_shader_src(library, parent_address, item);
                }
            }
        }
        the<debug>().error("SHADER: Can't find suitable shader version");
        return stdex::make_tuple_promise(std::make_tuple(
            stdex::make_rejected_promise<text_asset::load_result>(shader_asset_loading_exception()),
            stdex::make_rejected_promise<text_asset::load_result>(shader_asset_loading_exception())));
    }

    stdex::promise<shader_ptr> parse_shader(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        auto shader_src = std::make_shared<shader_source>();

        if ( root.HasMember("attributes") ) {
            auto& attributes = root["attributes"];
            E2D_ASSERT(attributes.IsArray());

            for ( rapidjson::SizeType i = 0; i < attributes.Size(); ++i ) {
                if ( !parse_attribute(attributes[i], *shader_src) ) {
                    the<debug>().error("SHADER: Incorrect formatting of 'attributes' property");
                    return stdex::make_rejected_promise<shader_ptr>(shader_asset_loading_exception());
                }
            }
        }
        if ( root.HasMember("samplers") ) {
            auto& samplers = root["samplers"];
            E2D_ASSERT(samplers.IsArray());

            for ( rapidjson::SizeType i = 0; i < samplers.Size(); ++i ) {
                if ( !parse_sampler(samplers[i], *shader_src) ) {
                    the<debug>().error("SHADER: Incorrect formatting of 'samplers' property");
                    return stdex::make_rejected_promise<shader_ptr>(shader_asset_loading_exception());
                }
            }
        }

        auto pass_block_p = parse_const_block(library, parent_address, root, "render_pass_block");
        auto mtr_block_p = parse_const_block(library, parent_address, root, "material_block");
        auto cmd_block_p = parse_const_block(library, parent_address, root, "draw_command_block");
        auto source = choose_shader_version(library, parent_address, root);
        
        return stdex::make_tuple_promise(std::make_tuple(source, pass_block_p, mtr_block_p, cmd_block_p))
            .then([shader_src](const std::tuple<
                std::tuple<text_asset::load_result, text_asset::load_result>,
                cbuffer_template_asset::load_result,
                cbuffer_template_asset::load_result,
                cbuffer_template_asset::load_result
            >& results) {
                return the<deferrer>().do_in_main_thread([results, shader_src]() {
                    auto& source = std::get<0>(results);
                    shader_src->vertex_shader(std::get<0>(source)->content());
                    shader_src->fragment_shader(std::get<1>(source)->content());

                    if ( auto& pass_block = std::get<1>(results) ) {
                        shader_src->set_block(pass_block->content(), shader_source::scope_type::render_pass);
                    }
                    if ( auto& mtr_block = std::get<2>(results) ) {
                        shader_src->set_block(mtr_block->content(), shader_source::scope_type::material);
                    }
                    if ( auto& cmd_block = std::get<3>(results) ) {
                        shader_src->set_block(cmd_block->content(), shader_source::scope_type::draw_command);
                    }

                    const shader_ptr content = the<render>().create_shader(*shader_src);
                    if ( !content ) {
                        throw shader_asset_loading_exception();
                    }
                    return content;
                });
            });
    }
}

namespace e2d
{
    shader_asset::load_async_result shader_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& shader_data){
            return the<deferrer>().do_in_worker_thread([address, shader_data](){
                const rapidjson::Document& doc = *shader_data->content();
                rapidjson::SchemaValidator validator(shader_asset_schema());

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

                throw shader_asset_loading_exception();
            })
            .then([&library, parent_address, shader_data](){
                return parse_shader(
                    library, parent_address, *shader_data->content());
            })
            .then([](auto&& content){
                return shader_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

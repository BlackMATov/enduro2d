/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/cbuffer_template_asset.hpp>
#include <enduro2d/high/assets/json_asset.hpp>

namespace
{
    using namespace e2d;
    
    class cbuffer_template_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "cbuffer_template asset loading exception";
        }
    };

    const char* cbuffer_template_asset_schema_source = R"json({
        "type" : "object",
        "required" : [ "uniforms" ],
        "additionalProperties" : false,
        "properties" : {
            "uniforms" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/uniform" }
            }
        },
        "definitions" : {
            "uniform" : {
                "type" : "object",
                "required" : [ "name", "offset", "type" ],
                "additionalProperties" : false,
                "properties" : {
                    "name" : { "$ref": "#/common_definitions/name" },
                    "offset" : { "type" : "integer", "minimum" : 0 },
                    "type" : { "$ref" : "#/definitions/uniform_type" }
                }
            },
            "uniform_type" : {
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
            }
        }
    })json";

    const rapidjson::SchemaDocument& cbuffer_template_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(cbuffer_template_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse cbuffer_template asset schema");
                throw cbuffer_template_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }
    
    bool parse_uniform_type(str_view str, cbuffer_template::value_type& value) noexcept {
    #define DEFINE_IF(x) if ( str == #x ) { value = cbuffer_template::value_type::x; return true; }
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

    bool parse_uniform(const rapidjson::Value& root, cbuffer_template& content) noexcept {
        E2D_ASSERT(root.IsObject());
        if ( root.HasMember("name") &&
             root.HasMember("offset") &&
             root.HasMember("type") )
        {
            u32 offset;
            if ( !json_utils::try_parse_value(root["offset"], offset) ) {
                the<debug>().error("CBUFFER_TEMPLATE: Incorrect formatting of 'uniform.offset' property");
                return false;
            }
            cbuffer_template::value_type type;
            if ( !parse_uniform_type(root["type"].GetString(), type) ) {
                the<debug>().error("CBUFFER_TEMPLATE: Incorrect formatting of 'uniform.type' property");
                return true;
            }
            E2D_ASSERT(root["name"].IsString());
            content.add_uniform(root["name"].GetString(), offset, type);
            return true;
        }
        return false;
    }

    stdex::promise<cbuffer_template_cptr> parse_cbuffer_template(
        const rapidjson::Value& root)
    {
        if ( !root.HasMember("uniforms") ) {
            the<debug>().error("CBUFFER_TEMPLATE: Property 'uniforms' does not exists");
            return stdex::make_rejected_promise<cbuffer_template_cptr>(
                cbuffer_template_asset_loading_exception());
        }

        cbuffer_template content;
        auto& json_uniforms = root["uniforms"];
        E2D_ASSERT(json_uniforms.IsArray());

        for ( rapidjson::SizeType i = 0; i < json_uniforms.Size(); ++i ) {
            auto& item = json_uniforms[i];
            if ( !parse_uniform(item, content) ) {
                the<debug>().error("CBUFFER_TEMPLATE: Incorrect formatting of 'uniform' property");
                return stdex::make_rejected_promise<cbuffer_template_cptr>(
                    cbuffer_template_asset_loading_exception());
            }
        }

        return stdex::make_resolved_promise(
            std::make_shared<const cbuffer_template>(std::move(content)));
    }
}

namespace e2d
{
    cbuffer_template_asset::load_async_result cbuffer_template_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([address = str(address)](const json_asset::load_result& cbuffer_template_data){
            return the<deferrer>().do_in_worker_thread([address, cbuffer_template_data](){
                const rapidjson::Document& doc = *cbuffer_template_data->content();
                rapidjson::SchemaValidator validator(cbuffer_template_asset_schema());

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

                throw cbuffer_template_asset_loading_exception();
            })
            .then([cbuffer_template_data](){
                return parse_cbuffer_template(
                    *cbuffer_template_data->content());
            })
            .then([](auto&& content){
                return cbuffer_template_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/sound_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/assets/binary_asset.hpp>

namespace
{
    using namespace e2d;

    class sound_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "sound asset loading exception";
        }
    };

    const char* sound_asset_schema_source = R"json({
        "type" : "object",
        "required" : [ "sound", "stream" ],
        "additionalProperties" : false,
        "properties" : {
            "sound" : { "$ref": "#/common_definitions/address" },
            "stream" : { "type" : "boolean" }
        }
    })json";

    const rapidjson::SchemaDocument& sound_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(sound_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse sound asset schema");
                throw sound_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    stdex::promise<sound_source_ptr> parse_sound(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        E2D_ASSERT(root.HasMember("sound") && root["sound"].IsString());
        str address = path::combine(parent_address, root["sound"].GetString());

        E2D_ASSERT(root.HasMember("stream") && root["stream"].IsBool());
        bool stream = root["stream"].GetBool();

        const auto asset_url = library.root() / address;
        jobber_hpp::promise<sound_source_ptr> result;
        if ( stream ) {
            result.resolve(std::move(the<audio>().create_source(
                                        the<audio>().create_stream(
                                            the<vfs>().read(asset_url)))));
        } else {
            result = the<vfs>().load_async(asset_url)
            .then([](auto&& content){
                return the<deferrer>().do_in_main_thread([content](){
                    return std::move(the<audio>().create_source(
                                        the<audio>().preload_stream(content)));
                });
            });
        }

        return result;
    }
}

namespace e2d
{
    sound_asset::load_async_result sound_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
               &library,
               address = str(address),
               parent_address = path::parent_path(address)
               ](const json_asset::load_result& sound_data){
            return the<deferrer>().do_in_worker_thread([address, sound_data](){
                const rapidjson::Document& doc = *sound_data->content();
                rapidjson::SchemaValidator validator(sound_asset_schema());

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

                throw sound_asset_loading_exception();
            })
            .then([&library, parent_address, sound_data](){
                return parse_sound(
                    library, parent_address, *sound_data->content());
            })
            .then([](auto&& content){
                return sound_asset::create(std::forward<decltype(content)>(content));
            });
        });
    }
}

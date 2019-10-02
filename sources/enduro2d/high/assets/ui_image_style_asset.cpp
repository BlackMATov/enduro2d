/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/ui_image_style_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/ui_image_style.hpp>

namespace
{
    using namespace e2d;

    class ui_image_style_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "ui image style asset loading exception";
        }
    };

    const char* ui_image_style_asset_schema_source = R"json({
        "type" : "object",
        "required" : ["idle"],
        "additionalProperties" : false,
        "properties" : {
            "disabled" : { "$ref": "#/common_definitions/address" },
            "idle" : { "$ref": "#/common_definitions/address" },
            "mouse_over" : { "$ref": "#/common_definitions/address" },
            "touched" : { "$ref": "#/common_definitions/address" },
            "selected" : { "$ref": "#/common_definitions/address" },
            "dragging" : { "$ref": "#/common_definitions/address" }
        }
    })json";

    const rapidjson::SchemaDocument& ui_image_style_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(ui_image_style_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse ui color style asset schema");
                throw ui_image_style_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    template < typename T >
    stdex::promise<ui_image_style_templ<typename T::ptr>> parse_ui_image_style(
        const library& library,
        str_view parent_address,
        const rapidjson::Value& root)
    {
        using style_t = ui_image_style_templ<typename T::ptr>;
        using set_image_fn = style_t& (style_t::*)(const typename T::ptr&);
        using sub_promise_t = typename T::load_async_result;
        using sub_promise_result_t = typename sub_promise_t::value_type;
        using image_array_t = std::array<sub_promise_t, 6>;
        using image_array_iter = typename image_array_t::iterator;
        
        image_array_t images;
        const std::array<set_image_fn, 6> set_image_fns = {{
            &style_t::disabled, &style_t::idle, &style_t::mouse_over,
            &style_t::touched, &style_t::selected, &style_t::dragging }};

        if ( root.HasMember("idle") ) {
            images[1] = library.load_asset_async<T>(path::combine(parent_address, root["idle"].GetString()));
        } else {
            E2D_ASSERT_MSG(false, "'idle' parameter should be set");
            return stdex::make_rejected_promise<ui_image_style_templ<typename T::ptr>>(
                ui_image_style_asset_loading_exception());
        }

        if ( root.HasMember("disabled") ) {
            images[0] = library.load_asset_async<T>(path::combine(parent_address, root["disabled"].GetString()));
        } else {
            images[0] = images[1];
        }

        if ( root.HasMember("mouse_over") ) {
            images[2] = library.load_asset_async<T>(path::combine(parent_address, root["mouse_over"].GetString()));
        } else {
            images[2] = images[1];
        }

        if ( root.HasMember("touched") ) {
            images[3] = library.load_asset_async<T>(path::combine(parent_address, root["touched"].GetString()));
        } else {
            images[3] = images[1];
        }
    
        if ( root.HasMember("selected") ) {
            images[4] = library.load_asset_async<T>(path::combine(parent_address, root["selected"].GetString()));
        } else {
            images[4] = images[1];
        }

        if ( root.HasMember("dragging") ) {
            images[5] = library.load_asset_async<T>(path::combine(parent_address, root["dragging"].GetString()));
        } else {
            images[5] = images[1];
        }

        return stdex::make_all_promise<image_array_iter, sub_promise_t, sub_promise_result_t>(images.begin(), images.end())
            .then([set_image_fns](const vector<typename T::load_result>& result){
                style_t style;
                for ( size_t i = 0; i < result.size(); ++i ) {
                    (style.*set_image_fns[i])(result[i]);
                }
                return stdex::make_resolved_promise(style);
            });
    }
}

namespace e2d
{
    ui_sprite_style_asset::load_async_result ui_sprite_style_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& ui_sprite_style_data){
            return the<deferrer>().do_in_worker_thread([address, ui_sprite_style_data](){
                const rapidjson::Document& doc = *ui_sprite_style_data->content();
                rapidjson::SchemaValidator validator(ui_image_style_asset_schema());

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

                throw ui_image_style_asset_loading_exception();
            })
            .then([&library, parent_address, ui_sprite_style_data](){
                return parse_ui_image_style<sprite_asset>(
                    library, parent_address, *ui_sprite_style_data->content());
            })
            .then([](auto&& content){
                return ui_sprite_style_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }

    ui_sprite_9p_style_asset::load_async_result ui_sprite_9p_style_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& ui_sprite_style_data){
            return the<deferrer>().do_in_worker_thread([address, ui_sprite_style_data](){
                const rapidjson::Document& doc = *ui_sprite_style_data->content();
                rapidjson::SchemaValidator validator(ui_image_style_asset_schema());

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

                throw ui_image_style_asset_loading_exception();
            })
            .then([&library, parent_address, ui_sprite_style_data](){
                return parse_ui_image_style<sprite_9p_asset>(
                    library, parent_address, *ui_sprite_style_data->content());
            })
            .then([](auto&& content){
                return ui_sprite_9p_style_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

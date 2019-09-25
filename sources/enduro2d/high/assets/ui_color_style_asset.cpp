/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/ui_color_style_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/ui_color_style.hpp>

namespace
{
    using namespace e2d;

    class ui_color_style_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "ui color style asset loading exception";
        }
    };

    const char* ui_color_style_asset_schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "disabled" : { "$ref": "#/common_definitions/color" },
            "idle" : { "$ref": "#/common_definitions/color" },
            "mouse_over" : { "$ref": "#/common_definitions/color" },
            "touched" : { "$ref": "#/common_definitions/color" },
            "selected" : { "$ref": "#/common_definitions/color" },
            "dragging" : { "$ref": "#/common_definitions/color" }
        }
    })json";

    const rapidjson::SchemaDocument& ui_color_style_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(ui_color_style_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse ui color style asset schema");
                throw ui_color_style_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    stdex::promise<ui_color_style> parse_ui_color_style(
        const library& library,
        const rapidjson::Value& root)
    {
        ui_color_style style;
        color32 color;

        if ( root.HasMember("disabled") ) {
            if ( !json_utils::try_parse_value(root["disabled"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'disabled' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.disabled(color);
        }

        if ( root.HasMember("idle") ) {
            if ( !json_utils::try_parse_value(root["idle"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'idle' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.idle(color);
        }

        if ( root.HasMember("mouse_over") ) {
            if ( !json_utils::try_parse_value(root["mouse_over"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'mouse_over' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.mouse_over(color);
        } else {
            style.mouse_over(style.idle());
        }

        if ( root.HasMember("touched") ) {
            if ( !json_utils::try_parse_value(root["touched"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'touched' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.touched(color);
        } else {
            style.touched(style.idle());
        }
        
        if ( root.HasMember("selected") ) {
            if ( !json_utils::try_parse_value(root["selected"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'selected' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.selected(color);
        } else {
            style.selected(style.idle());
        }
        
        if ( root.HasMember("dragging") ) {
            if ( !json_utils::try_parse_value(root["dragging"], color) ) {
                the<debug>().error("UI_COLOR_STYLE_ASSET: Incorrect formatting of 'dragging' property");
                return stdex::make_rejected_promise<ui_color_style>(ui_color_style_asset_loading_exception());
            }
            style.dragging(color);
        } else {
            style.dragging(style.idle());
        }

        return stdex::make_resolved_promise(style);
    }
}

namespace e2d
{
    ui_color_style_asset::load_async_result ui_color_style_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& ui_color_style_data){
            return the<deferrer>().do_in_worker_thread([address, ui_color_style_data](){
                const rapidjson::Document& doc = *ui_color_style_data->content();
                rapidjson::SchemaValidator validator(ui_color_style_asset_schema());

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

                throw ui_color_style_asset_loading_exception();
            })
            .then([&library, ui_color_style_data](){
                return parse_ui_color_style(
                    library, *ui_color_style_data->content());
            })
            .then([](auto&& content){
                return ui_color_style_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/ui_font_style_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/ui_font_style.hpp>

namespace
{
    using namespace e2d;

    class ui_font_style_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "ui font style asset loading exception";
        }
    };

    const char* ui_font_style_asset_schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "disabled" : { "$ref": "#/definitions/font_style" },
            "idle" : { "$ref": "#/definitions/font_style" },
            "mouse_over" : { "$ref": "#/definitions/font_style" },
            "touched" : { "$ref": "#/definitions/font_style" },
            "selected" : { "$ref": "#/definitions/font_style" },
            "dragging" : { "$ref": "#/definitions/font_style" }
        },
        "definitions" : {
            "font_style" : {
                "type" : "object",
                "required" : [ "tint" ],
                "additionalProperties" : false,
                "properties" : {
                    "tint" : { "$ref": "#/common_definitions/color" },
                    "outline_width" : { "type" : "number" },
                    "outline_color" : { "$ref": "#/common_definitions/color" }
                }
            }
        }
    })json";

    const rapidjson::SchemaDocument& ui_font_style_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(ui_font_style_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse ui font style asset schema");
                throw ui_font_style_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    bool parse_font_style(const rapidjson::Value& root, ui_font_style::style& style) {
        if ( root.HasMember("tint") ) {
            if ( !json_utils::try_parse_value(root["tint"], style.tint) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'tint' property");
                return false;
            }
        }
        if ( root.HasMember("outline_width") ) {
            style.outline_width = root["outline_width"].GetFloat();
        }
        if ( root.HasMember("outline_color") ) {
            if ( !json_utils::try_parse_value(root["outline_color"], style.outline_color) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'outline_color' property");
                return false;
            }
        }
        return true;
    }

    stdex::promise<ui_font_style> parse_ui_font_style(const rapidjson::Value& root) {
        ui_font_style style;

        if ( root.HasMember("disabled") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["disabled"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'disabled' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.disabled(s.tint, s.outline_color, s.outline_width);
        }

        if ( root.HasMember("idle") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["idle"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'idle' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.idle(s.tint, s.outline_color, s.outline_width);
        }

        if ( root.HasMember("mouse_over") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["mouse_over"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'mouse_over' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.mouse_over(s.tint, s.outline_color, s.outline_width);
        } else {
            style.mouse_over(style.idle().tint, style.idle().outline_color, style.idle().outline_width);
        }

        if ( root.HasMember("touched") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["touched"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'touched' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.touched(s.tint, s.outline_color, s.outline_width);
        } else {
            style.touched(style.idle().tint, style.idle().outline_color, style.idle().outline_width);
        }
        
        if ( root.HasMember("selected") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["selected"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'selected' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.selected(s.tint, s.outline_color, s.outline_width);
        } else {
            style.selected(style.idle().tint, style.idle().outline_color, style.idle().outline_width);
        }
        
        if ( root.HasMember("dragging") ) {
            ui_font_style::style s;
            if ( !parse_font_style(root["dragging"], s) ) {
                the<debug>().error("UI_FONT_STYLE_ASSET: Incorrect formatting of 'dragging' property");
                return stdex::make_rejected_promise<ui_font_style>(ui_font_style_asset_loading_exception());
            }
            style.dragging(s.tint, s.outline_color, s.outline_width);
        } else {
            style.dragging(style.idle().tint, style.idle().outline_color, style.idle().outline_width);
        }

        return stdex::make_resolved_promise(style);
    }
}

namespace e2d
{
    ui_font_style_asset::load_async_result ui_font_style_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& ui_font_style_data){
            return the<deferrer>().do_in_worker_thread([address, ui_font_style_data](){
                const rapidjson::Document& doc = *ui_font_style_data->content();
                rapidjson::SchemaValidator validator(ui_font_style_asset_schema());

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

                throw ui_font_style_asset_loading_exception();
            })
            .then([ui_font_style_data](){
                return parse_ui_font_style(*ui_font_style_data->content());
            })
            .then([](auto&& content){
                return ui_font_style_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/ui_layout.hpp>

namespace e2d
{
    ui_layout& ui_layout::update_fn(update_fn_t fn) noexcept {
        update_ = fn;
        return *this;
    }

    ui_layout::update_fn_t ui_layout::update_fn() const noexcept {
        return update_;
    }

    ui_layout& ui_layout::depends_on_childs(bool value) noexcept {
        depends_on_childs_ = value;
        return *this;
    }

    bool ui_layout::depends_on_childs() const noexcept {
        return depends_on_childs_;
    }

    ui_layout& ui_layout::depends_on_parent(bool value) noexcept {
        depends_on_parent_ = value;
        return *this;
    }

    bool ui_layout::depends_on_parent() const noexcept {
        return depends_on_parent_;
    }
    
    ui_layout& ui_layout::enable(bool value) noexcept {
        enabled_ = value;
        return *this;
    }

    bool ui_layout::enabled() const noexcept {
        return enabled_;
    }

    const char* factory_loader<ui_layout::root_tag>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_layout::root_tag>::operator()(
        ui_layout::root_tag& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_layout::root_tag>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<ui_layout::shape2d_update_size_tag>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_layout::shape2d_update_size_tag>::operator()(
        ui_layout::shape2d_update_size_tag& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_layout::shape2d_update_size_tag>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<ui_layout::scissor_update_rect_tag>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<ui_layout::scissor_update_rect_tag>::operator()(
        ui_layout::scissor_update_rect_tag& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<ui_layout::scissor_update_rect_tag>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    fixed_layout::fixed_layout(const v2f& pos, const v2f& size)
    : position_(pos)
    , size_(size) {}

    fixed_layout& fixed_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& fixed_layout::size() const noexcept {
        return size_;
    }
    
    fixed_layout& fixed_layout::position(const v2f& value) noexcept {
        position_ = value;
        return *this;
    }

    const v2f& fixed_layout::position() const noexcept {
        return position_;
    }

    const char* factory_loader<fixed_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [ "size" ],
        "additionalProperties" : false,
        "properties" : {
            "position" : { "$ref": "#/common_definitions/v2" },
            "size" : { "$ref": "#/common_definitions/v2" }
        }
    })json";

    bool factory_loader<fixed_layout>::operator()(
        fixed_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("position") ) {
            v2f pos;
            if ( !json_utils::try_parse_value(ctx.root["position"], pos) ) {
                the<debug>().error("FIXED_LAYOUT: Incorrect formatting of 'position' property");
                return false;
            }
            component.position(pos);
        }
        
        E2D_ASSERT(ctx.root.HasMember("size"));
        v2f size;
        if ( !json_utils::try_parse_value(ctx.root["size"], size) ) {
            the<debug>().error("FIXED_LAYOUT: Incorrect formatting of 'size' property");
            return false;
        }
        component.size(size);
        return true;
    }

    bool factory_loader<fixed_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<fixed_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<fixed_layout::dirty>::operator()(
        fixed_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<fixed_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    auto_layout& auto_layout::min_size(const v2f& value) noexcept {
        min_size_ = value;
        return *this;
    }

    const v2f& auto_layout::min_size() const noexcept {
        return min_size_;
    }

    auto_layout& auto_layout::max_size(const v2f& value) noexcept {
        max_size_ = value;
        return *this;
    }

    const v2f& auto_layout::max_size() const noexcept {
        return max_size_;
    }

    const char* factory_loader<auto_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "min_size" : { "$ref": "#/common_definitions/v2" },
            "max_size" : { "$ref": "#/common_definitions/v2" }
        }
    })json";

    bool factory_loader<auto_layout>::operator()(
        auto_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("min_size") ) {
            v2f size;
            if ( !json_utils::try_parse_value(ctx.root["min_size"], size) ) {
                the<debug>().error("AUTO_LAYOUT: Incorrect formatting of 'min_size' property");
                return false;
            }
            component.min_size(size);
        }
        if ( ctx.root.HasMember("max_size") ) {
            v2f size(std::numeric_limits<f32>::max());
            if ( !json_utils::try_parse_value(ctx.root["max_size"], size) ) {
                the<debug>().error("AUTO_LAYOUT: Incorrect formatting of 'max_size' property");
                return false;
            }
            component.max_size(size);
        }
        return true;
    }

    bool factory_loader<auto_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<auto_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<auto_layout::dirty>::operator()(
        auto_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<auto_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    stack_layout::stack_layout(stack_origin value)
    : origin_(value) {}

    stack_layout& stack_layout::origin(stack_origin value) noexcept {
        origin_ = value;
        return *this;
    }

    stack_layout::stack_origin stack_layout::origin() const noexcept {
        return origin_;
    }
    
    stack_layout& stack_layout::spacing(f32 value) noexcept {
        spacing_ = value;
        return *this;
    }

    f32 stack_layout::spacing() const noexcept {
        return spacing_;
    }

    const char* factory_loader<stack_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [ "origin" ],
        "additionalProperties" : false,
        "properties" : {
            "origin" : { "$ref": "#/definitions/stack_origin" },
            "spacing" : { "type": "number" }
        },
        "definitions" : {
            "stack_origin" : {
                "type" : "string",
                "enum" : [
                    "left",
                    "top",
                    "right",
                    "bottom"
                ]
            }
        }
    })json";

    bool factory_loader<stack_layout>::operator()(
        stack_layout& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("origin"));
        str_view str = ctx.root["origin"].GetString();
        if ( str == "left" ) {
            component.origin(stack_layout::stack_origin::left);
        } else if ( str == "top" ) {
            component.origin(stack_layout::stack_origin::top);
        } else if ( str == "right" ) {
            component.origin(stack_layout::stack_origin::right);
        } else if ( str == "bottom" ) {
            component.origin(stack_layout::stack_origin::bottom);
        } else {
            the<debug>().error("STACK_LAYOUT: Incorrect formatting of 'origin' property");
            return false;
        }

        if ( ctx.root.HasMember("spacing") ) {
            component.spacing(ctx.root["spacing"].GetFloat());
        }
        return true;
    }

    bool factory_loader<stack_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<stack_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<stack_layout::dirty>::operator()(
        stack_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<stack_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    dock_layout::dock_layout(dock_type dock)
    : dock_(dock) {}

    dock_layout& dock_layout::dock(dock_type value) noexcept {
        dock_ = value;
        return *this;
    }

    dock_layout::dock_type dock_layout::dock() const noexcept {
        return dock_;
    }
    
    bool dock_layout::has_dock(dock_type value) const noexcept {
        auto mask = utils::enum_to_underlying(value);
        return (utils::enum_to_underlying(dock_) & mask) == mask;
    }

    dock_layout::dock_type operator|(dock_layout::dock_type l, dock_layout::dock_type r) noexcept {
        return dock_layout::dock_type(utils::enum_to_underlying(l) | utils::enum_to_underlying(r));
    }

    const char* factory_loader<dock_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [ "hdock", "vdock" ],
        "additionalProperties" : false,
        "properties" : {
            "hdock" : { "$ref": "#/definitions/horizontal_dock_type" },
            "vdock" : { "$ref": "#/definitions/vertical_dock_type" }
        },
        "definitions" : {
            "horizontal_dock_type" : {
                "type" : "string",
                "enum" : [
                    "left",
                    "right",
                    "center",
                    "fill"
                ]
            },
            "vertical_dock_type" : {
                "type" : "string",
                "enum" : [
                    "bottom",
                    "top",
                    "center",
                    "fill"
                ]
            }
        }
    })json";
    
    using dock_type = dock_layout::dock_type;

    bool parse_hdock(const rapidjson::Value& root, dock_type& dock) {
        E2D_ASSERT(root.IsString());
        str_view hdock = root.GetString();
        if ( hdock == "left" ) {
            dock = dock | dock_type::left;
        } else if ( hdock == "right" ) {
            dock = dock | dock_type::right;
        } else if ( hdock == "center" ) {
            dock = dock | dock_type::center_x;
        } else if ( hdock == "fill" ) {
            dock = dock | dock_type::fill_x;
        } else if ( hdock != "none" ) {
            return false;
        }
        return true;
    }
    
    bool parse_vdock(const rapidjson::Value& root, dock_type& dock) {
        E2D_ASSERT(root.IsString());
        str_view vdock = root.GetString();
        if ( vdock == "top" ) {
            dock = dock | dock_type::top;
        } else if ( vdock == "bottom" ) {
            dock = dock | dock_type::bottom;
        } else if ( vdock == "center" ) {
            dock = dock | dock_type::center_y;
        } else if ( vdock == "fill" ) {
            dock = dock | dock_type::fill_y;
        } else if ( vdock != "none" ) {
            return false;
        }
        return true;
    }

    bool factory_loader<dock_layout>::operator()(
        dock_layout& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("hdock"));
        E2D_ASSERT(ctx.root.HasMember("vdock"));

        dock_type dock = dock_type::none;

        if ( !parse_hdock(ctx.root["hdock"], dock) ) {
            the<debug>().error("DOCK_LAYOUT: Incorrect formatting of 'hdock' property");
            return false;
        }
        if ( !parse_vdock(ctx.root["vdock"], dock) ) {
            the<debug>().error("DOCK_LAYOUT: Incorrect formatting of 'vdock' property");
            return false;
        }

        component.dock(dock);
        return true;
    }

    bool factory_loader<dock_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<dock_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<dock_layout::dirty>::operator()(
        dock_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<dock_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    sized_dock_layout& sized_dock_layout::dock(dock_type value) noexcept {
        dock_ = value;
        return *this;
    }

    sized_dock_layout::dock_type sized_dock_layout::dock() const noexcept {
        return dock_;
    }
    
    bool sized_dock_layout::has_dock(dock_type value) const noexcept {
        auto mask = utils::enum_to_underlying(value);
        return (utils::enum_to_underlying(dock_) & mask) == mask;
    }
    
    sized_dock_layout& sized_dock_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& sized_dock_layout::size() const noexcept {
        return size_;
    }

    const char* factory_loader<sized_dock_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "hdock" : { "$ref": "#/definitions/horizontal_dock_type" },
            "vdock" : { "$ref": "#/definitions/vertical_dock_type" },
            "size" : { "$ref": "#/common_definitions/v2" }
        },
        "definitions" : {
            "horizontal_dock_type" : {
                "type" : "string",
                "enum" : [
                    "left",
                    "right",
                    "center",
                    "fill",
                    "none"
                ]
            },
            "vertical_dock_type" : {
                "type" : "string",
                "enum" : [
                    "bottom",
                    "top",
                    "center",
                    "fill",
                    "none"
                ]
            }
        }
    })json";

    bool factory_loader<sized_dock_layout>::operator()(
        sized_dock_layout& component,
        const fill_context& ctx) const
    {
        dock_type dock = dock_type::none;

        if ( ctx.root.HasMember("hdock") ) {
            if ( !parse_hdock(ctx.root["hdock"], dock) ) {
                the<debug>().error("SIZED_DOCK_LAYOUT: Incorrect formatting of 'hdock' property");
                return false;
            }
        }
        if ( ctx.root.HasMember("vdock") ) {
            if ( !parse_vdock(ctx.root["vdock"], dock) ) {
                the<debug>().error("SIZED_DOCK_LAYOUT: Incorrect formatting of 'vdock' property");
                return false;
            }
        }
        component.dock(dock);

        if ( ctx.root.HasMember("size") ) {
            v2f size;
            if ( !json_utils::try_parse_value(ctx.root["size"], size) ) {
                the<debug>().error("SIZED_DOCK_LAYOUT: Incorrect formatting of 'size' property");
                return false;
            }
            component.size(size);
        }
        return true;
    }

    bool factory_loader<sized_dock_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<sized_dock_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<sized_dock_layout::dirty>::operator()(
        sized_dock_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<sized_dock_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    image_layout& image_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    image_layout& image_layout::preserve_aspect(bool value) noexcept {
        preserve_aspect_ = value;
        return *this;
    }

    const v2f& image_layout::size() const noexcept {
        return size_;
    }

    bool image_layout::preserve_aspect() const noexcept {
        return preserve_aspect_;
    }

    const char* factory_loader<image_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "preserve_aspect" : { "type" : "boolean" }
        }
    })json";

    bool factory_loader<image_layout>::operator()(
        image_layout& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("preserve_aspect"));
        component.preserve_aspect(ctx.root["preserve_aspect"].GetBool());
        return true;
    }

    bool factory_loader<image_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<image_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<image_layout::dirty>::operator()(
        image_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<image_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<label_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<label_layout>::operator()(
        label_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<label_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<label_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<label_layout::dirty>::operator()(
        label_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<label_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    label_autoscale_layout& label_autoscale_layout::max_scale(f32 value) noexcept {
        max_scale_ = value;
        return *this;
    }

    label_autoscale_layout& label_autoscale_layout::min_scale(f32 value) noexcept {
        min_scale_ = value;
        return *this;
    }

    f32 label_autoscale_layout::max_scale() const noexcept {
        return max_scale_;
    }

    f32 label_autoscale_layout::min_scale() const noexcept {
        return min_scale_;
    }

    const char* factory_loader<label_autoscale_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "max_scale" : { "type" : "number" },
            "min_scale" : { "type" : "number" }
        }
    })json";

    bool factory_loader<label_autoscale_layout>::operator()(
        label_autoscale_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("max_scale") ) {
            component.max_scale(ctx.root["max_scale"].GetFloat());
        }
        if ( ctx.root.HasMember("min_scale") ) {
            component.min_scale(ctx.root["min_scale"].GetFloat());
        }
        return true;
    }

    bool factory_loader<label_autoscale_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<label_autoscale_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<label_autoscale_layout::dirty>::operator()(
        label_autoscale_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<label_autoscale_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    margin_layout::margin_layout(f32 margin)
    : left_(margin)
    , bottom_(margin)
    , right_(margin)
    , top_(margin) {}

    margin_layout::margin_layout(f32 left, f32 bottom, f32 right, f32 top)
    : left_(left)
    , bottom_(bottom)
    , right_(right)
    , top_(top) {}

    margin_layout& margin_layout::left(f32 value) noexcept {
        left_ = value;
        return *this;
    }

    f32 margin_layout::left() const noexcept {
        return left_;
    }

    margin_layout& margin_layout::top(f32 value) noexcept {
        top_ = value;
        return *this;
    }

    f32 margin_layout::top() const noexcept {
        return top_;
    }

    margin_layout& margin_layout::right(f32 value) noexcept {
        right_ = value;
        return *this;
    }
    
    f32 margin_layout::right() const noexcept {
        return right_;
    }

    margin_layout& margin_layout::bottom(f32 value) noexcept {
        bottom_ = value;
        return *this;
    }

    f32 margin_layout::bottom() const noexcept {
        return bottom_;
    }
    
    margin_layout& margin_layout::set_margin(f32 value) noexcept {
        left_ = value;
        bottom_ = value;
        right_ = value;
        top_ = value;
        return *this;
    }

    const char* factory_loader<margin_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "margin" : { "type" : "number" },
            "left" : { "type" : "number" },
            "top" : { "type" : "number" },
            "right" : { "type" : "number" },
            "bottom" : { "type" : "number" }
        }
    })json";

    bool factory_loader<margin_layout>::operator()(
        margin_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("margin") ) {
            component.set_margin(ctx.root["margin"].GetFloat());
            E2D_ASSERT(!ctx.root.HasMember("left"));
            E2D_ASSERT(!ctx.root.HasMember("top"));
            E2D_ASSERT(!ctx.root.HasMember("right"));
            E2D_ASSERT(!ctx.root.HasMember("bottom"));
            return true;
        }
        
        if ( ctx.root.HasMember("left") ) {
            component.left(ctx.root["left"].GetFloat());
        }
        if ( ctx.root.HasMember("top") ) {
            component.top(ctx.root["top"].GetFloat());
        }
        if ( ctx.root.HasMember("right") ) {
            component.right(ctx.root["right"].GetFloat());
        }
        if ( ctx.root.HasMember("bottom") ) {
            component.bottom(ctx.root["bottom"].GetFloat());
        }
        return true;
    }

    bool factory_loader<margin_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }

    const char* factory_loader<margin_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<margin_layout::dirty>::operator()(
        margin_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<margin_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    padding_layout::padding_layout(f32 left, f32 bottom, f32 right, f32 top)
    : left_(left)
    , bottom_(bottom)
    , right_(right)
    , top_(top) {}

    padding_layout& padding_layout::left(f32 value) noexcept {
        left_ = value;
        return *this;
    }

    f32 padding_layout::left() const noexcept {
        return left_;
    }

    padding_layout& padding_layout::top(f32 value) noexcept {
        top_ = value;
        return *this;
    }

    f32 padding_layout::top() const noexcept {
        return top_;
    }

    padding_layout& padding_layout::right(f32 value) noexcept {
        right_ = value;
        return *this;
    }
    
    f32 padding_layout::right() const noexcept {
        return right_;
    }

    padding_layout& padding_layout::bottom(f32 value) noexcept {
        bottom_ = value;
        return *this;
    }

    f32 padding_layout::bottom() const noexcept {
        return bottom_;
    }

    padding_layout& padding_layout::set_padding(f32 value) noexcept {
        left_ = value;
        bottom_ = value;
        right_ = value;
        top_ = value;
        return *this;
    }

    const char* factory_loader<padding_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "padding" : { "type" : "number" },
            "left" : { "type" : "number" },
            "top" : { "type" : "number" },
            "right" : { "type" : "number" },
            "bottom" : { "type" : "number" }
        }
    })json";

    bool factory_loader<padding_layout>::operator()(
        padding_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("padding") ) {
            component.set_padding(ctx.root["padding"].GetFloat());
            E2D_ASSERT(!ctx.root.HasMember("left"));
            E2D_ASSERT(!ctx.root.HasMember("top"));
            E2D_ASSERT(!ctx.root.HasMember("right"));
            E2D_ASSERT(!ctx.root.HasMember("bottom"));
            return true;
        }

        if ( ctx.root.HasMember("left") ) {
            component.left(ctx.root["left"].GetFloat());
        }
        if ( ctx.root.HasMember("top") ) {
            component.top(ctx.root["top"].GetFloat());
        }
        if ( ctx.root.HasMember("right") ) {
            component.right(ctx.root["right"].GetFloat());
        }
        if ( ctx.root.HasMember("bottom") ) {
            component.bottom(ctx.root["bottom"].GetFloat());
        }
        return true;
    }

    bool factory_loader<padding_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<padding_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<padding_layout::dirty>::operator()(
        padding_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<padding_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    anchor_layout& anchor_layout::left_bottom(const anchor& value) noexcept {
        left_bottom_ = value;
        return *this;
    }

    const anchor_layout::anchor& anchor_layout::left_bottom() const noexcept {
        return left_bottom_;
    }

    anchor_layout& anchor_layout::right_top(const anchor& value) noexcept {
        right_top_ = value;
        return *this;
    }

    const anchor_layout::anchor& anchor_layout::right_top() const noexcept {
        return right_top_;
    }

    const char* factory_loader<anchor_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [ "left_bottom", "right_top" ],
        "additionalProperties" : false,
        "properties" : {
            "left_bottom" : { "$ref": "#/definitions/anchor" },
            "right_top" : { "$ref": "#/definitions/anchor" }
        },
        "definitions" : {
            "anchor" : {
                "type" : "object",
                "required" : [ "position" ],
                "additionalProperties" : false,
                "properties" : {
                    "position" : { "$ref": "#/common_definitions/v2" },
                    "offset_rel" : { "$ref": "#/common_definitions/v2" },
                    "offset_px" : { "$ref": "#/common_definitions/v2" }
                }
            }
        }
    })json";

    bool parse_anchor(const rapidjson::Value& root, anchor_layout::anchor& a) {
        E2D_ASSERT(root.HasMember("position"));

        if ( !json_utils::try_parse_value(root["position"], a.position) ) {
            the<debug>().error("ANCHOR_LAYOUT: Incorrect formatting of 'anchor.position' property");
            return false;
        }

        if ( root.HasMember("offset_rel") ) {
            E2D_ASSERT(!root.HasMember("offset_px"));
            a.relative_offset = true;
            if ( !json_utils::try_parse_value(root["offset_rel"], a.offset) ) {
                the<debug>().error("ANCHOR_LAYOUT: Incorrect formatting of 'anchor.offset_rel' property");
                return false;
            }
        } else if ( root.HasMember("offset_px") ) {
            E2D_ASSERT(!root.HasMember("offset_rel"));
            a.relative_offset = false;
            if ( !json_utils::try_parse_value(root["offset_px"], a.offset) ) {
                the<debug>().error("ANCHOR_LAYOUT: Incorrect formatting of 'anchor.offset_px' property");
                return false;
            }
        } else {
            the<debug>().error("ANCHOR_LAYOUT: One of the property 'offset_px' or 'offset_rel' is required");
            return false;
        }

        return true;
    }

    bool factory_loader<anchor_layout>::operator()(
        anchor_layout& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("left_bottom"));
        E2D_ASSERT(ctx.root.HasMember("right_top"));

        anchor_layout::anchor a;
        if ( !parse_anchor(ctx.root["left_bottom"], a) ) {
            the<debug>().error("ANCHOR_LAYOUT: Incorrect formatting of 'left_bottom' property");
            return false;
        }
        component.left_bottom(a);

        if ( !parse_anchor(ctx.root["right_top"], a) ) {
            the<debug>().error("ANCHOR_LAYOUT: Incorrect formatting of 'right_top' property");
            return false;
        }
        component.right_top(a);

        return true;
    }

    bool factory_loader<anchor_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<anchor_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<anchor_layout::dirty>::operator()(
        anchor_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<anchor_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<bounded_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<bounded_layout>::operator()(
        bounded_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<bounded_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<bounded_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<bounded_layout::dirty>::operator()(
        bounded_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<bounded_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    split_layout& split_layout::origin(origin_type value) noexcept {
        origin_ = value;
        return *this;
    }

    split_layout::origin_type split_layout::origin() const noexcept {
        return origin_;
    }

    split_layout& split_layout::offset_relative(f32 value) noexcept {
        relative_offset_ = true;
        offset_ = value;
        return *this;
    }

    split_layout& split_layout::offset_pixels(f32 value) noexcept {
        relative_offset_ = false;
        offset_ = value;
        return *this;
    }

    f32 split_layout::offset() const noexcept {
        return offset_;
    }

    bool split_layout::is_relative_offset() const noexcept {
        return relative_offset_;
    }
    
    const char* factory_loader<split_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [ "origin" ],
        "additionalProperties" : false,
        "properties" : {
            "offset_rel" : { "type" : "number" },
            "offset_px" : { "type" : "number" },
            "origin" : { "$ref": "#/definitions/origin_type" }
        },
        "definitions" : {
            "origin_type" : {
                "type" : "string",
                "enum" : [
                    "left",
                    "right",
                    "bottom",
                    "top"
                ]
            }
        }
    })json";

    bool factory_loader<split_layout>::operator()(
        split_layout& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("offset_rel") ) {
            E2D_ASSERT(!ctx.root.HasMember("offset_px"));
            component.offset_relative(ctx.root["offset_rel"].GetFloat());
        } else if ( ctx.root.HasMember("offset_px") ) {
            E2D_ASSERT(!ctx.root.HasMember("offset_rel"));
            component.offset_pixels(ctx.root["offset_px"].GetFloat());
        } else {
            the<debug>().error("SPLIT_LAYOUT: One of the property 'offset_px' or 'offset_rel' is required");
            return false;
        }

        E2D_ASSERT(ctx.root.HasMember("origin"));
        str_view dir = ctx.root["origin"].GetString();
        if ( dir == "left" ) {
            component.origin(split_layout::origin_type::left);
        } else if ( dir == "right" ) {
            component.origin(split_layout::origin_type::right);
        } else if ( dir == "top" ) {
            component.origin(split_layout::origin_type::top);
        } else if ( dir == "bottom" ) {
            component.origin(split_layout::origin_type::bottom);
        } else {
            the<debug>().error("SPLIT_LAYOUT: Incorrect formatting of 'origin' property");
            return false;
        }

        return true;
    }

    bool factory_loader<split_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
    
    const char* factory_loader<split_layout::dirty>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<split_layout::dirty>::operator()(
        split_layout::dirty& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<split_layout::dirty>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

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

    ui_layout& ui_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& ui_layout::size() const noexcept {
        return size_;
    }

    ui_layout& ui_layout::depends_on_childs(bool enable) noexcept {
        depends_on_childs_ = enable;
        return *this;
    }

    bool ui_layout::depends_on_childs() const noexcept {
        return depends_on_childs_;
    }

    ui_layout& ui_layout::depends_on_parent(bool enable) noexcept {
        depends_on_parent_ = enable;
        return *this;
    }

    bool ui_layout::depends_on_parent() const noexcept {
        return depends_on_parent_;
    }
}

namespace e2d
{
    fixed_layout::fixed_layout(const v2f& size)
    : size_(size) {}

    fixed_layout& fixed_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    const v2f& fixed_layout::size() const noexcept {
        return size_;
    }
}

namespace e2d
{
    const char* factory_loader<fixed_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<fixed_layout>::operator()(
        fixed_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<fixed_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    const char* factory_loader<auto_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<auto_layout>::operator()(
        auto_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<auto_layout>::operator()(
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
}

namespace e2d
{
    const char* factory_loader<stack_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<stack_layout>::operator()(
        stack_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<stack_layout>::operator()(
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
}

namespace e2d
{
    const char* factory_loader<dock_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<dock_layout>::operator()(
        dock_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<dock_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    image_layout& image_layout::pivot(const v2f& value) noexcept {
        pivot_ = value;
        return *this;
    }

    image_layout& image_layout::size(const v2f& value) noexcept {
        size_ = value;
        return *this;
    }

    image_layout& image_layout::preserve_aspect(bool value) noexcept {
        preserve_aspect_ = value;
        return *this;
    }
        
    const v2f& image_layout::pivot() const noexcept {
        return pivot_;
    }

    const v2f& image_layout::size() const noexcept {
        return size_;
    }

    bool image_layout::preserve_aspect() const noexcept {
        return preserve_aspect_;
    }
}

namespace e2d
{
    const char* factory_loader<image_layout>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";

    bool factory_loader<image_layout>::operator()(
        image_layout& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<image_layout>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

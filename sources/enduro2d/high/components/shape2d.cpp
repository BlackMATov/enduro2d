/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/shape2d.hpp>

namespace e2d
{
    rectangle_shape::rectangle_shape(const b2f& r)
    : rect_(r) {}
    
    rectangle_shape& rectangle_shape::rectangle(const b2f& value) noexcept {
        rect_ = value;
        return *this;
    }

    const b2f& rectangle_shape::rectangle() const noexcept {
        return rect_;
    }
    
    const char* factory_loader<rectangle_shape>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<rectangle_shape>::operator()(
        rectangle_shape& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<rectangle_shape>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    circle_shape::circle_shape(const v2f& c, f32 r)
    : center_(c)
    , radius_(r) {}
        
    circle_shape& circle_shape::center(const v2f& value) noexcept {
        center_ = value;
        return *this;
    }

    circle_shape& circle_shape::radius(f32 value) noexcept {
        radius_ = value;
        return *this;
    }

    v2f circle_shape::center() const noexcept {
        return center_;
    }

    f32 circle_shape::radius() const noexcept {
        return radius_;
    }

    const char* factory_loader<circle_shape>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<circle_shape>::operator()(
        circle_shape& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<circle_shape>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

namespace e2d
{
    polygon_shape::polygon_shape(std::initializer_list<triangle> list)
    : triangles_(list) {}
    
    polygon_shape::polygon_shape(std::vector<triangle>&& arr)
    : triangles_(std::move(arr)) {}

    const std::vector<polygon_shape::triangle>& polygon_shape::triangles() const noexcept {
        return triangles_;
    }

    std::vector<polygon_shape::triangle>& polygon_shape::triangles() noexcept {
        return triangles_;
    }
    
    const char* factory_loader<polygon_shape>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {}
    })json";

    bool factory_loader<polygon_shape>::operator()(
        polygon_shape& component,
        const fill_context& ctx) const
    {
        E2D_UNUSED(component, ctx);
        return true;
    }

    bool factory_loader<polygon_shape>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

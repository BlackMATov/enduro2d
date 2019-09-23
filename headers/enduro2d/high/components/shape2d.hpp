/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"
#include "../factory.hpp"

namespace e2d
{
    class rectangle_shape final {
    public:
        rectangle_shape() = default;
        rectangle_shape(const b2f& r);
        
        rectangle_shape& rectangle(const b2f& value) noexcept;
        const b2f& rectangle() const noexcept;
    private:
        b2f rect_;
    };
    
    template <>
    class factory_loader<rectangle_shape> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            rectangle_shape& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class circle_shape final {
    public:
        circle_shape() = default;
        circle_shape(const v2f& c, f32 r);
        
        circle_shape& center(const v2f& value) noexcept;
        circle_shape& radius(f32 value) noexcept;

        v2f center() const noexcept;
        f32 radius() const noexcept;

        static constexpr u32 detail_level = 16; // segments per circle
    private:
        v2f center_;
        f32 radius_ = 0.0f;
    };
    
    template <>
    class factory_loader<circle_shape> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            circle_shape& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class polygon_shape final {
    public:
        struct triangle {
            v3f p0, p1, p2;
        };
    public:
        polygon_shape() = default;
        polygon_shape(std::initializer_list<triangle> list);
        polygon_shape(std::vector<triangle>&& arr);

        const std::vector<triangle>& triangles() const noexcept;
        std::vector<triangle>& triangles() noexcept;
    private:
        std::vector<triangle> triangles_;
    };
    
    template <>
    class factory_loader<polygon_shape> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            polygon_shape& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

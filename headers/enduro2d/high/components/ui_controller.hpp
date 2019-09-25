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
    class ui_button final {
    public:
        ui_button() = default;
    };

    template <>
    class factory_loader<ui_button> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_button& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_selectable final {
    public:
        ui_selectable() = default;
    };

    template <>
    class factory_loader<ui_selectable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_selectable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_draggable final {
    public:
        ui_draggable() = default;
    };

    template <>
    class factory_loader<ui_draggable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_draggable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

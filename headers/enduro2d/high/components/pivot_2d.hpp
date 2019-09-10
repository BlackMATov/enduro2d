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
    class pivot_2d final {
    public:
        pivot_2d() = default;
        pivot_2d(const v2f& p);

        pivot_2d& pivot(const v2f& value) noexcept;
        const v2f& pivot() const noexcept;
    private:
        v2f pivot_;
    };

    template <>
    class factory_loader<pivot_2d> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            pivot_2d& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

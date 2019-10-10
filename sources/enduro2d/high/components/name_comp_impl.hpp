/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include <enduro2d/high/components/name_comp.hpp>

namespace e2d
{
    class name_comp_impl final {
    public:
        name_comp_impl() = default;

        name_comp_impl& name(str value) {
            name_ = std::move(value);
            return *this;
        }

        const str& name() const noexcept {
            return name_;
        }
    private:
        str name_;
    };

    template <>
    class factory_loader<name_comp_impl> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            name_comp_impl& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}


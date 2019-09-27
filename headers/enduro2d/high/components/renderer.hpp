/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"
#include "../assets/material_asset.hpp"

namespace e2d
{
    class renderer final {
    public:
        renderer() = default;

        renderer& enabled(bool value) noexcept;
        bool enabled() const noexcept;

        renderer& materials(vector<material_asset::ptr>&& value) noexcept;
        renderer& materials(const vector<material_asset::ptr>& value);

        vector<material_asset::ptr>& materials() noexcept;
        const vector<material_asset::ptr>& materials() const noexcept;
    private:
        bool enabled_ = true;
        vector<material_asset::ptr> materials_;
    };

    template <>
    class factory_loader<renderer> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            renderer& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline renderer& renderer::enabled(bool value) noexcept {
        enabled_ = value;
        return *this;
    }

    inline bool renderer::enabled() const noexcept {
        return enabled_;
    }

    inline renderer& renderer::materials(vector<material_asset::ptr>&& value) noexcept {
        materials_ = std::move(value);
        return *this;
    }

    inline renderer& renderer::materials(const vector<material_asset::ptr>& value) {
        materials_ = value;
        return *this;
    }

    inline vector<material_asset::ptr>& renderer::materials() noexcept {
        return materials_;
    }

    inline const vector<material_asset::ptr>& renderer::materials() const noexcept {
        return materials_;
    }
}

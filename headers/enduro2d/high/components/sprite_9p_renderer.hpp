/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"
#include "../assets/atlas_asset.hpp"
#include "../assets/sprite_9p_asset.hpp"

namespace e2d
{
    class sprite_9p_renderer final {
    public:
        sprite_9p_renderer() = default;
        sprite_9p_renderer(const sprite_9p_asset::ptr& sprite);

        sprite_9p_renderer& tint(const color32& value) noexcept;
        const color32& tint() const noexcept;

        sprite_9p_renderer& filtering(bool value) noexcept;
        bool filtering() const noexcept;

        sprite_9p_renderer& sprite(const sprite_9p_asset::ptr& value) noexcept;
        const sprite_9p_asset::ptr& sprite() const noexcept;
    private:
        color32 tint_ = color32::white();
        bool filtering_ = true;
        sprite_9p_asset::ptr sprite_;
    };

    template <>
    class factory_loader<sprite_9p_renderer> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            sprite_9p_renderer& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline sprite_9p_renderer::sprite_9p_renderer(const sprite_9p_asset::ptr& sprite)
    : sprite_(sprite) {}

    inline sprite_9p_renderer& sprite_9p_renderer::tint(const color32& value) noexcept {
        tint_ = value;
        return *this;
    }

    inline const color32& sprite_9p_renderer::tint() const noexcept {
        return tint_;
    }

    inline sprite_9p_renderer& sprite_9p_renderer::filtering(bool value) noexcept {
        filtering_ = value;
        return *this;
    }

    inline bool sprite_9p_renderer::filtering() const noexcept {
        return filtering_;
    }

    inline sprite_9p_renderer& sprite_9p_renderer::sprite(const sprite_9p_asset::ptr& value) noexcept {
        sprite_ = value;
        return *this;
    }

    inline const sprite_9p_asset::ptr& sprite_9p_renderer::sprite() const noexcept {
        return sprite_;
    }
}

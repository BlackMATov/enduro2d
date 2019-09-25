/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

#include "assets/texture_asset.hpp"

namespace e2d
{
    class sprite_9p final {
    public:
        sprite_9p() = default;
        ~sprite_9p() noexcept = default;

        sprite_9p(sprite_9p&& other) noexcept;
        sprite_9p& operator=(sprite_9p&& other) noexcept;

        sprite_9p(const sprite_9p& other);
        sprite_9p& operator=(const sprite_9p& other);

        void clear() noexcept;
        void swap(sprite_9p& other) noexcept;

        sprite_9p& assign(sprite_9p&& other) noexcept;
        sprite_9p& assign(const sprite_9p& other);

        sprite_9p& set_texrect(const b2f& value) noexcept;
        sprite_9p& set_inner_texrect(const b2f& value) noexcept;
        sprite_9p& set_texture(const texture_asset::ptr& value) noexcept;

        const b2f& texrect() const noexcept;
        const b2f& inner_texrect() const noexcept;
        const texture_asset::ptr& texture() const noexcept;
    private:
        b2f texrect_;
        b2f inner_rect_;
        texture_asset::ptr texture_;
    };

    void swap(sprite_9p& l, sprite_9p& r) noexcept;
    bool operator==(const sprite_9p& l, const sprite_9p& r) noexcept;
    bool operator!=(const sprite_9p& l, const sprite_9p& r) noexcept;
}

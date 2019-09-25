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
    class sprite final {
    public:
        sprite() = default;
        ~sprite() noexcept = default;

        sprite(sprite&& other) noexcept;
        sprite& operator=(sprite&& other) noexcept;

        sprite(const sprite& other);
        sprite& operator=(const sprite& other);

        void clear() noexcept;
        void swap(sprite& other) noexcept;

        sprite& assign(sprite&& other) noexcept;
        sprite& assign(const sprite& other);

        sprite& set_pivot(const v2f& pivot) noexcept;
        sprite& set_texrect(const b2f& texrect) noexcept;
        sprite& set_texture(const texture_asset::ptr& texture) noexcept;

        const b2f& texrect() const noexcept;
        const texture_asset::ptr& texture() const noexcept;
    private:
        v2f pivot_;
        b2f texrect_;
        texture_asset::ptr texture_;
    };

    void swap(sprite& l, sprite& r) noexcept;
    bool operator==(const sprite& l, const sprite& r) noexcept;
    bool operator!=(const sprite& l, const sprite& r) noexcept;
}

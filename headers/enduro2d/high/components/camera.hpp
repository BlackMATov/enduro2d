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
    class camera final {
    public:
        camera() = default;

        camera& depth(i32 value) noexcept;
        camera& viewport(const b2u& value) noexcept;
        camera& projection(const m4f& value) noexcept;
        camera& target(const render_target_ptr& value) noexcept;
        camera& background(const color& value) noexcept;

        i32 depth() const noexcept;
        const b2u& viewport() const noexcept;
        const m4f& projection() const noexcept;
        const render_target_ptr& target() const noexcept;
        const color& background() const noexcept;
    private:
        i32 depth_ = 0;
        b2u viewport_ = b2u::zero();
        m4f projection_ = m4f::identity();
        render_target_ptr target_ = nullptr;
        color background_ = color::clear();
    };

    template <>
    class factory_loader<camera> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            camera& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline camera& camera::depth(i32 value) noexcept {
        depth_ = value;
        return *this;
    }

    inline camera& camera::viewport(const b2u& value) noexcept {
        viewport_ = value;
        return *this;
    }

    inline camera& camera::projection(const m4f& value) noexcept {
        projection_ = value;
        return *this;
    }

    inline camera& camera::target(const render_target_ptr& value) noexcept {
        target_ = value;
        return *this;
    }

    inline camera& camera::background(const color& value) noexcept {
        background_ = value;
        return *this;
    }

    inline i32 camera::depth() const noexcept {
        return depth_;
    }

    inline const b2u& camera::viewport() const noexcept {
        return viewport_;
    }

    inline const m4f& camera::projection() const noexcept {
        return projection_;
    }

    inline const render_target_ptr& camera::target() const noexcept {
        return target_;
    }

    inline const color& camera::background() const noexcept {
        return background_;
    }
}

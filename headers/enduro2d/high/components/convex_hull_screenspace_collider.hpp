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
    class convex_hull_screenspace_collider final {
    public:
        struct plane2d {
            v2f norm;
            f32 dist;
        };
        using planes_array_t = std::array<plane2d, 8>;
    public:
        convex_hull_screenspace_collider() = default;

        void set_planes(const planes_array_t& pl, size_t count);
        
        const plane2d& plane(size_t i) const noexcept;
        size_t plane_count() const noexcept;
    private:
        planes_array_t planes_;
        size_t plane_count_ = 0;
    };
    
    template <>
    class factory_loader<convex_hull_screenspace_collider> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            convex_hull_screenspace_collider& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline void convex_hull_screenspace_collider::set_planes(const planes_array_t& pl, size_t count) {
        planes_ = pl;
        plane_count_ = count;
    }
        
    inline const convex_hull_screenspace_collider::plane2d& convex_hull_screenspace_collider::plane(size_t i) const noexcept {
        return planes_[i];
    }

    inline size_t convex_hull_screenspace_collider::plane_count() const noexcept {
        return plane_count_;
    }
}

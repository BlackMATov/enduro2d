/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"
#include "../assets/spine_asset.hpp"
#include "../assets/material_asset.hpp"

struct spSkeleton;
struct spSkeletonClipping;
struct spAnimationState;

namespace e2d
{
    class bad_spine_player_access final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad spine player access";
        }
    };

    class spine_player final {
    public:
        using clipping_ptr = std::shared_ptr<spSkeletonClipping>;
        using skeleton_ptr = std::shared_ptr<spSkeleton>;
        using animation_ptr = std::shared_ptr<spAnimationState>;
    public:
        spine_player() = default;
        spine_player(const spine_asset::ptr& spine);

        spine_player& spine(
            const spine_asset::ptr& value);

        spine_player& materials(
            flat_map<str_hash, material_asset::ptr>&& value) noexcept;

        spine_player& materials(
            const flat_map<str_hash, material_asset::ptr>& value);

        spine_player& skin(const str& value) noexcept;
        spine_player& attachment(const str& slot, const str& name) noexcept;

        bool has_skin(const str& name) const noexcept;
        bool has_animation(const str& name) const noexcept;

        const spine_asset::ptr& spine() const noexcept;
        const clipping_ptr& clipper() const noexcept;
        const skeleton_ptr& skeleton() const noexcept;
        const animation_ptr& animation() const noexcept;

        material_asset::ptr find_material(str_hash name) const noexcept;
        const flat_map<str_hash, material_asset::ptr>& materials() const noexcept;
    private:
        spine_asset::ptr spine_;
        clipping_ptr clipping_;
        skeleton_ptr skeleton_;
        animation_ptr animation_;
        flat_map<str_hash, material_asset::ptr> materials_;
    };

    template <>
    class factory_loader<spine_player> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            spine_player& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

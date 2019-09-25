/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

struct spAtlas;
struct spSkeletonData;
struct spAnimationStateData;

namespace e2d
{
    class bad_spine_access final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad spine access";
        }
    };

    class spine final {
    public:
        using atlas_ptr = std::shared_ptr<spAtlas>;
        using skeleton_data_ptr = std::shared_ptr<spSkeletonData>;
        using animation_data_ptr = std::shared_ptr<spAnimationStateData>;
    public:
        spine() = default;
        ~spine() noexcept = default;

        spine(spine&& other) noexcept;
        spine& operator=(spine&& other) noexcept;

        spine(const spine& other);
        spine& operator=(const spine& other);

        void clear() noexcept;
        void swap(spine& other) noexcept;

        spine& assign(spine&& other) noexcept;
        spine& assign(const spine& other);

        spine& set_atlas(atlas_ptr atlas);
        spine& set_skeleton(skeleton_data_ptr skeleton);

        spine& set_default_mix(secf duration);
        spine& set_animation_mix(
            const str& from,
            const str& to,
            secf duration);

        const atlas_ptr& atlas() const noexcept;
        const skeleton_data_ptr& skeleton() const noexcept;
        const animation_data_ptr& animation() const noexcept;
    private:
        atlas_ptr atlas_;
        skeleton_data_ptr skeleton_;
        animation_data_ptr animation_;
    };

    void swap(spine& l, spine& r) noexcept;
    bool operator==(const spine& l, const spine& r) noexcept;
    bool operator!=(const spine& l, const spine& r) noexcept;
}

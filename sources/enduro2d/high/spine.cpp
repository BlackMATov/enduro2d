/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/spine.hpp>

#include <spine/spine.h>

namespace e2d
{
    spine::spine(spine&& other) noexcept {
        assign(std::move(other));
    }

    spine& spine::operator=(spine&& other) noexcept {
        return assign(std::move(other));
    }

    spine::spine(const spine& other) {
        assign(other);
    }

    spine& spine::operator=(const spine& other) {
        return assign(other);
    }

    void spine::clear() noexcept {
        atlas_.reset();
        skeleton_.reset();
        animation_.reset();
    }

    void spine::swap(spine& other) noexcept {
        using std::swap;
        swap(atlas_, other.atlas_);
        swap(skeleton_, other.skeleton_);
        swap(animation_, other.animation_);
    }

    spine& spine::assign(spine&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    spine& spine::assign(const spine& other) {
        if ( this != &other ) {
            spine m;
            m.atlas_ = other.atlas_;
            m.skeleton_ = other.skeleton_;
            m.animation_ = other.animation_;
            swap(m);
        }
        return *this;
    }

    spine& spine::set_atlas(atlas_ptr atlas) {
        atlas_ = std::move(atlas);
        return *this;
    }

    spine& spine::set_skeleton(skeleton_data_ptr skeleton) {
        animation_data_ptr animation;
        if ( skeleton ) {
            animation.reset(
                spAnimationStateData_create(skeleton.get()),
                spAnimationStateData_dispose);
            if ( !animation ) {
                throw std::bad_alloc();
            }
        }
        skeleton_ = std::move(skeleton);
        animation_ = std::move(animation);
        return *this;
    }

    spine& spine::set_default_mix(secf duration) {
        if ( !animation_ ) {
            throw bad_spine_access();
        }
        animation_->defaultMix = duration.value;
        return *this;
    }

    spine& spine::set_animation_mix(
        const str& from,
        const str& to,
        secf duration)
    {
        spAnimation* from_anim = animation_
            ? spSkeletonData_findAnimation(animation_->skeletonData, from.c_str())
            : nullptr;

        spAnimation* to_anim = animation_
            ? spSkeletonData_findAnimation(animation_->skeletonData, to.c_str())
            : nullptr;

        if ( !from_anim || !to_anim ) {
            throw bad_spine_access();
        }

        spAnimationStateData_setMix(animation_.get(), from_anim, to_anim, duration.value);
        return *this;
    }

    const spine::atlas_ptr& spine::atlas() const noexcept {
        return atlas_;
    }

    const spine::skeleton_data_ptr& spine::skeleton() const noexcept {
        return skeleton_;
    }

    const spine::animation_data_ptr& spine::animation() const noexcept {
        return animation_;
    }
}

namespace e2d
{
    void swap(spine& l, spine& r) noexcept {
        l.swap(r);
    }

    bool operator==(const spine& l, const spine& r) noexcept {
        return l.atlas() == r.atlas()
            && l.skeleton() == r.skeleton()
            && l.animation() == r.animation();
    }

    bool operator!=(const spine& l, const spine& r) noexcept {
        return !(l == r);
    }
}

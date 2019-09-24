/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/sprite_9p.hpp>

namespace e2d
{
    sprite_9p::sprite_9p(sprite_9p&& other) noexcept {
        assign(std::move(other));
    }

    sprite_9p& sprite_9p::operator=(sprite_9p&& other) noexcept {
        return assign(std::move(other));
    }

    sprite_9p::sprite_9p(const sprite_9p& other) {
        assign(other);
    }

    sprite_9p& sprite_9p::operator=(const sprite_9p& other) {
        return assign(other);
    }

    void sprite_9p::clear() noexcept {
        texrect_ = b2f::zero();
        inner_rect_ = b2f::zero();
        texture_.reset();
    }

    void sprite_9p::swap(sprite_9p& other) noexcept {
        using std::swap;
        swap(texrect_, other.texrect_);
        swap(inner_rect_, other.inner_rect_);
        swap(texture_, other.texture_);
    }

    sprite_9p& sprite_9p::assign(sprite_9p&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    sprite_9p& sprite_9p::assign(const sprite_9p& other) {
        if ( this != &other ) {
            sprite_9p s;
            s.texrect_ = other.texrect_;
            s.inner_rect_ = other.inner_rect_;
            s.texture_ = other.texture_;
            swap(s);
        }
        return *this;
    }

    sprite_9p& sprite_9p::set_texrect(const b2f& value) noexcept {
        texrect_ = value;
        return *this;
    }
    
    sprite_9p& sprite_9p::set_inner_texrect(const b2f& value) noexcept {
        inner_rect_ = value;
        return *this;
    }

    sprite_9p& sprite_9p::set_texture(const texture_asset::ptr& value) noexcept {
        texture_ = value;
        return *this;
    }

    const b2f& sprite_9p::texrect() const noexcept {
        return texrect_;
    }
    
    const b2f& sprite_9p::inner_texrect() const noexcept {
        return inner_rect_;
    }

    const texture_asset::ptr& sprite_9p::texture() const noexcept {
        return texture_;
    }
}

namespace e2d
{
    void swap(sprite_9p& l, sprite_9p& r) noexcept {
        l.swap(r);
    }

    bool operator==(const sprite_9p& l, const sprite_9p& r) noexcept {
        return l.inner_texrect() == r.inner_texrect()
            && l.texrect() == r.texrect()
            && l.texture() == r.texture();
    }

    bool operator!=(const sprite_9p& l, const sprite_9p& r) noexcept {
        return !(l == r);
    }
}

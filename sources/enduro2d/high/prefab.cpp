/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/prefab.hpp>

namespace e2d
{
    prefab::prefab(prefab&& other) noexcept {
        assign(std::move(other));
    }

    prefab& prefab::operator=(prefab&& other) noexcept {
        return assign(std::move(other));
    }

    prefab::prefab(const prefab& other) {
        assign(other);
    }

    prefab& prefab::operator=(const prefab& other) {
        return assign(other);
    }

    void prefab::clear() noexcept {
        prototype_.clear();
        children_.clear();
    }

    void prefab::swap(prefab& other) noexcept {
        using std::swap;
        swap(prototype_, other.prototype_);
        swap(children_, other.children_);
    }

    prefab& prefab::assign(prefab&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    prefab& prefab::assign(const prefab& other) {
        if ( this != &other ) {
            prefab s;
            s.prototype_ = other.prototype_;
            s.children_ = other.children_;
            swap(s);
        }
        return *this;
    }

    prefab& prefab::set_prototype(ecs::prototype&& proto) noexcept {
        prototype_ = std::move(proto);
        return *this;
    }

    prefab& prefab::set_prototype(const ecs::prototype& proto) {
        prototype_ = proto;
        return *this;
    }

    prefab& prefab::set_children(vector<prefab>&& children) noexcept {
        children_ = std::move(children);
        return *this;
    }

    prefab& prefab::set_children(const vector<prefab>& children) {
        children_ = children;
        return *this;
    }

    ecs::prototype& prefab::prototype() noexcept {
        return prototype_;
    }

    const ecs::prototype& prefab::prototype() const noexcept {
        return prototype_;
    }
    
    vector<prefab>& prefab::children() noexcept {
        return children_;
    }

    const vector<prefab>& prefab::children() const noexcept {
        return children_;
    }
}

namespace e2d
{
    void swap(prefab& l, prefab& r) noexcept {
        l.swap(r);
    }

    bool operator==(const prefab& l, const prefab& r) noexcept {
        return l.prototype().empty() && l.children().empty()
            && r.prototype().empty() && r.children().empty();
    }

    bool operator!=(const prefab& l, const prefab& r) noexcept {
        return !(l == r);
    }
}

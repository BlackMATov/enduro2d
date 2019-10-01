/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

namespace e2d
{
    class prefab final {
    public:
        prefab() = default;
        ~prefab() noexcept = default;

        prefab(prefab&& other) noexcept;
        prefab& operator=(prefab&& other) noexcept;

        prefab(const prefab& other);
        prefab& operator=(const prefab& other);

        void clear() noexcept;
        void swap(prefab& other) noexcept;

        prefab& assign(prefab&& other) noexcept;
        prefab& assign(const prefab& other);

        prefab& set_prototype(ecs::prototype&& proto) noexcept;
        prefab& set_prototype(const ecs::prototype& proto);

        prefab& set_children(vector<prefab>&& children) noexcept;
        prefab& set_children(const vector<prefab>& children);

        ecs::prototype& prototype() noexcept;
        const ecs::prototype& prototype() const noexcept;
        
        vector<prefab>& children() noexcept;
        const vector<prefab>& children() const noexcept;
    private:
        ecs::prototype prototype_;
        vector<prefab> children_;
    };

    void swap(prefab& l, prefab& r) noexcept;
    bool operator==(const prefab& l, const prefab& r) noexcept;
    bool operator!=(const prefab& l, const prefab& r) noexcept;
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

#include "prefab.hpp"
#include "gobject.hpp"
#include "node.hpp"

namespace e2d
{
    class world final : public module<world> {
    public:
        world() = default;
        ~world() noexcept final;

        ecs::registry& registry() noexcept;
        const ecs::registry& registry() const noexcept;

        gobject_iptr instantiate();
        gobject_iptr instantiate(const prefab& prefab);
        void destroy_instance(const gobject_iptr& inst) noexcept;

        gobject_iptr resolve(ecs::entity_id ent) const noexcept;
        gobject_iptr resolve(const ecs::const_entity& ent) const noexcept;

        vector<gobject_iptr> find_gobject(str_hash name) const noexcept;
        node_iptr find_child_node(const node_iptr& root, str_hash name) const noexcept;
        node_iptr find_root_node(const node_iptr& child, str_hash name) const noexcept;
    private:
        ecs::registry registry_;
        hash_map<ecs::entity_id, gobject_iptr> gobjects_;
    };
}

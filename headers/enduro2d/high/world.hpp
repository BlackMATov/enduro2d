/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

#include "prefab.hpp"
#include "gobject.hpp"

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
    private:
        ecs::registry registry_;
        hash_map<ecs::entity_id, gobject_iptr> gobjects_;
    };
}

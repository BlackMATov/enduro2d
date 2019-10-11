/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/world.hpp>

#include <enduro2d/high/node.hpp>
#include <enduro2d/high/components/actor.hpp>

#include <enduro2d/high/single_components/name_map_comp.hpp>

namespace e2d
{
    world::~world() noexcept {
        while ( !gobjects_.empty() ) {
            destroy_instance(gobjects_.begin()->second);
        }
    }

    ecs::registry& world::registry() noexcept {
        return registry_;
    }

    const ecs::registry& world::registry() const noexcept {
        return registry_;
    }

    gobject_iptr world::instantiate() {
        auto inst = make_intrusive<gobject>(registry_);
        gobjects_.emplace(inst->entity().id(), inst);

        try {
            auto inst_n = node::create(inst);
            auto inst_a = inst->get_component<actor>();
            if ( inst_a && inst_a->node() ) {
                inst_n->transform(inst_a->node()->transform());
            }
            inst_a.assign(inst_n);
        } catch (...) {
            destroy_instance(inst);
            throw;
        }

        return inst;
    }

    gobject_iptr world::instantiate(const prefab& prefab) {
        auto inst = make_intrusive<gobject>(registry_, prefab.prototype());
        gobjects_.emplace(inst->entity().id(), inst);

        try {
            auto inst_a = inst->get_component<actor>();
            auto inst_n = node::create_copy(inst, inst_a ? inst_a->node() : nullptr);
            inst_a.assign(inst_n);
        } catch (...) {
            destroy_instance(inst);
            throw;
        }

        try {
            for ( const auto& child_prefab : prefab.children() ) {
                auto child = instantiate(child_prefab);
                try {
                    auto inst_a = inst->get_component<actor>();
                    auto child_a = child->get_component<actor>();
                    inst_a->node()->add_child(child_a->node());
                } catch (...) {
                    destroy_instance(child);
                    throw;
                }
            }
        } catch (...) {
            destroy_instance(inst);
            throw;
        }

        return inst;
    }

    void world::destroy_instance(const gobject_iptr& inst) noexcept {
        node_iptr inst_n = inst && inst->get_component<actor>()
            ? inst->get_component<actor>()->node()
            : nullptr;
        if ( inst_n ) {
            inst_n->for_each_child([this](const node_iptr& child_n){
                destroy_instance(child_n->owner());
            });
        }
        if ( inst ) {
            inst->entity().remove_all_components();
            gobjects_.erase(inst->entity().id());
        }
    }

    gobject_iptr world::resolve(ecs::entity_id ent) const noexcept {
        E2D_ASSERT(registry_.valid_entity(ent));
        const auto iter = gobjects_.find(ent);
        return iter != gobjects_.end()
            ? iter->second
            : nullptr;
    }

    gobject_iptr world::resolve(const ecs::const_entity& ent) const noexcept {
        E2D_ASSERT(registry_.valid_entity(ent));
        const auto iter = gobjects_.find(ent.id());
        return iter != gobjects_.end()
            ? iter->second
            : nullptr;
    }

    vector<gobject_iptr> world::find_gobject(str_hash name) const noexcept {
        auto* name_map = registry_.find_single_component<name_map_comp>();
        if ( !name_map ) {
            return {};
        }
        return name_map->find_all(name);
    }

    node_iptr world::find_child_node(const node_iptr& root, str_hash name) const noexcept {
        if ( !root ) {
            return nullptr;
        }
        vector<node_iptr> pending;
        vector<node_iptr> temp;
        pending.push_back(root);

        for (; !pending.empty(); ) {
            auto curr = pending.front();
            pending.erase(pending.begin());

            if ( auto* comp = curr->owner()->entity().find_component<name_comp>() ) {
                if ( comp->name() == name ) {
                    return curr;
                }
            }

            curr->for_each_child([&temp](const node_iptr& n) {
                temp.push_back(n);
            });

            for ( auto i = temp.rbegin(); i != temp.rend(); ++i ) {
                pending.push_back(*i);
            }
            temp.clear();
        }
        return nullptr;
    }

    node_iptr world::find_root_node(const node_iptr& child, str_hash name) const noexcept {
        if ( !child ) {
            return nullptr;
        }
        for (node_iptr n = child->parent(); n; n = n->parent()) {
            if ( auto* comp = n->owner()->entity().find_component<name_comp>() ) {
                if ( comp->name() == name ) {
                    return n;
                }
            }
        }
        return nullptr;
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/render_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/scene.hpp>
#include <enduro2d/high/components/scissor_comp.hpp>

#include "render_system_impl/render_system_base.hpp"
#include "render_system_impl/render_system_batcher.hpp"
#include "render_system_impl/render_system_drawer.hpp"

namespace
{
    using namespace e2d;
    using namespace e2d::render_system_impl;

    void for_each_by_nodes(const const_node_iptr& root, drawer::context& ctx) {
        struct item {
            const_node_iptr node;
            u32 depth;
        };
        struct scissor {
            b2u rect;
            u32 depth;
        };
        static vector<item> pending_nodes, temp_nodes;
        static vector<scissor> scissor_stack;
        try {
            scissor_stack.clear();
            scissor_stack.push_back({b2u(~0u, ~0u), 0});
            pending_nodes.push_back({root, 1});

            for (; !pending_nodes.empty(); ) {
                auto curr = pending_nodes.back();
                pending_nodes.pop_back();
                
                if ( scissor_stack.back().depth == curr.depth ) {
                    scissor_stack.pop_back();
                    E2D_ASSERT(!scissor_stack.empty());
                    ctx.set_scissor(scissor_stack.back().rect);
                }
                if ( auto* sc = curr.node->owner()->entity().find_component<scissor_comp>() ) {
                    scissor_stack.push_back({sc->rect(), curr.depth});
                    ctx.set_scissor(scissor_stack.back().rect);
                }

                ctx.draw(curr.node);
                
                curr.node->for_each_child([&curr](const const_node_iptr& n) {
                    temp_nodes.push_back({n, curr.depth + 1});
                });

                for ( auto i = temp_nodes.rbegin(); i != temp_nodes.rend(); ++i ) {
                    pending_nodes.push_back(*i);
                }
                temp_nodes.clear();
            }
        } catch (...) {
            pending_nodes.clear();
            temp_nodes.clear();
            throw;
        }
        pending_nodes.clear();
        temp_nodes.clear();
    }

    template < typename T, typename Comp, typename F >
    void for_each_by_sorted_components(ecs::registry& owner, Comp&& comp, F&& f) {
        static vector<std::pair<ecs::const_entity,T>> temp_components;
        try {
            temp_components.reserve(owner.component_count<T>());
            owner.for_each_component<T>([](const ecs::const_entity& e, const T& t){
                temp_components.emplace_back(e, t);
            });
            std::sort(
                temp_components.begin(),
                temp_components.end(),
                [&comp](const auto& l, const auto& r){
                    return comp(l.second, r.second);
                });
            for ( auto& p : temp_components ) {
                f(p.first, p.second);
            }
        } catch (...) {
            temp_components.clear();
            throw;
        }
        temp_components.clear();
    }

    void for_all_scenes(drawer::context& ctx, ecs::registry& owner) {
        const auto comp = [](const scene& l, const scene& r) noexcept {
            return l.depth() < r.depth();
        };
        const auto func = [&ctx](const ecs::const_entity& scn_e, const scene&) {
            const actor* scn_a = scn_e.find_component<actor>();
            if ( scn_a && scn_a->node() ) {
                for_each_by_nodes(scn_a->node(), ctx);
            }
        };
        for_each_by_sorted_components<scene>(owner, comp, func);
    }

    void for_all_cameras(drawer& drawer, ecs::registry& owner) {
        const auto comp = [](const camera& l, const camera& r) noexcept {
            return l.depth() < r.depth();
        };
        const auto func = [&drawer, &owner](const ecs::const_entity& cam_e, const camera& cam) {
            const actor* const cam_a = cam_e.find_component<actor>();
            const const_node_iptr cam_n = cam_a ? cam_a->node() : nullptr;
            drawer.with(cam, cam_n, [&owner](drawer::context& ctx){
                for_all_scenes(ctx, owner);
            });
        };
        for_each_by_sorted_components<camera>(owner, comp, func);
    }
}

namespace e2d
{
    //
    // render_system::internal_state
    //

    class render_system::internal_state final : private noncopyable {
    public:
        internal_state()
        : drawer_(the<engine>(), the<debug>(), the<render>(), the<window>()) {}
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner) {
            for_all_cameras(drawer_, owner);
        }
    private:
        drawer drawer_;
    };

    //
    // render_system
    //

    render_system::render_system()
    : state_(new internal_state()) {}
    render_system::~render_system() noexcept = default;

    void render_system::process(ecs::registry& owner) {
        state_->process(owner);
    }
}

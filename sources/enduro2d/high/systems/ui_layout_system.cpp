/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_layout_system.hpp>
#include <enduro2d/high/components/ui_layout.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/shape2d.hpp>
#include <enduro2d/high/node.hpp>

using namespace e2d;

namespace
{
    void join_rect(b2f& l, const b2f& r) noexcept {
        const v2f l1 = l.position + l.size;
        const v2f r1 = r.position + r.size;
        l.position.x = math::min(l.position.x, r.position.x);
        l.position.y = math::min(l.position.y, r.position.y);
        f32 ax = math::max(l1.x, r1.x);
        f32 ay = math::max(l1.y, r1.y);
        l.size.x = ax - l.position.x;
        l.size.y = ay - l.position.y;
    }

    void update_fixed_layout(
        ecs::entity& e,
        b2f& inout_region,
        const node_iptr&,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<fixed_layout>();

        inout_region.size = layout.size();
    }
    
    void update_auto_layout2(
        ecs::entity& e,
        b2f& inout_region,
        const node_iptr&,
        std::vector<ui_layout::layout_state>& childs)
    {
        if ( childs.empty() ) {
            inout_region = b2f() + inout_region.position;
            return;
        }
        inout_region = childs.front().region;
        for ( size_t i = 1; i < childs.size(); ++i ) {
            join_rect(inout_region, childs[i].region);
        }
    }

    void update_auto_layout(
        ecs::entity& e,
        b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        for ( auto& c : childs ) {
            c.region = parent_rect;
        }
        childs.push_back({
            e.id(),
            &update_auto_layout2,
            node,
            parent_rect});
    }

    void update_stack_layout(
        ecs::entity& e,
        b2f& inout_region,
        const node_iptr&,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<stack_layout>();

        const v2f max_size = inout_region.size;
        v2f offset;
        b2f local_r;

        for (auto& c : childs ) {
            auto& cell_r = c.region;

            switch ( layout.origin() ) {
                case stack_layout::stack_origin::left:
                    cell_r += v2f(offset.x, 0.f);
                    offset.x += cell_r.size.x;
                    break;
                case stack_layout::stack_origin::right:
                    cell_r += v2f(max_size.x - offset.x, 0.f);
                    offset.x += cell_r.size.x;
                    break;
                case stack_layout::stack_origin::bottom:
                    cell_r += v2f(0.0f, offset.y);
                    offset.y += cell_r.size.y;
                    break;
                case stack_layout::stack_origin::top:
                    cell_r += v2f(0.0f, max_size.y - offset.y);
                    offset.y += cell_r.size.y;
                    break;
            }
            if ( &c != childs.data() ) {
                join_rect(local_r, cell_r);
            } else {
                local_r = cell_r;
            }
        }
        inout_region = local_r;
    }

    void update_fill_stack_layout(
        ecs::entity& e,
        b2f& inout_region,
        const node_iptr&,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<fill_stack_layout>();
    }

    void update_dock_layout(
        ecs::entity& e,
        b2f& inout_region,
        const node_iptr&,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<dock_layout>();
    }
}

namespace
{
    b2f unproject(const m4f& mvp, const b2f& viewport) noexcept {
        const auto inv_mvp_opt = math::inversed(mvp);
        const m4f inv_mvp = inv_mvp_opt.second
            ? inv_mvp_opt.first
            : m4f::identity();

        const f32 z = 0.0f;
        const v3f points[] = {
            math::unproject(
                v3f(viewport.position.x, viewport.position.y, z),
                inv_mvp, viewport),
            math::unproject(
                v3f(viewport.position.x, viewport.position.y + viewport.size.y, z),
                inv_mvp, viewport),
            math::unproject(
                v3f(viewport.position.x + viewport.size.x, viewport.position.y, z),
                inv_mvp, viewport),
            math::unproject(
                v3f(viewport.position.x + viewport.size.x, viewport.position.y + viewport.size.y, z),
                inv_mvp, viewport)
        };

        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        return b2f(min, max - min);
    }

    b2f project_to_parent(const node_iptr& n, const b2f& r) noexcept {
        const m4f m = n->local_matrix();
        const v4f points[] = {
            v4f(r.position.x,            r.position.y,            0.0f, 1.0f) * m,
            v4f(r.position.x,            r.position.y + r.size.y, 0.0f, 1.0f) * m,
            v4f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f, 1.0f) * m,
            v4f(r.position.x + r.size.x, r.position.y,            0.0f, 1.0f) * m
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        return b2f(min, max - min);
    }

    void project_to_local(const node_iptr& n, ui_layout& layout, const b2f& r) noexcept {
        // TODO:
        // - rotation is not supported
        const auto inv_opt = math::inversed(n->local_matrix());
        const m4f inv = inv_opt.second
            ? inv_opt.first
            : m4f::identity();
        const v4f points[] = {
            v4f(r.position.x,            r.position.y,            0.0f, 1.0f) * inv,
            v4f(r.position.x,            r.position.y + r.size.y, 0.0f, 1.0f) * inv,
            v4f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f, 1.0f) * inv,
            v4f(r.position.x + r.size.x, r.position.y,            0.0f, 1.0f) * inv
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        n->translation(n->translation() + v3f(min.x, min.y, 0.0f));
        layout.size(max - min);
    }

    b2f get_bounding_box(const node_iptr& root) {
        if ( auto* rect = root->owner()->entity().find_component<rectangle_shape>() ) {
            return rect->rectangle();
        }

        if ( auto* circle = root->owner()->entity().find_component<circle_shape>() ) {
            return b2f(v2f(circle->radius())) + circle->center();
        }

        return b2f();
    }

    std::tuple<m4f,b2f> get_vp_and_viewport(
        const ecs::const_entity& e,
        const camera& cam) noexcept
    {
        const actor* cam_a = e.find_component<actor>();
        const m4f& cam_w = cam_a && cam_a->node()
            ? cam_a->node()->world_matrix()
            : m4f::identity();
        const auto cam_w_inv = math::inversed(cam_w);
        const m4f& m_v = cam_w_inv.second
            ? cam_w_inv.first
            : m4f::identity();
        const m4f& m_p = cam.projection();

        return std::make_tuple(
            m_v *  m_p,
            cam.viewport().cast_to<f32>());
    }

    void update_layouts(ecs::registry& owner) {
        node_iptr root;
        owner.for_joined_components<ui_layout::root_tag, actor>(
        [&root](const ecs::const_entity&, ui_layout::root_tag, actor& act) {
            root = act.node();
        });

        if ( !root ) {
            return;
        }

        // get screen size
        std::tuple<m4f,b2f> vp_and_viewport;
        owner.for_joined_components<camera>(
        [&vp_and_viewport](const ecs::const_entity& e, const camera& cam) {
            if ( cam.target() ) {
                return;
            }
            vp_and_viewport = get_vp_and_viewport(e, cam);
        });
        
        std::vector<ui_layout::layout_state> temp_layouts;
        std::vector<ui_layout::layout_state> pending;

        temp_layouts.reserve(64);
        pending.reserve(512);
        {
            const m4f mvp = root->world_matrix() * std::get<0>(vp_and_viewport);
            const b2f bbox = unproject(mvp, std::get<1>(vp_and_viewport));
            auto& e = root->owner()->entity();
            const ui_layout& layout = e.get_component<ui_layout>();

            pending.push_back({
                e.id(),
                layout.update_fn(),
                root,
                bbox});
        }

        for (; !pending.empty(); ) {
            auto curr = pending.back();
            pending.pop_back();

            ecs::entity e(owner, curr.id);
            actor& act = e.get_component<actor>();
            node_iptr node = act.node();
            ui_layout& layout = e.get_component<ui_layout>();
            
            node->for_each_child([&temp_layouts](const node_iptr& n) {
                auto& e = n->owner()->entity();
                ui_layout& layout = e.get_component<ui_layout>();
                b2f bbox = project_to_parent(n, b2f(layout.size()));

                temp_layouts.push_back({
                    e.id(),
                    layout.update_fn(),
                    n,
                    bbox});
            });

            if ( curr.update ) {
                curr.update(e, curr.region, curr.node, temp_layouts);
            }
            project_to_local(node, layout, curr.region);

            for ( auto i = temp_layouts.rbegin(); i != temp_layouts.rend(); ++i ) {
                pending.push_back(*i);
            }
            temp_layouts.clear();
        }
    }

    void register_update_fn(ecs::registry& owner) {
        owner.for_joined_components<fixed_layout::dirty_flag, fixed_layout, ui_layout>(
        [](const ecs::entity&, fixed_layout::dirty_flag, const fixed_layout& fl, ui_layout& layout) {
            layout.update_fn(&update_fixed_layout);
            layout.size(fl.size());
        });
        owner.remove_all_components<fixed_layout::dirty_flag>();
        
        owner.for_joined_components<auto_layout::dirty_flag, auto_layout, ui_layout>(
        [](const ecs::entity&, auto_layout::dirty_flag, const auto_layout&, ui_layout& layout) {
            layout.update_fn(&update_auto_layout);
        });
        owner.remove_all_components<auto_layout::dirty_flag>();
        
        owner.for_joined_components<stack_layout::dirty_flag, stack_layout, ui_layout>(
        [](const ecs::entity&, stack_layout::dirty_flag, const stack_layout&, ui_layout& layout) {
            layout.update_fn(&update_stack_layout);
        });
        owner.remove_all_components<stack_layout::dirty_flag>();
        
        owner.for_joined_components<fill_stack_layout::dirty_flag, fill_stack_layout, ui_layout>(
        [](const ecs::entity&, fill_stack_layout::dirty_flag, const fill_stack_layout&, ui_layout& layout) {
            layout.update_fn(&update_fill_stack_layout);
        });
        owner.remove_all_components<fill_stack_layout::dirty_flag>();
        
        owner.for_joined_components<dock_layout::dirty_flag, dock_layout, ui_layout>(
        [](const ecs::entity&, dock_layout::dirty_flag, const dock_layout&, ui_layout& layout) {
            layout.update_fn(&update_dock_layout);
        });
        owner.remove_all_components<dock_layout::dirty_flag>();
    }
}

namespace e2d
{
    ui_layout_system::ui_layout_system() = default;

    ui_layout_system::~ui_layout_system() noexcept = default;

    void ui_layout_system::process(ecs::registry& owner) {
        register_update_fn(owner);
        update_layouts(owner);
    }
}

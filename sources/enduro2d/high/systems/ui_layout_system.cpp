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

    v2f project_to_parent(const node_iptr& n, const v2f& p) noexcept {
        const m4f m = n->local_matrix();
        return v2f(v4f(p.x, p.y, 0.0f, 1.0f) * m);
    }

    b2f project_to_local(const node_iptr& n, const b2f& r) noexcept {
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
        return b2f(min, max - min);
    }

    v2f project_to_local(const node_iptr& n, const v2f& p) noexcept {
        const auto inv_opt = math::inversed(n->local_matrix());
        const m4f inv = inv_opt.second
            ? inv_opt.first
            : m4f::identity();
        return v2f(v4f(p.x, p.y, 0.0f, 1.0f) * inv);
    }
    
    void update_auto_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<ui_layout>();
        b2f region;

        if ( !childs.empty() ) {
            // project childs into auto-layout space and join regions
            region = project_to_parent(childs.front().node, b2f(childs.front().layout->size()));

            for ( size_t i = 1; i < childs.size(); ++i ) {
                b2f r = project_to_parent(childs[i].node, b2f(childs[i].layout->size()));
                join_rect(region, r);
            }
            
            // update layout size and node position
            node->translation(node->translation() + v3f(region.position, 0.0f));
            layout.size(region.size);

            // update child transformation
            for ( auto& c : childs ) {
                v2f off = project_to_local(c.node, v2f()) -
                    project_to_local(c.node, v2f(node->translation()));
                c.node->translation(c.node->translation() + v3f(off, 0.0f) * c.node->scale());
            }
        } else {
            layout.size(v2f());
        }
    }

    void update_auto_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        // project into auto-layout space
        b2f local = project_to_local(node, parent_rect);
        auto& layout = e.get_component<ui_layout>();

        for ( auto& c : childs ) {
            c.parent_rect = local;
        }
        childs.push_back({
            e.id(),
            &update_auto_layout2,
            node,
            &layout,
            parent_rect});
    }
    
    void update_stack_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& sl = e.get_component<stack_layout>();
        auto& layout = e.get_component<ui_layout>();
        const b2f local = project_to_local(node, parent_rect);
        
        // project childs into stack layout and calculate max size
        v2f max_size;
        std::vector<v2f> projected(childs.size());
        for ( size_t i = 0; i < childs.size(); ++i ) {
            v2f p = project_to_parent(childs[i].node, b2f(childs[i].layout->size())).size;
            projected[i] = p;
            max_size += p;
        }
        max_size.x = math::min(max_size.x, local.size.x);
        max_size.y = math::min(max_size.y, local.size.y);
        
        v2f offset;
        b2f local_r;
        for ( size_t i = 0; i < childs.size(); ++i ) {
            auto& c = childs[i];
            b2f item_r(projected[i]);

            // place into stack
            switch ( sl.origin() ) {
                case stack_layout::stack_origin::left:
                    item_r += v2f(offset.x, 0.f);
                    offset.x += item_r.size.x;
                    break;
                case stack_layout::stack_origin::right:
                    item_r += v2f(max_size.x - offset.x - item_r.size.x, 0.f);
                    offset.x += item_r.size.x;
                    break;
                case stack_layout::stack_origin::bottom:
                    item_r += v2f(0.0f, offset.y);
                    offset.y += item_r.size.y;
                    break;
                case stack_layout::stack_origin::top:
                    item_r += v2f(0.0f, max_size.y - offset.y - item_r.size.y);
                    offset.y += item_r.size.y;
                    break;
            }
            // project back into child local space and set position
            b2f bbox = project_to_local(c.node, item_r);
            c.node->translation(c.node->translation() + v3f(bbox.position, 0.0f) * c.node->scale());

            if ( &c != childs.data() ) {
                join_rect(local_r, item_r);
            } else {
                local_r = item_r;
            }
        }
        
        layout.size(local_r.size);
    }

    void update_stack_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        bool post_update = false;
        for (auto& c : childs ) {
            post_update |= c.layout->post_update();
        }
        auto& layout = e.get_component<ui_layout>();

        if ( post_update ) {
            childs.push_back({
                e.id(),
                &update_stack_layout2,
                node,
                &layout,
                parent_rect});
            return;
        }

        update_stack_layout2(e, parent_rect, node, childs);
    }

    void update_dock_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        v2f size;
        if ( childs.size() ) {
            E2D_ASSERT(childs.size() == 1);
            size = project_to_parent(childs.front().node, b2f(childs.front().layout->size())).size;
        }

        using dock = dock_layout::dock_type;
        auto& dl = e.get_component<dock_layout>();
        auto& layout = e.get_component<ui_layout>();
        const b2f local = project_to_local(node, parent_rect);
        b2f region;

        // horizontal docking
        if ( dl.has_dock(dock::center_x) ) {
            region.position.x = (local.size.x - size.x) * 0.5f;
            region.size.x = size.x;
        } else if ( dl.has_dock(dock::left | dock::right) ) {
            region.position.x = 0.0f;
            region.size.x = local.size.x;
        } else if ( dl.has_dock(dock::left) ) {
            region.position.x = 0.0f;
            region.size.x = size.x;
        } else if ( dl.has_dock(dock::right) ) {
            region.position.x = local.size.x - size.x;
            region.size.x = size.x;
        } else {
            E2D_ASSERT_MSG(false, "undefined horizontal docking");
        }

        // vertical docking
        if ( dl.has_dock(dock::center_y) ) {
            region.position.y = (local.size.y - size.y) * 0.5f;
            region.size.y = size.y;
        } else if ( dl.has_dock(dock::top | dock::bottom) ) {
            region.position.y = 0.0f;
            region.size.y = local.size.y;
        } else if ( dl.has_dock(dock::bottom) ) {
            region.position.y = 0.0f;
            region.size.y = size.y;
        } else if ( dl.has_dock(dock::top) ) {
            region.position.y = local.size.y - size.y;
            region.size.y = size.y;
        } else {
            E2D_ASSERT_MSG(false, "undefined vertial docking");
        }
        
        node->translation(node->translation() + v3f(region.position, 0.0f));
        layout.size(region.size);
    }

    void update_dock_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        bool post_update = false;
        for (auto& c : childs ) {
            post_update |= c.layout->post_update();
        }
        auto& layout = e.get_component<ui_layout>();

        if ( post_update ) {
            childs.push_back({
                e.id(),
                &update_dock_layout2,
                node,
                &layout,
                parent_rect});
            return;
        }

        update_dock_layout2(e, parent_rect, node, childs);
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
                &layout,
                bbox});
        }

        for (; !pending.empty(); ) {
            auto curr = pending.back();
            pending.pop_back();

            ecs::entity e(owner, curr.id);
            actor& act = e.get_component<actor>();
            node_iptr node = act.node();
            const ui_layout& layout = e.get_component<ui_layout>();
            const b2f parent_rect(layout.size());
            
            node->for_each_child([&temp_layouts, &parent_rect](const node_iptr& n) {
                auto& e = n->owner()->entity();
                const ui_layout& layout = e.get_component<ui_layout>();

                temp_layouts.push_back({
                    e.id(),
                    layout.update_fn(),
                    n,
                    &layout,
                    parent_rect});
            });

            if ( curr.update ) {
                curr.update(e, curr.parent_rect, curr.node, temp_layouts);
            }

            for ( auto i = temp_layouts.rbegin(); i != temp_layouts.rend(); ++i ) {
                pending.push_back(*i);
            }
            temp_layouts.clear();
        }
    }

    void register_update_fn(ecs::registry& owner) {
        owner.for_joined_components<fixed_layout::dirty_flag, fixed_layout, ui_layout>(
        [](const ecs::entity&, fixed_layout::dirty_flag, const fixed_layout& fl, ui_layout& layout) {
            layout.size(fl.size());
        });
        owner.remove_all_components<fixed_layout::dirty_flag>();
        
        owner.for_joined_components<auto_layout::dirty_flag, auto_layout, ui_layout>(
        [](const ecs::entity&, auto_layout::dirty_flag, const auto_layout&, ui_layout& layout) {
            layout.update_fn(&update_auto_layout);
            layout.post_update(true);
        });
        owner.remove_all_components<auto_layout::dirty_flag>();
        
        owner.for_joined_components<stack_layout::dirty_flag, stack_layout, ui_layout>(
        [](const ecs::entity&, stack_layout::dirty_flag, const stack_layout&, ui_layout& layout) {
            layout.update_fn(&update_stack_layout);
            layout.post_update(true);
        });
        owner.remove_all_components<stack_layout::dirty_flag>();
        
        owner.for_joined_components<dock_layout::dirty_flag, dock_layout, ui_layout>(
        [](const ecs::entity&, dock_layout::dirty_flag, const dock_layout&, ui_layout& layout) {
            layout.update_fn(&update_dock_layout);
            layout.post_update(true); // TODO ???
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

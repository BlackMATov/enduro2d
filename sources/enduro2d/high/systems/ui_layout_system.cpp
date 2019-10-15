/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_system.hpp>

#include <enduro2d/high/components/ui_layout.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/label.hpp>
#include <enduro2d/high/components/shape2d.hpp>
#include <enduro2d/high/components/scissor_comp.hpp>
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
        const v3f off = v3f(n->size() * n->pivot(), 0.0f);
        const auto tr = [&n, off](const v2f& p) {
            return v2f(
                ((v3f(p, 0.0f) - off) * n->transform().scale)
                * n->transform().rotation
                + off);
        };
        const v2f points[] = {
            tr(v2f(r.position.x,            r.position.y           )),
            tr(v2f(r.position.x,            r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y           ))
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

    b2f project_to_local(const node_iptr& n, const b2f& r) noexcept {
        const v3f off = v3f(n->size() * n->pivot(), 0.0f);
        const q4f inv_rot = math::inversed(n->transform().rotation);
        const v3f inv_scale = 1.0f / n->transform().scale;

        const auto tr = [inv_rot, inv_scale, off](const v2f& p) {
            return v2f(
                ((v3f(p, 0.0f) - off) * inv_rot)
                * inv_scale
                + off);
        };
        const v2f points[] = {
            tr(v2f(r.position.x,            r.position.y           )),
            tr(v2f(r.position.x,            r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y + r.size.y)),
            tr(v2f(r.position.x + r.size.x, r.position.y           ))
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        v2f aabb_size = max - min;
        f32 scale = 1.0f;
        if ( !math::is_near_zero(aabb_size.x) && !math::is_near_zero(aabb_size.y) ) {
            scale = math::min(
                math::abs(r.size.x / aabb_size.x),
                math::abs(r.size.y / aabb_size.y));
        }
        return b2f(
            math::length(points[3] - points[0]) * scale,
            math::length(points[1] - points[0]) * scale);
    }
    
    void update_fixed_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>&)
    {
        const b2f local = project_to_local(node, parent_rect);
        const auto& fl = e.get_component<fixed_layout>();

        node->translation(v3f(parent_rect.position + fl.position(), 0.0f));
    }

    void update_auto_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& al = e.get_component<auto_layout>();

        // project childs into auto-layout space and join regions
        bool first = true;
        b2f region;
        for ( auto& c : childs ) {
            if ( c.depends_on_parent ) {
                continue;
            }
            b2f r = project_to_parent(c.node, b2f(c.node->size()))
                + v2f(c.node->translation());
            if ( !first ) {
                join_rect(region, r);
            } else {
                first = false;
                region = r;
            }
        }

        region.size = math::minimized(region.size, al.max_size());
        region.size = math::maximized(region.size, al.min_size());
        node->size(region.size);
        node->translation(v3f(region.position + parent_rect.position, 0.0f));

        // update child transformation
        for ( auto& c : childs ) {
            c.parent_rect = b2f(-region.position, node->size());
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
            parent_rect,
            true,
            layout.depends_on_childs(),
            layout.depends_on_parent()});
    }

    void update_stack_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        using origin = stack_layout::stack_origin;
        auto& sl = e.get_component<stack_layout>();
        const b2f local = project_to_local(node, parent_rect);
        
        // project childs into stack layout and calculate max size
        const f32 spacing_sum = sl.spacing() * (math::max<size_t>(1, childs.size()) - 1);
        v2f max_size;
        v2f items_sum;
        std::vector<b2f> projected(childs.size());
        for ( size_t i = 0; i < childs.size(); ++i ) {
            b2f r = project_to_parent(childs[i].node, b2f(childs[i].node->size()));
            projected[i] = r;
            items_sum += r.size;
            max_size = math::maximized(max_size, r.size);
        }
        switch ( sl.origin() ) {
            case origin::left:
            case origin::right: items_sum.x += spacing_sum; break;
            case origin::bottom:
            case origin::top: items_sum.y += spacing_sum; break;
        }
        
        v2f offset;
        b2f local_r;
        for ( size_t i = 0; i < childs.size(); ++i ) {
            auto& c = childs[i];
            b2f item_r(projected[i].size);
            v2f cell_size;

            // place into stack
            switch ( sl.origin() ) {
                case origin::left:
                    item_r += v2f(offset.x, 0.f);
                    offset.x += item_r.size.x + sl.spacing();
                    cell_size = v2f(item_r.size.x, max_size.y);
                    break;
                case origin::right:
                    item_r += v2f(items_sum.x - offset.x - item_r.size.x, 0.f);
                    offset.x += item_r.size.x + sl.spacing();
                    cell_size = v2f(item_r.size.x, max_size.y);
                    break;
                case origin::bottom:
                    item_r += v2f(0.0f, offset.y);
                    offset.y += item_r.size.y + sl.spacing();
                    cell_size = v2f(max_size.x, item_r.size.y);
                    break;
                case origin::top:
                    item_r += v2f(0.0f, items_sum.y - offset.y - item_r.size.y);
                    offset.y += item_r.size.y + sl.spacing();
                    cell_size = v2f(max_size.x, item_r.size.y);
                    break;
            }
            
            const v2f pos = item_r.position - projected[i].position;
            c.parent_rect = b2f(pos, cell_size);

            if ( &c != childs.data() ) {
                join_rect(local_r, item_r);
            } else {
                local_r = item_r;
            }
        }
        
        node->size(local_r.size);
    }

    void update_stack_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        const b2f local = project_to_local(node, parent_rect);
        bool post_update = false;
        for (auto& c : childs ) {
            c.parent_rect = local;
            post_update |= c.depends_on_childs;
        }

        if ( post_update ) {
            auto& layout = e.get_component<ui_layout>();
            childs.push_back({
                e.id(),
                &update_stack_layout2,
                node,
                parent_rect,
                true,
                layout.depends_on_childs(),
                layout.depends_on_parent()});
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
        // project childs into dock-layout space and join regions
        bool first = true;
        b2f child_region;
        for ( auto& c : childs ) {
            if ( c.depends_on_parent ) {
                continue;
            }
            b2f r = project_to_parent(c.node, b2f(c.node->size()))
                + v2f(c.node->translation());
            if ( !first ) {
                join_rect(child_region, r);
            } else {
                first = false;
                child_region = r;
            }
        }

        using dock = dock_layout::dock_type;
        auto& dl = e.get_component<dock_layout>();
        const b2f local = project_to_local(node, parent_rect);
        b2f region;

        // horizontal docking
        if ( dl.has_dock(dock::center_x) ) {
            region.position.x = (local.size.x - child_region.size.x) * 0.5f;
            region.size.x = child_region.size.x;
        } else if ( dl.has_dock(dock::fill_x) ) {
            region.position.x = 0.0f;
            region.size.x = local.size.x;
        } else if ( dl.has_dock(dock::left) ) {
            region.position.x = 0.0f;
            region.size.x = child_region.size.x;
        } else if ( dl.has_dock(dock::right) ) {
            region.position.x = local.size.x - child_region.size.x;
            region.size.x = child_region.size.x;
        } else {
            E2D_ASSERT_MSG(false, "undefined horizontal docking");
        }

        // vertical docking
        if ( dl.has_dock(dock::center_y) ) {
            region.position.y = (local.size.y - child_region.size.y) * 0.5f;
            region.size.y = child_region.size.y;
        } else if ( dl.has_dock(dock::fill_y) ) {
            region.position.y = 0.0f;
            region.size.y = local.size.y;
        } else if ( dl.has_dock(dock::bottom) ) {
            region.position.y = 0.0f;
            region.size.y = child_region.size.y;
        } else if ( dl.has_dock(dock::top) ) {
            region.position.y = local.size.y - child_region.size.y;
            region.size.y = child_region.size.y;
        } else {
            E2D_ASSERT_MSG(false, "undefined vertical docking");
        }
        
        v2f off = -project_to_parent(node, b2f(region.size)).position + parent_rect.position;
        node->translation(v3f(region.position + off, 0.0f));
        node->size(region.size);
        
        // update child transformation
        for ( auto& c : childs ) {
            c.parent_rect = b2f(-child_region.position, node->size());
        }
    }

    void update_dock_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        bool post_update = false;
        const b2f local = project_to_local(node, parent_rect);

        for (auto& c : childs ) {
            post_update |= c.depends_on_childs;
            c.parent_rect = local;
        }

        //if ( post_update ) {
            auto& layout = e.get_component<ui_layout>();
            childs.push_back({
                e.id(),
                &update_dock_layout2,
                node,
                parent_rect,
                true,
                layout.depends_on_childs(),
                layout.depends_on_parent()});
            return;
        //}
        //update_dock_layout2(e, parent_rect, node, childs);
    }

    void update_sized_dock_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        using dock = dock_layout::dock_type;
        auto& dl = e.get_component<sized_dock_layout>();
        const b2f local = project_to_local(node, parent_rect);
        const v2f size = dl.size();
        b2f region;

        // horizontal docking
        if ( dl.has_dock(dock::center_x) ) {
            region.position.x = (local.size.x - size.x) * 0.5f;
            region.size.x = size.x;
        } else if ( dl.has_dock(dock::fill_x) ) {
            region.position.x = 0.0f;
            region.size.x = local.size.x;
        } else if ( dl.has_dock(dock::left) ) {
            region.position.x = 0.0f;
            region.size.x = size.x;
        } else if ( dl.has_dock(dock::right) ) {
            region.position.x = local.size.x - size.x;
            region.size.x = size.x;
        } else {
            region.size.x = size.x;
        }

        // vertical docking
        if ( dl.has_dock(dock::center_y) ) {
            region.position.y = (local.size.y - size.y) * 0.5f;
            region.size.y = size.y;
        } else if ( dl.has_dock(dock::fill_y) ) {
            region.position.y = 0.0f;
            region.size.y = local.size.y;
        } else if ( dl.has_dock(dock::bottom) ) {
            region.position.y = 0.0f;
            region.size.y = size.y;
        } else if ( dl.has_dock(dock::top) ) {
            region.position.y = local.size.y - size.y;
            region.size.y = size.y;
        } else {
            region.size.y = size.y;
        }
        
        v2f off = -project_to_parent(node, b2f(region.size)).position + parent_rect.position;
        node->translation(v3f(region.position + off, 0.0f));
        node->size(region.size);

        for ( auto& c : childs ) {
            c.parent_rect = b2f(region.size);
        }
    }

    void update_image_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>&)
    {   
        if ( math::approximately(parent_rect.size, v2f()) ) {
            return;
        }
        
        node->scale(v3f(1.0f));

        auto& il = e.get_component<image_layout>();
        const b2f region = project_to_parent(node, b2f(il.size()));
        const v3f scale = il.preserve_aspect()
            ? v3f(math::minimum(parent_rect.size / region.size))
            : v3f(parent_rect.size / region.size, 1.0f);

        const v2f pos = -region.position * v2f(scale) + parent_rect.position;
        node->translation(v3f(pos, 0.0f));
        node->scale(scale);
    }

    void update_label_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>&)
    {
        auto& lbl = e.get_component<label>();
        const v2f old_size = node->size();

        node->size(lbl.preferred_size());
        node->translation(v3f(parent_rect.position, 0.0f));

        if ( !math::approximately(old_size, node->size(), 0.01f) ) {
            e.assign_component<label::dirty>();
        }
    }
    
    void update_label_autoscale_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>&)
    {
        auto& lbl = e.get_component<label>();
        
        if ( math::is_near_zero(lbl.preferred_size().x) ||
             math::is_near_zero(lbl.preferred_size().y) )
        {
            return;
        }
        
        auto& layout = e.get_component<label_autoscale_layout>();
        const b2f local = project_to_local(node, parent_rect);

        f32 scale = math::minimum(local.size / lbl.preferred_size());
        scale = math::clamp(scale, layout.min_scale(), layout.max_scale());

        const v2f old_size = node->size();

        node->size(local.size / scale);
        node->scale(v3f(scale));
        node->translation(v3f(parent_rect.position, 0.0f));

        if ( !math::approximately(old_size, node->size(), 0.01f) ) {
            e.assign_component<label::dirty>();
        }
    }

    void update_margin_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        if ( childs.empty() ) {
            return;
        }
        
        E2D_ASSERT(childs.size() == 1);
        auto& child = childs.front();
        const auto& ml = e.get_component<margin_layout>();

        const b2f region = project_to_parent(child.node, b2f(child.node->size()));
        node->size(region.size + v2f(ml.left() + ml.right(), ml.top() + ml.bottom()));
        node->translation(v3f(parent_rect.position, 0.0f));
    }

    void update_margin_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        const b2f local = project_to_local(node, parent_rect);
        const auto& ml = e.get_component<margin_layout>();
        bool post_update = false;

        for (auto& c : childs ) {
            c.parent_rect = b2f(v2f(ml.left(), ml.bottom()), local.size);
            post_update |= c.depends_on_childs;
        }

        if ( post_update ) {
            auto& layout = e.get_component<ui_layout>();
            childs.push_back({
                e.id(),
                &update_margin_layout2,
                node,
                parent_rect,
                true,
                layout.depends_on_childs(),
                layout.depends_on_parent()});
            return;
        }
        update_margin_layout2(e, parent_rect, node, childs);
    }

    void update_padding_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        const auto& pl = e.get_component<padding_layout>();
        const b2f local = project_to_local(node, parent_rect);
        const v2f pad_size = v2f(pl.left() + pl.right(), pl.bottom() + pl.top());

        if ( local.size.x > pad_size.x && local.size.y > pad_size.y ) {
            const v2f pos = v2f(pl.left(), pl.bottom()) + parent_rect.position;
            node->translation(v3f(pos, 0.0f));
            node->size(local.size - pad_size);
        } else {
            node->translation(v3f(parent_rect.position, 0.0f));
            node->size(v2f());
        }

        for ( auto& c : childs ) {
            c.parent_rect = b2f(node->size());
        }
    }

    void update_anchor_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& al = e.get_component<anchor_layout>();
        const b2f local = project_to_local(node, parent_rect);

        const v2f lb = local.size * al.left_bottom().position +
            (al.left_bottom().relative_offset
                ? local.size * al.left_bottom().offset
                : al.left_bottom().offset);
        const v2f rt = local.size * al.right_top().position +
            (al.right_top().relative_offset
                ? local.size * al.right_top().offset
                : al.right_top().offset);
        const v2f pos = lb + parent_rect.position;

        E2D_ASSERT((rt.x - lb.x) >= 0.0f);
        E2D_ASSERT((rt.y - lb.y) >= 0.0f);

        node->translation(v3f(pos, 0.0f));
        node->size(math::maximized(rt - lb, v2f(0.0f)));

        for ( auto& c : childs ) {
            c.parent_rect = b2f(node->size());
        }
    }
    
    void update_bounded_layout2(
        ecs::entity&,
        const b2f&,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        for ( auto& c : childs ) {
            b2f r = project_to_parent(c.node, b2f(c.node->size())) + v2f(c.node->translation());

            r.position = math::maximized(r.position, v2f(0.0f));
            r.position = math::minimized(r.position + r.size, node->size()) - r.size;

            // todo
            c.node->translation(v3f(r.position, 0.0f));
        }
    }

    void update_bounded_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        const b2f local = project_to_local(node, parent_rect);
        node->size(local.size);

        bool post_update = false;
        for ( auto& c : childs ) {
            c.parent_rect = b2f(node->size());
            post_update |= c.depends_on_childs;
        }

        if ( post_update ) {
            auto& layout = e.get_component<ui_layout>();
            childs.push_back({
                e.id(),
                &update_bounded_layout2,
                node,
                parent_rect,
                true,
                layout.depends_on_childs(),
                layout.depends_on_parent()});
            return;
        }

        update_bounded_layout2(e, parent_rect, node, childs);
    }

    void update_split_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        const b2f local = project_to_local(node, parent_rect);
        const auto& sl = e.get_component<split_layout>();

        //E2D_ASSERT(childs.empty() || childs.size() == 2);

        const auto calc_size = [&sl, &local](f32 size) {
            return sl.is_relative_offset() ? size * sl.offset() : sl.offset();
        };

        b2f first_r;
        b2f second_r;
        switch ( sl.origin() ) {
            case split_layout::origin_type::left: {
                f32 s = calc_size(local.size.x);
                first_r.position = v2f(0.f, 0.f);
                second_r.position = v2f(s, 0.f);
                first_r.size = second_r.size = local.size - v2f(s, 0.f);
                break;
            }
            case split_layout::origin_type::right: {
                f32 s = calc_size(local.size.x);
                first_r.position = v2f(local.size.x - s, 0.f);
                second_r.position = v2f(0.f, 0.f);
                first_r.size = second_r.size = local.size - v2f(s, 0.f);
                break;
            }
            case split_layout::origin_type::bottom: {
                f32 s = calc_size(local.size.y);
                first_r.position = v2f(0.f, 0.f);
                second_r.position = v2f(0.f, s);
                first_r.size = second_r.size = local.size - v2f(0.f, s);
                break;
            }
            case split_layout::origin_type::top: {
                f32 s = calc_size(local.size.y);
                first_r.position = v2f(0.f, local.size.y - s);
                second_r.position = v2f(0.f, 0.f);
                first_r.size = second_r.size = local.size - v2f(0.f, s);
                break;
            }
        }

        for ( size_t i = 0; i < childs.size(); ++i ) {
            if ( i ) {
                childs[i].parent_rect = second_r;
            } else {
                childs[i].parent_rect = first_r;
            }
        }

        node->translation(v3f(parent_rect.position, 0.0f));
        node->size(local.size);
    }
}

namespace
{
    b2f unproject(const m4f& mvp, const b2f& viewport) noexcept {
        const auto inv_mvp_opt = math::inversed(mvp, 0.0f);
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

    b2u project(const m4f& mvp, const v2f& size, const b2f& viewport) noexcept {
        const v3f points[] = {
            math::project(v3f(0.0f,   0.0f,   0.0f), mvp, viewport),
            math::project(v3f(0.0f,   size.y, 0.0f), mvp, viewport),
            math::project(v3f(size.x, size.y, 0.0f), mvp, viewport),
            math::project(v3f(size.x, 0.0f,   0.0f), mvp, viewport)
        };
        v2f min = v2f(points[0]);
        v2f max = min;
        for ( size_t i = 1; i < std::size(points); ++i ) {
            min.x = math::min(min.x, points[i].x);
            min.y = math::min(min.y, points[i].y);
            max.x = math::max(max.x, points[i].x);
            max.y = math::max(max.y, points[i].y);
        }
        min = math::maximized(min - 0.5f, viewport.position);
        max = math::minimized(max + 0.5f, viewport.position + viewport.size);
        return b2f(min, max - min).cast_to<u32>();

    }

    std::tuple<m4f,b2f> get_vp_and_viewport(
        const ecs::const_entity& e,
        const camera& cam) noexcept
    {
        const actor* cam_a = e.find_component<actor>();
        const m4f& cam_w = cam_a && cam_a->node()
            ? cam_a->node()->world_matrix()
            : m4f::identity();
        const auto cam_w_inv = math::inversed(cam_w, 0.0f);
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

        if ( math::approximately(std::get<1>(vp_and_viewport).size, v2f()) ) {
            return;
        }
        
        std::vector<ui_layout::layout_state> temp_layouts;
        std::vector<ui_layout::layout_state> pending;

        temp_layouts.reserve(64);
        pending.reserve(512);
        {
            const m4f mvp = root->world_matrix() * std::get<0>(vp_and_viewport);
            const b2f bbox = unproject(mvp, std::get<1>(vp_and_viewport));
            ecs::entity e = root->owner()->entity();
            const ui_layout* layout = e.find_component<ui_layout>();
            root->size(bbox.size);
            root->translation(root->translation() + v3f(bbox.position, 0.0f));

            if ( layout && !layout->enabled() ) {
                return;
            }

            pending.push_back({
                e.id(),
                layout ? layout->update_fn() : nullptr,
                root,
                bbox,
                false,
                layout ? layout->depends_on_childs() : false,
                layout ? layout->depends_on_parent() : false});
        }

        for (; !pending.empty(); ) {
            auto curr = pending.back();
            pending.pop_back();

            ecs::entity e(owner, curr.id);
            const b2f parent_rect(curr.node->size());
            const bool is_post_update = curr.is_post_update && !curr.depends_on_childs;
            
            curr.node->for_each_child([&temp_layouts, &parent_rect, is_post_update](const node_iptr& n) {
                ecs::entity e = n->owner()->entity();
                const ui_layout* layout = e.find_component<ui_layout>();

                if ( !layout || !layout->enabled() /*|| (is_post_update && !layout->depends_on_parent())*/ ) {
                    return;
                }
                temp_layouts.push_back({
                    e.id(),
                    layout->update_fn(),
                    n,
                    parent_rect,
                    is_post_update,
                    layout->depends_on_childs(),
                    layout->depends_on_parent()});
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
        owner.for_joined_components<fixed_layout::dirty, fixed_layout, actor>(
        [](ecs::entity e, fixed_layout::dirty, const fixed_layout& fl, actor& act) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_fixed_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(false);
            if ( act.node() ) {
                act.node()->size(fl.size());
            }
        });
        owner.remove_all_components<fixed_layout::dirty>();
        
        owner.for_joined_components<auto_layout::dirty, auto_layout>(
        [](ecs::entity e, auto_layout::dirty, const auto_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_auto_layout);
            layout.depends_on_childs(true);
            layout.depends_on_parent(false);
        });
        owner.remove_all_components<auto_layout::dirty>();
        
        owner.for_joined_components<stack_layout::dirty, stack_layout>(
        [](ecs::entity e, stack_layout::dirty, const stack_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_stack_layout);
            layout.depends_on_childs(true);
            layout.depends_on_parent(false);
        });
        owner.remove_all_components<stack_layout::dirty>();
        
        owner.for_joined_components<dock_layout::dirty, dock_layout>(
        [](ecs::entity e, dock_layout::dirty, const dock_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_dock_layout);
            layout.depends_on_childs(true);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<dock_layout::dirty>();
        
        owner.for_joined_components<sized_dock_layout::dirty, sized_dock_layout>(
        [](ecs::entity e, sized_dock_layout::dirty, const sized_dock_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_sized_dock_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<sized_dock_layout::dirty>();

        owner.for_joined_components<image_layout::dirty, image_layout, sprite_renderer, actor>(
        [](ecs::entity e, image_layout::dirty, image_layout& img_layout, const sprite_renderer& spr, actor& act) {
            auto& layout = e.ensure_component<ui_layout>();
            img_layout.size(spr.sprite()->content().texrect().size);
            layout.update_fn(&update_image_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
            if ( act.node() ) {
                act.node()->size(img_layout.size());
            }
        });
        owner.remove_all_components<image_layout::dirty>();
        
        owner.for_joined_components<label_layout::dirty, label_layout, label>(
        [](ecs::entity e, label_layout::dirty, const label_layout&, const label&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_label_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(false);
        });
        owner.remove_all_components<label_layout::dirty>();
        
        owner.for_joined_components<label_autoscale_layout::dirty, label_autoscale_layout, label>(
        [](ecs::entity e, label_autoscale_layout::dirty, const label_autoscale_layout&, const label&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_label_autoscale_layout);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<label_autoscale_layout::dirty>();

        owner.for_joined_components<margin_layout::dirty, margin_layout>(
        [](ecs::entity e, margin_layout::dirty, const margin_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_margin_layout);
            layout.depends_on_childs(true);
            layout.depends_on_parent(false);
        });
        owner.remove_all_components<margin_layout::dirty>();
        
        owner.for_joined_components<padding_layout::dirty, padding_layout>(
        [](ecs::entity e, padding_layout::dirty, const padding_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_padding_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<padding_layout::dirty>();

        owner.for_joined_components<anchor_layout::dirty, anchor_layout>(
        [](ecs::entity e, anchor_layout::dirty, const anchor_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_anchor_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<anchor_layout::dirty>();
        
        owner.for_joined_components<bounded_layout::dirty, bounded_layout>(
        [](ecs::entity e, bounded_layout::dirty, const bounded_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_bounded_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<bounded_layout::dirty>();
        
        owner.for_joined_components<split_layout::dirty, split_layout>(
        [](ecs::entity e, split_layout::dirty, const split_layout&) {
            auto& layout = e.ensure_component<ui_layout>();
            layout.update_fn(&update_split_layout);
            layout.depends_on_childs(false);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<split_layout::dirty>();
    }

    void update_shape_size(ecs::registry& owner) {
        owner.for_joined_components<ui_layout::shape2d_update_size_tag, rectangle_shape, actor>(
        [](const ecs::const_entity&, ui_layout::shape2d_update_size_tag, rectangle_shape& rshape, const actor& act) {
            if ( act.node() ) {
                rshape.rectangle(b2f(act.node()->size()));
            }
        });

        owner.for_joined_components<ui_layout::shape2d_update_size_tag, circle_shape, actor>(
        [](const ecs::const_entity&, ui_layout::shape2d_update_size_tag, circle_shape& cshape, const actor& act) {
            if ( act.node() ) {
                cshape.radius(math::minimum(act.node()->size()) * 0.5f);
            }
        });
    }

    void update_scissor_rect(ecs::registry& owner) {
        // get screen size
        std::tuple<m4f,b2f> vp_and_viewport;
        owner.for_joined_components<camera>(
        [&vp_and_viewport](const ecs::const_entity& e, const camera& cam) {
            if ( cam.target() ) {
                return;
            }
            vp_and_viewport = get_vp_and_viewport(e, cam);
        });

        if ( math::approximately(std::get<1>(vp_and_viewport).size, v2f()) ) {
            return;
        }

        owner.for_joined_components<ui_layout::scissor_update_rect_tag, scissor_comp, actor>(
        [&vp_and_viewport](const ecs::entity&, ui_layout::scissor_update_rect_tag, scissor_comp& sc, const actor& act) {
            if ( !act.node() ) {
                return;
            }
            const m4f mvp = act.node()->world_matrix() * std::get<0>(vp_and_viewport);
            const b2u view = project(mvp, act.node()->size(), std::get<1>(vp_and_viewport));
            sc.rect(view);
        });
    }
}

namespace e2d
{
    void ui_layout_system::process(ecs::registry& owner) {
        register_update_fn(owner);
        update_layouts(owner);
        update_shape_size(owner);
        update_scissor_rect(owner);
    }
}

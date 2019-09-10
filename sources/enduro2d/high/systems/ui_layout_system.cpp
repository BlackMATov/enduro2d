/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/ui_layout_system.hpp>
#include <enduro2d/high/components/ui_layout.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/pivot_2d.hpp>
#include <enduro2d/high/components/label.hpp>
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

#if 0
    t3f inverse_pos_scale(const t3f& v) noexcept {
        E2D_ASSERT(math::approximately(v.rotation, q4f::identity()));
        t3f result;
        result.scale = 1.0f / v.scale;
        result.translation = -v.translation * result.scale;
        return result;
    }

    v3f trnasform(const t3f& t, const v3f& v) noexcept {
        return v * t.scale + t.translation;
    }

    b2f project_to_parent(const node_iptr& n, const b2f& r) noexcept {
        const t3f tr = n->transform();
        const v3f points[] = {
            trnasform(tr, v3f(r.position.x,            r.position.y,            0.0f)),
            trnasform(tr, v3f(r.position.x,            r.position.y + r.size.y, 0.0f)),
            trnasform(tr, v3f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f)),
            trnasform(tr, v3f(r.position.x + r.size.x, r.position.y,            0.0f))
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
        const t3f tr = n->transform();
        return v2f(trnasform(tr, v3f(p.x, p.y, 0.0f)));
    }

    b2f project_to_local(const node_iptr& n, const b2f& r) noexcept {
        // TODO:
        // - rotation is not supported
        const t3f tr = inverse_pos_scale(n->transform());
        const v3f points[] = {
            trnasform(tr, v3f(r.position.x,            r.position.y,            0.0f)),
            trnasform(tr, v3f(r.position.x,            r.position.y + r.size.y, 0.0f)),
            trnasform(tr, v3f(r.position.x + r.size.x, r.position.y + r.size.y, 0.0f)),
            trnasform(tr, v3f(r.position.x + r.size.x, r.position.y,            0.0f))
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
        const t3f tr = inverse_pos_scale(n->transform());
        return v2f(trnasform(tr, v3f(p.x, p.y, 0.0f)));
    }
#else

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
        const auto inv_opt = math::inversed(n->local_matrix(), 0.0f);
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
        const auto inv_opt = math::inversed(n->local_matrix(), 0.0f);
        const m4f inv = inv_opt.second
            ? inv_opt.first
            : m4f::identity();
        return v2f(v4f(p.x, p.y, 0.0f, 1.0f) * inv);
    }
#endif
    
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
            bool first = true;
            for ( auto& c : childs ) {
                if ( c.layout->depends_on_parent() ) {
                    continue;
                }
                b2f r = project_to_parent(c.node, b2f(c.layout->size()));
                if ( !first ) {
                    join_rect(region, r);
                } else {
                    first = false;
                    region = r;
                }
            }
            
            // update layout size and node position
            node->translation(node->translation() + v3f(region.position, 0.0f));
            layout.size(region.size);

            // update child transformation
            for ( auto& c : childs ) {
                v2f off = project_to_local(c.node, v2f()) - project_to_local(c.node, region.position);
                c.node->translation(c.node->translation() + v3f(off, 0.0f) * c.node->scale());
                c.parent_rect = b2f(region.size);
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
            parent_rect,
            true});
    }
    
    void update_stack_layout2(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& sl = e.get_component<stack_layout>();
        auto& layout = e.get_component<ui_layout>();
        
        // project childs into stack layout and calculate max size
        v2f max_size;
        std::vector<v2f> projected(childs.size());
        for ( size_t i = 0; i < childs.size(); ++i ) {
            v2f p = project_to_parent(childs[i].node, b2f(childs[i].layout->size())).size;
            projected[i] = p;
            max_size += p;
        }
        
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
            post_update |= c.layout->depends_on_childs();
        }

        if ( post_update ) {
            auto& layout = e.get_component<ui_layout>();
            childs.push_back({
                e.id(),
                &update_stack_layout2,
                node,
                &layout,
                parent_rect,
                true});
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
            E2D_ASSERT_MSG(false, "undefined vertical docking");
        }
        
        node->translation(v3f(region.position, node->translation().z));
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
            post_update |= c.layout->depends_on_childs();
        }
        auto& layout = e.get_component<ui_layout>();

        if ( post_update ) {
            childs.push_back({
                e.id(),
                &update_dock_layout2,
                node,
                &layout,
                parent_rect,
                true});
            return;
        }
        update_dock_layout2(e, parent_rect, node, childs);
    }

    void update_image_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>&)
    {
        node->translation(v3f(0.0f, 0.0f, node->translation().z));
        node->scale(v3f(1.0f));

        const b2f local = project_to_local(node, parent_rect);
        auto& layout = e.get_component<ui_layout>();
        auto& il = e.get_component<image_layout>();

        if ( math::approximately(local.size, v2f()) ) {
            return;
        }

        if ( il.preserve_aspect() ) {
            f32 scale = math::min(local.size.x / il.size().x, local.size.y / il.size().y);
            node->scale(v3f(scale));
        } else {
            node->scale(v3f(local.size / il.size(), 1.0f));
        }

        //node->translation(v3f(pv ? pv->pivot() : v2f(), 0.0f) * node->scale());
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
        auto& layout = e.get_component<ui_layout>();

        v2f size = project_to_parent(child.node, b2f(child.layout->size())).size;
        v2f off = project_to_local(child.node, v2f(ml.left(), ml.bottom()));

        layout.size(size + v2f(ml.left() + ml.right(), ml.top() + ml.bottom()));
        child.node->translation(child.node->translation() + v3f(off, 0.0f));
    }

    void update_margin_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        bool post_update = false;
        for (auto& c : childs ) {
            post_update |= c.layout->depends_on_childs();
        }
        auto& layout = e.get_component<ui_layout>();

        if ( post_update ) {
            childs.push_back({
                e.id(),
                &update_margin_layout2,
                node,
                &layout,
                parent_rect,
                true});
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
        auto& layout = e.get_component<ui_layout>();
        const auto& pl = e.get_component<padding_layout>();
        const b2f local = project_to_local(node, parent_rect);
        const v2f pad_size = v2f(pl.left() + pl.right(), pl.bottom() + pl.top());

        if ( local.size.x > pad_size.x && local.size.y > pad_size.y ) {
            node->translation(v3f(pl.left(), pl.bottom(), node->translation().z));
            layout.size(local.size - pad_size);
        } else {
            node->translation(v3f(0.0f, 0.0f, node->translation().z));
            layout.size(v2f());
        }

        for ( auto& c : childs ) {
            c.parent_rect = b2f(layout.size());
        }
    }
    
    void update_label_layout(
        ecs::entity& e,
        const b2f& parent_rect,
        const node_iptr& node,
        std::vector<ui_layout::layout_state>& childs)
    {
        auto& layout = e.get_component<ui_layout>();
        const auto& ll = e.get_component<label_layout>();
        const label& lbl = e.get_component<label>();

        layout.size(lbl.bounds().size);
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
            ui_layout& layout = e.get_component<ui_layout>();
            layout.size(bbox.size);
            root->translation(root->translation() + v3f(bbox.position, 0.0f));

            pending.push_back({
                e.id(),
                layout.update_fn(),
                root,
                &layout,
                bbox,
                false});
        }

        for (; !pending.empty(); ) {
            auto curr = pending.back();
            pending.pop_back();

            ecs::entity e(owner, curr.id);
            const b2f parent_rect(curr.layout->size());
            const bool is_post_update = curr.is_post_update && !curr.layout->depends_on_childs();
            
            curr.node->for_each_child([&temp_layouts, &parent_rect, is_post_update](const node_iptr& n) {
                ecs::entity e = n->owner()->entity();
                const ui_layout* layout = e.find_component<ui_layout>();

                if ( !layout || (is_post_update && !layout->depends_on_parent()) ) {
                    return;
                }
                temp_layouts.push_back({
                    e.id(),
                    layout->update_fn(),
                    n,
                    layout,
                    parent_rect,
                    is_post_update});
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
        owner.for_joined_components<fixed_layout::dirty, fixed_layout, ui_layout>(
        [](const ecs::entity&, fixed_layout::dirty, const fixed_layout& fl, ui_layout& layout) {
            layout.size(fl.size());
        });
        owner.remove_all_components<fixed_layout::dirty>();
        
        owner.for_joined_components<auto_layout::dirty, auto_layout, ui_layout>(
        [](const ecs::entity&, auto_layout::dirty, const auto_layout&, ui_layout& layout) {
            layout.update_fn(&update_auto_layout);
            layout.depends_on_childs(true);
        });
        owner.remove_all_components<auto_layout::dirty>();
        
        owner.for_joined_components<stack_layout::dirty, stack_layout, ui_layout>(
        [](const ecs::entity&, stack_layout::dirty, const stack_layout& sl, ui_layout& layout) {
            layout.update_fn(&update_stack_layout);
            layout.depends_on_childs(true);
        });
        owner.remove_all_components<stack_layout::dirty>();
        
        owner.for_joined_components<dock_layout::dirty, dock_layout, ui_layout>(
        [](const ecs::entity&, dock_layout::dirty, const dock_layout&, ui_layout& layout) {
            layout.update_fn(&update_dock_layout);
            layout.depends_on_childs(true);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<dock_layout::dirty>();
        
        owner.for_joined_components<image_layout::dirty, image_layout, ui_layout, sprite_renderer>(
        [](const ecs::entity&, image_layout::dirty, image_layout& img_layout,
           ui_layout& layout, const sprite_renderer& spr)
        {
            img_layout.size(spr.sprite()->content().texrect().size);
            layout.update_fn(&update_image_layout);
            layout.depends_on_parent(true);
            layout.size(img_layout.size());
        });
        owner.remove_all_components<image_layout::dirty>();
        
        owner.for_joined_components<margin_layout::dirty, margin_layout, ui_layout>(
        [](const ecs::entity&, margin_layout::dirty, const margin_layout&, ui_layout& layout) {
            layout.update_fn(&update_margin_layout);
            layout.depends_on_childs(true);
        });
        owner.remove_all_components<margin_layout::dirty>();
        
        owner.for_joined_components<padding_layout::dirty, padding_layout, ui_layout>(
        [](const ecs::entity&, padding_layout::dirty, const padding_layout&, ui_layout& layout) {
            layout.update_fn(&update_padding_layout);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<padding_layout::dirty>();

        owner.for_joined_components<label_layout::dirty, label_layout, ui_layout>(
        [](const ecs::entity&, label_layout::dirty, const label_layout&, ui_layout& layout) {
            layout.update_fn(&update_label_layout);
            layout.depends_on_parent(true);
        });
        owner.remove_all_components<label_layout::dirty>();
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

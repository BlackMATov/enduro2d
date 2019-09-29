/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/render.hpp>
#include <enduro2d/core/engine.hpp>
#include <enduro2d/core/render_queue.hpp>

#include <enduro2d/high/systems/render_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/draw_order.hpp>

#include "render_system_impl.hpp"

namespace
{
    using namespace e2d;

    struct vertex_v3f_t2f_c32b {
        v3f v;
        v2f t;
        color32 c;

        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_vertex")
                .add_attribute<v2f>("a_st0")
                .add_attribute<color32>("a_color0").normalized();
        }
    };
}

namespace
{
    void draw_sprite(
        render_queue::command_encoder& cmd_encoder,
        const draw_order& draw_idx,
        const sprite_renderer& spr_r,
        const renderer& node_r,
        const actor& actor)
    {
        if ( !actor.node() || !node_r.enabled() ) {
            return;
        }
        
        if ( !spr_r.sprite() ) {
            E2D_ASSERT(false);
            return;
        }

        const sprite& spr = spr_r.sprite()->content();
        const texture_ptr& tex_p = spr.texture()
            ? spr.texture()->content()
            : texture_ptr();

        if ( !tex_p || math::is_near_zero(spr_r.tint().a) ) {
            E2D_ASSERT(false);
            return;
        }

        const b2f& tex_r = spr.texrect();
        const v2f& tex_s = tex_p->size().cast_to<f32>();
        const v2f& size = actor.node()->size();

        const v4f p1{0.0f,   0.0f,   0.f, 1.f};
        const v4f p2{size.x, 0.0f,   0.f, 1.f};
        const v4f p3{size.x, size.y, 0.f, 1.f};
        const v4f p4{0.0f,   size.y, 0.f, 1.f};

        const f32 tx = tex_r.position.x / tex_s.x;
        const f32 ty = tex_r.position.y / tex_s.y;
        const f32 tw = tex_r.size.x / tex_s.x;
        const f32 th = tex_r.size.y / tex_s.y;
        
        const m4f& sm = actor.node()->world_matrix();
        const color32& tc = spr_r.tint();

        const render::sampler_min_filter tex_min_f = spr_r.filtering()
            ? render::sampler_min_filter::linear
            : render::sampler_min_filter::nearest;

        const render::sampler_mag_filter tex_mag_f = spr_r.filtering()
            ? render::sampler_mag_filter::linear
            : render::sampler_mag_filter::nearest;

        material_asset::ptr mat_a;
        switch ( spr_r.blending() ) {
        case sprite_renderer::blendings::normal:
            mat_a = spr_r.find_material(normal_material_hash);
            break;
        case sprite_renderer::blendings::additive:
            mat_a = spr_r.find_material(additive_material_hash);
            break;
        case sprite_renderer::blendings::multiply:
            mat_a = spr_r.find_material(multiply_material_hash);
            break;
        case sprite_renderer::blendings::screen:
            mat_a = spr_r.find_material(screen_material_hash);
            break;
        default:
            E2D_ASSERT_MSG(false, "unexpected blend mode for sprite");
            break;
        }

        if ( !mat_a ) {
            E2D_ASSERT(false);
            return;
        }

        auto batch = cmd_encoder.alloc_batch<vertex_v3f_t2f_c32b>(
            draw_idx.index(),
            4, 6,
            render::topology::triangles,
            std::make_shared<const render::material>(
                render::material(mat_a->content())
                .sampler(texture_sampler_hash, render::sampler_state()
                    .texture(tex_p)
                    .min_filter(tex_min_f)
                    .mag_filter(tex_mag_f))
                ));

        batch.vertices++ = { v3f(p1 * sm), {tx + 0.f, ty + 0.f}, tc };
        batch.vertices++ = { v3f(p2 * sm), {tx + tw,  ty + 0.f}, tc };
        batch.vertices++ = { v3f(p3 * sm), {tx + tw,  ty + th }, tc };
        batch.vertices++ = { v3f(p4 * sm), {tx + 0.f, ty + th }, tc };

        batch.indices++ = 0;  batch.indices++ = 1;  batch.indices++ = 2;
        batch.indices++ = 2;  batch.indices++ = 3;  batch.indices++ = 0;
    }
}

namespace e2d
{
    void sprite_render_system::process(ecs::registry& owner, ecs::event_ref event) {
        auto& encoder = event.cast<render_system::render_with_camera_evt>().rq;

        owner.for_joined_components<draw_order, sprite_renderer, renderer, actor>([encoder](
            const ecs::const_entity&,
            const draw_order& draw_idx,
            const sprite_renderer& spr_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_sprite(*encoder, draw_idx, spr_r, node_r, actor);
        });
    }
}

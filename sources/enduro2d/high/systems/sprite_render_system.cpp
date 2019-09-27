/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/render.hpp>
#include <enduro2d/core/engine.hpp>

#include <enduro2d/high/systems/render_system.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/camera.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/draw_order.hpp>

#include <enduro2d/high/node.hpp>
/*
namespace
{
    using namespace e2d;

    struct index_u16 {
        using type = u16;
        static index_declaration decl() noexcept {
            return index_declaration::index_type::unsigned_short;
        }
    };

    struct vertex_v3f_t2f_c32b {
        v3f v;
        v2f t;
        color32 c;

        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_vertex")
                .add_attribute<v2f>("a_st")
                .add_attribute<color32>("a_tint").normalized();
        }
    };
}

namespace
{
    const str_hash sprite_texture_sampler_hash = "u_texture";

    void draw_sprite(
        render::batchr& the_batcher,
        const sprite_renderer& spr_r,
        const renderer& node_r,
        const actor& actor)
    {
        if ( !actor.node() || !node_r.enabled() ) {
            return;
        }

        if ( !spr_r.sprite() || node_r.materials().empty() ) {
            return;
        }

        const sprite& spr = spr_r.sprite()->content();
        const texture_asset::ptr& tex_a = spr.texture();
        const material_asset::ptr& mat_a = node_r.materials().front();

        if ( !tex_a || !tex_a->content() || !mat_a ) {
            return;
        }

        const b2f& tex_r = spr.texrect();
        const v2f& tex_s = tex_a->content()->size().cast_to<f32>();

        const f32 sw = tex_r.size.x;
        const f32 sh = tex_r.size.y;

        const f32 px = tex_r.position.x - spr.pivot().x;
        const f32 py = tex_r.position.y - spr.pivot().y;

        const v4f p1{px + 0.f, py + 0.f, 0.f, 1.f};
        const v4f p2{px + sw,  py + 0.f, 0.f, 1.f};
        const v4f p3{px + sw,  py + sh,  0.f, 1.f};
        const v4f p4{px + 0.f, py + sh,  0.f, 1.f};

        const f32 tx = tex_r.position.x / tex_s.x;
        const f32 ty = tex_r.position.y / tex_s.y;
        const f32 tw = tex_r.size.x / tex_s.x;
        const f32 th = tex_r.size.y / tex_s.y;

        const m4f& sm = actor.node()->world_matrix();
        const color32& tc = spr_r.tint();

        const render::sampler_min_filter min_filter = spr_r.filtering()
            ? render::sampler_min_filter::linear
            : render::sampler_min_filter::nearest;

        const render::sampler_mag_filter mag_filter = spr_r.filtering()
            ? render::sampler_mag_filter::linear
            : render::sampler_mag_filter::nearest;

        auto batch = the_batcher.alloc_batch<vertex_v3f_t2f_c32b>(4, 6,
            render::topology::triangles,
            render::material(mat_a->content())
                .sampler(sprite_texture_sampler_hash, render::sampler_state()
                    .texture(tex_a->content())
                    .min_filter(min_filter)
                    .mag_filter(mag_filter)));

        batch.vertices++ = { v3f(p1 * sm), {tx + 0.f, ty + 0.f}, tc };
        batch.vertices++ = { v3f(p2 * sm), {tx + tw,  ty + 0.f}, tc };
        batch.vertices++ = { v3f(p3 * sm), {tx + tw,  ty + th }, tc };
        batch.vertices++ = { v3f(p4 * sm), {tx + 0.f, ty + th }, tc };

        batch.indices++ = 0;  batch.indices++ = 1;  batch.indices++ = 2;
        batch.indices++ = 2;  batch.indices++ = 3;  batch.indices++ = 0;
    }
}
*/
namespace e2d
{
    void sprite_render_system::process(ecs::registry& owner, ecs::event_ref event) {
        //auto& camera = event.cast<render_system::render_with_camera_evt>();

        owner.for_joined_components<draw_order, sprite_renderer, renderer, actor>([](
            const ecs::const_entity&,
            const draw_order& order,
            const sprite_renderer& spr_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_sprite(spr_r, node_r, actor);
        });
    }
}

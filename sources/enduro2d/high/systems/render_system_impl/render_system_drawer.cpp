/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_system_drawer.hpp"

#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/sprite_9p_renderer.hpp>

namespace
{
    using namespace e2d;

    const str_hash screen_s_property_hash = "u_screen_s";
    const str_hash matrix_v_property_hash = "u_matrix_v";
    const str_hash matrix_p_property_hash = "u_matrix_p";
    const str_hash matrix_vp_property_hash = "u_matrix_vp";
    const str_hash game_time_property_hash = "u_game_time";
    const str_hash sprite_texture_sampler_hash = "u_texture";
}

namespace e2d::render_system_impl
{
    //
    // drawer::context
    //

    drawer::context::context(
        const camera& cam,
        const const_node_iptr& cam_n,
        engine& engine,
        render& render,
        window& window,
        batcher_type& batcher)
    : render_(render)
    , batcher_(batcher)
    {
        const m4f& cam_w = cam_n
            ? cam_n->world_matrix()
            : m4f::identity();
        const std::pair<m4f,bool> cam_w_inv = math::inversed(cam_w);

        const m4f& m_v = cam_w_inv.second
            ? cam_w_inv.first
            : m4f::identity();
        const m4f& m_p = cam.projection();

        batcher_.flush()
            .property(screen_s_property_hash, cam.target()
                ? cam.target()->size().cast_to<f32>()
                : window.framebuffer_size().cast_to<f32>())
            .property(matrix_v_property_hash, m_v)
            .property(matrix_p_property_hash, m_p)
            .property(matrix_vp_property_hash, m_v * m_p)
            .property(game_time_property_hash, engine.time());

        render.execute(render::command_block<3>()
            .add_command(render::target_command(cam.target()))
            .add_command(render::viewport_command(cam.viewport()))
            .add_command(render::clear_command()
                .color_value(cam.background())));
    }

    drawer::context::~context() noexcept {
        batcher_.clear(true);
    }

    void drawer::context::draw(
        const const_node_iptr& node)
    {
        if ( !node || !node->owner() ) {
            return;
        }

        E2D_ASSERT(node->owner()->entity().valid());
        ecs::const_entity node_e = node->owner()->entity();
        const renderer* node_r = node_e.find_component<renderer>();

        if ( node_r && node_r->enabled() ) {
            const m4f& world_m = node->world_matrix();
            const model_renderer* mdl_r = node_e.find_component<model_renderer>();
            if ( mdl_r ) {
                draw(world_m, *node_r, *mdl_r);
            }
            const sprite_renderer* spr_r = node_e.find_component<sprite_renderer>();
            if ( spr_r ) {
                draw(world_m, node->size(), *node_r, *spr_r);
            }
            const sprite_9p_renderer* spr9_r = node_e.find_component<sprite_9p_renderer>();
            if ( spr9_r ) {
                draw(world_m, node->size(), *node_r, *spr9_r);
            }
        }
    }

    void drawer::context::draw(
        const m4f& world_mat,
        const renderer& node_r,
        const model_renderer& mdl_r)
    {
        if ( !mdl_r.model() || !mdl_r.model()->content().mesh() ) {
            return;
        }

        const model& mdl = mdl_r.model()->content();
        const mesh& msh = mdl.mesh()->content();

        try {
            property_cache_
                .merge(batcher_.flush())
                .property("u_matrix_m", world_mat)
                .merge(node_r.properties());

            const std::size_t submesh_count = math::min(
                msh.indices_submesh_count(),
                node_r.materials().size());

            for ( std::size_t i = 0, first_index = 0; i < submesh_count; ++i ) {
                const std::size_t index_count = msh.indices(i).size();
                const material_asset::ptr& mat = node_r.materials()[i];
                if ( mat ) {
                    render_.execute(render::draw_command(
                        mat->content(),
                        mdl.geometry(),
                        property_cache_
                    ).index_range(first_index, index_count));
                }
                first_index += index_count;
            }
        } catch (...) {
            property_cache_.clear();
            throw;
        }
        property_cache_.clear();
    }

    void drawer::context::draw(
        const m4f& world_mat,
        const v2f& size,
        const renderer& node_r,
        const sprite_renderer& spr_r)
    {
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

        const v4f p1{0.0f,   0.0f,   0.f, 1.f};
        const v4f p2{size.x, 0.0f,   0.f, 1.f};
        const v4f p3{size.x, size.y, 0.f, 1.f};
        const v4f p4{0.0f,   size.y, 0.f, 1.f};

        const f32 tx = tex_r.position.x / tex_s.x;
        const f32 ty = tex_r.position.y / tex_s.y;
        const f32 tw = tex_r.size.x / tex_s.x;
        const f32 th = tex_r.size.y / tex_s.y;

        const color32& tc = spr_r.tint();

        const batcher_type::index_type indices[] = {
            0u, 1u, 2u, 2u, 3u, 0u};

        const batcher_type::vertex_type vertices[] = {
            { v3f(p1 * world_mat), {tx + 0.f, ty + 0.f}, tc },
            { v3f(p2 * world_mat), {tx + tw,  ty + 0.f}, tc },
            { v3f(p3 * world_mat), {tx + tw,  ty + th }, tc },
            { v3f(p4 * world_mat), {tx + 0.f, ty + th }, tc }};

        const render::sampler_min_filter min_filter = spr_r.filtering()
            ? render::sampler_min_filter::linear
            : render::sampler_min_filter::nearest;

        const render::sampler_mag_filter mag_filter = spr_r.filtering()
            ? render::sampler_mag_filter::linear
            : render::sampler_mag_filter::nearest;

        try {
            property_cache_
                .sampler(sprite_texture_sampler_hash, render::sampler_state()
                    .texture(tex_a->content())
                    .min_filter(min_filter)
                    .mag_filter(mag_filter))
                .merge(node_r.properties());

            batcher_.batch(
                mat_a,
                property_cache_,
                indices, std::size(indices),
                vertices, std::size(vertices));
        } catch (...) {
            property_cache_.clear();
            throw;
        }
        property_cache_.clear();
    }
    
    void drawer::context::draw(
        const m4f& world_mat,
        const v2f& size,
        const renderer& node_r,
        const sprite_9p_renderer& spr_r)
    {
        if ( !spr_r.sprite() || node_r.materials().empty() ) {
            return;
        }
        
        const sprite_9p& spr = spr_r.sprite()->content();
        const texture_asset::ptr& tex_a = spr.texture();
        const material_asset::ptr& mat_a = node_r.materials().front();

        if ( !tex_a || !tex_a->content() || !mat_a ) {
            return;
        }
        
        const b2f& tex_r = spr.texrect();
        const b2f& in_r = spr.inner_texrect();
        const v2f& tex_s = tex_a->content()->size().cast_to<f32>();
        f32 scale = math::min(1.0f, math::minimum(size / tex_r.size));
        f32 left = (in_r.position.x - tex_r.position.x) * scale;
        f32 right = (tex_r.size.x - in_r.size.x) * scale - left;
        f32 bottom = (in_r.position.y - tex_r.position.y) * scale;
        f32 top = (tex_r.size.y - in_r.size.y) * scale - bottom;

        v4f tcx = v4f(tex_r.position.x, in_r.position.x, in_r.position.x + in_r.size.x, tex_r.position.x + tex_r.size.x) / tex_s.x;
        v4f tcy = v4f(tex_r.position.y, in_r.position.y, in_r.position.y + in_r.size.y, tex_r.position.y + tex_r.size.y) / tex_s.y;
        v4f posx = v4f(0.0f, left, size.x - right, size.x);
        v4f posy = v4f(0.0f, bottom, size.y - top, size.y);
        
        const color32& tc = spr_r.tint();

        const batcher_type::index_type indices[6*9] = {
            1, 0, 4, 1, 4, 5,
            2, 1, 5, 2, 5, 6,
            3, 2, 6, 3, 6, 7,
            5, 4, 8, 5, 8, 9,
            6, 5, 9, 6, 9, 10,
            7, 6, 10, 7, 10, 11,
            9, 8, 12, 9, 12, 13,
            10, 9, 13, 10, 13, 14,
            11, 10, 14, 11, 14, 15};

        batcher_type::vertex_type vertices[16];

        vertices[ 0] = {v3f(v4f(posx[0], posy[0], 0.0f, 1.0f) * world_mat), v2f{tcx[0], tcy[0]}, tc};
        vertices[ 1] = {v3f(v4f(posx[1], posy[0], 0.0f, 1.0f) * world_mat), v2f{tcx[1], tcy[0]}, tc};
        vertices[ 2] = {v3f(v4f(posx[2], posy[0], 0.0f, 1.0f) * world_mat), v2f{tcx[2], tcy[0]}, tc};
        vertices[ 3] = {v3f(v4f(posx[3], posy[0], 0.0f, 1.0f) * world_mat), v2f{tcx[3], tcy[0]}, tc};
        
        vertices[ 4] = {v3f(v4f(posx[0], posy[1], 0.0f, 1.0f) * world_mat), v2f{tcx[0], tcy[1]}, tc};
        vertices[ 5] = {v3f(v4f(posx[1], posy[1], 0.0f, 1.0f) * world_mat), v2f{tcx[1], tcy[1]}, tc};
        vertices[ 6] = {v3f(v4f(posx[2], posy[1], 0.0f, 1.0f) * world_mat), v2f{tcx[2], tcy[1]}, tc};
        vertices[ 7] = {v3f(v4f(posx[3], posy[1], 0.0f, 1.0f) * world_mat), v2f{tcx[3], tcy[1]}, tc};
        
        vertices[ 8] = {v3f(v4f(posx[0], posy[2], 0.0f, 1.0f) * world_mat), v2f{tcx[0], tcy[2]}, tc};
        vertices[ 9] = {v3f(v4f(posx[1], posy[2], 0.0f, 1.0f) * world_mat), v2f{tcx[1], tcy[2]}, tc};
        vertices[10] = {v3f(v4f(posx[2], posy[2], 0.0f, 1.0f) * world_mat), v2f{tcx[2], tcy[2]}, tc};
        vertices[11] = {v3f(v4f(posx[3], posy[2], 0.0f, 1.0f) * world_mat), v2f{tcx[3], tcy[2]}, tc};
        
        vertices[12] = {v3f(v4f(posx[0], posy[3], 0.0f, 1.0f) * world_mat), v2f{tcx[0], tcy[3]}, tc};
        vertices[13] = {v3f(v4f(posx[1], posy[3], 0.0f, 1.0f) * world_mat), v2f{tcx[1], tcy[3]}, tc};
        vertices[14] = {v3f(v4f(posx[2], posy[3], 0.0f, 1.0f) * world_mat), v2f{tcx[2], tcy[3]}, tc};
        vertices[15] = {v3f(v4f(posx[3], posy[3], 0.0f, 1.0f) * world_mat), v2f{tcx[3], tcy[3]}, tc};

        const render::sampler_min_filter min_filter = spr_r.filtering()
            ? render::sampler_min_filter::linear
            : render::sampler_min_filter::nearest;

        const render::sampler_mag_filter mag_filter = spr_r.filtering()
            ? render::sampler_mag_filter::linear
            : render::sampler_mag_filter::nearest;

        try {
            property_cache_
                .sampler(sprite_texture_sampler_hash, render::sampler_state()
                    .texture(tex_a->content())
                    .min_filter(min_filter)
                    .mag_filter(mag_filter))
                .merge(node_r.properties());

            batcher_.batch(
                mat_a,
                property_cache_,
                indices, std::size(indices),
                vertices, std::size(vertices));
        } catch (...) {
            property_cache_.clear();
            throw;
        }
        property_cache_.clear();
    }

    void drawer::context::flush() {
        batcher_.flush();
    }

    //
    // drawer
    //

    drawer::drawer(engine& e, debug& d, render& r, window& w)
    : engine_(e)
    , render_(r)
    , window_(w)
    , batcher_(d, r) {}
}

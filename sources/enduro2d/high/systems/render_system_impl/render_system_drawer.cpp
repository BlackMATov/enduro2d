/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_system_drawer.hpp"

#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>

#include <spine/spine.h>

namespace
{
    using namespace e2d;

    const str_hash screen_s_property_hash = "u_screen_s";

    const str_hash matrix_m_property_hash = "u_matrix_m";
    const str_hash matrix_v_property_hash = "u_matrix_v";
    const str_hash matrix_p_property_hash = "u_matrix_p";
    const str_hash matrix_vp_property_hash = "u_matrix_vp";

    const str_hash time_property_hash = "u_time";
    const str_hash texture_sampler_hash = "u_texture";

    const str_hash normal_material_hash = "normal";
    const str_hash additive_material_hash = "additive";
    const str_hash multiply_material_hash = "multiply";
    const str_hash screen_material_hash = "screen";
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
        const std::pair<m4f, bool> cam_w_inv = math::inversed(cam_w);

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
            .property(time_property_hash, engine.time());

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
            if ( auto mdl_r = node_e.find_component<model_renderer>() ) {
                draw(world_m, *node_r, *mdl_r);
            }
            if ( auto spn_p = node_e.find_component<spine_player>() ) {
                draw(world_m, *node_r, *spn_p);
            }
            if ( auto spr_r = node_e.find_component<sprite_renderer>() ) {
                draw(world_m, node->size(), *node_r, *spr_r);
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
                .property(matrix_m_property_hash, world_mat)
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
        const renderer& node_r,
        const spine_player& spine_r)
    {
        static std::vector<float> temp_vertices(1000u, 0.f);
        static std::vector<batcher_type::vertex_type> batch_vertices(1000u);

        spSkeleton* skeleton = spine_r.skeleton().get();
        spSkeletonClipping* clipper = spine_r.clipper().get();

        if ( !skeleton || !clipper ) {
            return;
        }

        if ( math::is_near_zero(skeleton->color.a) ) {
            return;
        }

        material_asset::ptr normal_mat_a;
        material_asset::ptr additive_mat_a;
        material_asset::ptr multiply_mat_a;
        material_asset::ptr screen_mat_a;

        unsigned short quad_indices[6] = { 0, 1, 2, 2, 3, 0 };

        for ( int i = 0; i < skeleton->slotsCount; ++i ) {
            spSlot* slot = skeleton->drawOrder[i];

            spAttachment* attachment = slot->attachment;
            if ( !attachment || math::is_near_zero(slot->color.a) ) {
                spSkeletonClipping_clipEnd(clipper, slot);
                continue;
            }

            int vertex_count = 0;
            float* uvs = nullptr;
            unsigned short* indices = nullptr;
            int index_count = 0;
            const spAtlasPage* atlas_page = nullptr;
            const spColor* attachment_color = nullptr;

            if ( attachment->type == SP_ATTACHMENT_REGION ) {
                spRegionAttachment* region = reinterpret_cast<spRegionAttachment*>(attachment);

                attachment_color = &region->color;
                if ( math::is_near_zero(attachment_color->a) ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }

                try {
                    vertex_count = 8;
                    if ( vertex_count > math::numeric_cast<int>(temp_vertices.size()) ) {
                        temp_vertices.resize(vertex_count);
                    }
                } catch (...) {
                    property_cache_.clear();
                    spSkeletonClipping_clipEnd(clipper, slot);
                    spSkeletonClipping_clipEnd2(clipper);
                    throw;
                }

                spRegionAttachment_computeWorldVertices(
                    region,
                    slot->bone,
                    temp_vertices.data(),
                    0, 2);

                uvs = region->uvs;
                indices = quad_indices;
                index_count = 6;
                atlas_page = static_cast<spAtlasRegion*>(region->rendererObject)->page;
            } else if ( attachment->type == SP_ATTACHMENT_MESH ) {
                spMeshAttachment* mesh = reinterpret_cast<spMeshAttachment*>(attachment);

                attachment_color = &mesh->color;
                if ( math::is_near_zero(attachment_color->a) ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }

                try {
                    vertex_count = mesh->super.worldVerticesLength;
                    if ( vertex_count > math::numeric_cast<int>(temp_vertices.size()) ) {
                        temp_vertices.resize(vertex_count);
                    }
                } catch (...) {
                    property_cache_.clear();
                    spSkeletonClipping_clipEnd(clipper, slot);
                    spSkeletonClipping_clipEnd2(clipper);
                    throw;
                }

                spVertexAttachment_computeWorldVertices(
                    &mesh->super,
                    slot,
                    0,
                    mesh->super.worldVerticesLength,
                    temp_vertices.data(),
                    0, 2);

                uvs = mesh->uvs;
                indices = mesh->triangles;
                index_count = mesh->trianglesCount;
                atlas_page = static_cast<spAtlasRegion*>(mesh->rendererObject)->page;
            } else if ( attachment->type == SP_ATTACHMENT_CLIPPING ) {
                spClippingAttachment* clip = reinterpret_cast<spClippingAttachment*>(attachment);
                spSkeletonClipping_clipStart(clipper, slot, clip);
                continue;
            } else {
                continue;
            }

            color32 vert_color = color32(
                color(skeleton->color.r, skeleton->color.g, skeleton->color.b, skeleton->color.a) *
                color(slot->color.r, slot->color.g, slot->color.b, slot->color.a) *
                color(attachment_color->r, attachment_color->g, attachment_color->b, attachment_color->a));

            texture_ptr tex_p;
            const texture_asset* texture_asset_ptr = atlas_page
                ? static_cast<texture_asset*>(atlas_page->rendererObject)
                : nullptr;
            if ( texture_asset_ptr ) {
                tex_p = texture_asset_ptr->content();
            }

            render::sampler_min_filter tex_min_f = render::sampler_min_filter::linear;
            switch ( atlas_page->minFilter ) {
            case SP_ATLAS_NEAREST:
            case SP_ATLAS_MIPMAP_NEAREST_LINEAR:
            case SP_ATLAS_MIPMAP_NEAREST_NEAREST:
                tex_min_f = render::sampler_min_filter::nearest;
                break;
            default:
                tex_min_f = render::sampler_min_filter::linear;
                break;
            }

            render::sampler_mag_filter tex_mag_f = render::sampler_mag_filter::linear;
            switch ( atlas_page->magFilter ) {
            case SP_ATLAS_NEAREST:
            case SP_ATLAS_MIPMAP_NEAREST_LINEAR:
            case SP_ATLAS_MIPMAP_NEAREST_NEAREST:
                tex_mag_f = render::sampler_mag_filter::nearest;
                break;
            default:
                tex_mag_f = render::sampler_mag_filter::linear;
                break;
            }

            render::sampler_wrap tex_wrap_s = render::sampler_wrap::repeat;
            switch ( atlas_page->uWrap ) {
            case SP_ATLAS_MIRROREDREPEAT:
                tex_wrap_s = render::sampler_wrap::mirror;
                break;
            case SP_ATLAS_CLAMPTOEDGE:
                tex_wrap_s = render::sampler_wrap::clamp;
                break;
            case SP_ATLAS_REPEAT:
                tex_wrap_s = render::sampler_wrap::repeat;
                break;
            default:
                E2D_ASSERT_MSG(false, "unexpected wrap mode for slot");
                break;
            }

            render::sampler_wrap tex_wrap_t = render::sampler_wrap::repeat;
            switch ( atlas_page->vWrap ) {
            case SP_ATLAS_MIRROREDREPEAT:
                tex_wrap_t = render::sampler_wrap::mirror;
                break;
            case SP_ATLAS_CLAMPTOEDGE:
                tex_wrap_t = render::sampler_wrap::clamp;
                break;
            case SP_ATLAS_REPEAT:
                tex_wrap_t = render::sampler_wrap::repeat;
                break;
            default:
                E2D_ASSERT_MSG(false, "unexpected wrap mode for slot");
                break;
            }

            material_asset::ptr mat_a;
            switch ( slot->data->blendMode ) {
                case SP_BLEND_MODE_NORMAL:
                    mat_a = normal_mat_a
                        ? normal_mat_a
                        : (normal_mat_a = spine_r.find_material(normal_material_hash));
                    break;
                case SP_BLEND_MODE_ADDITIVE:
                    mat_a = additive_mat_a
                        ? additive_mat_a
                        : (additive_mat_a = spine_r.find_material(additive_material_hash));
                    break;
                case SP_BLEND_MODE_MULTIPLY:
                    mat_a = multiply_mat_a
                        ? multiply_mat_a
                        : (multiply_mat_a = spine_r.find_material(multiply_material_hash));
                    break;
                case SP_BLEND_MODE_SCREEN:
                    mat_a = screen_mat_a
                        ? screen_mat_a
                        : (screen_mat_a = spine_r.find_material(screen_material_hash));
                    break;
                default:
                    E2D_ASSERT_MSG(false, "unexpected blend mode for slot");
                    break;
            }

            if ( math::is_near_zero(vert_color.a) || !tex_p || !mat_a ) {
                spSkeletonClipping_clipEnd(clipper, slot);
                continue;
            }

            const float* vertices = temp_vertices.data();
            if ( spSkeletonClipping_isClipping(clipper) ) {
                spSkeletonClipping_clipTriangles(
                    clipper,
                    temp_vertices.data(), vertex_count,
                    indices, index_count,
                    uvs,
                    2);
                vertices = clipper->clippedVertices->items;
                vertex_count = clipper->clippedVertices->size;
                uvs = clipper->clippedUVs->items;
                indices = clipper->clippedTriangles->items;
                index_count = clipper->clippedTriangles->size;
            }

            try {
                const std::size_t batch_vertex_count =
                    math::numeric_cast<std::size_t>(vertex_count >> 1);
                if ( batch_vertex_count > batch_vertices.size() ) {
                    batch_vertices.resize(batch_vertex_count);
                }

                for ( std::size_t j = 0; j < batch_vertex_count; ++j ) {
                    batcher_type::vertex_type& vert = batch_vertices[j];
                    vert.v = v3f(v4f(vertices[j * 2], vertices[j * 2 + 1], 0.f, 1.f) * world_mat);
                    vert.t = v2f(uvs[j * 2], uvs[j * 2 + 1]);
                    vert.c = vert_color;
                }

                property_cache_
                    .sampler(texture_sampler_hash, render::sampler_state()
                        .texture(tex_p)
                        .filter(tex_min_f, tex_mag_f)
                        .wrap(tex_wrap_s, tex_wrap_t))
                    .merge(node_r.properties());

                batcher_.batch(
                    mat_a,
                    property_cache_,
                    indices, index_count,
                    batch_vertices.data(), batch_vertex_count);
            } catch (...) {
                property_cache_.clear();
                spSkeletonClipping_clipEnd(clipper, slot);
                spSkeletonClipping_clipEnd2(clipper);
                throw;
            }

            spSkeletonClipping_clipEnd(clipper, slot);
        }

        spSkeletonClipping_clipEnd2(clipper);
        property_cache_.clear();
    }

    void drawer::context::draw(
        const m4f& world_mat,
        const v2f& size,
        const renderer& node_r,
        const sprite_renderer& spr_r)
    {
        if ( !spr_r.sprite() ) {
            return;
        }

        const sprite& spr = spr_r.sprite()->content();
        const texture_ptr& tex_p = spr.texture()
            ? spr.texture()->content()
            : texture_ptr();

        if ( !tex_p || math::is_near_zero(spr_r.tint().a) ) {
            return;
        }

        const b2f& tex_r = spr.texrect();
        const v2f& tex_s = tex_p->size().cast_to<f32>();

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
            return;
        }

        try {
            property_cache_
                .sampler(texture_sampler_hash, render::sampler_state()
                    .texture(tex_p)
                    .filter(tex_min_f, tex_mag_f))
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

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_system_drawer.hpp"

#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>
#include <enduro2d/high/components/spine_renderer.hpp>
#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/render_technique.hpp>

#include <spine/AnimationState.h>
#include <spine/Skeleton.h>
#include <spine/VertexEffect.h>
#include <spine/SkeletonClipping.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>

namespace
{
    using namespace e2d;
    
    const str_hash matrix_m_property_hash = "u_matrix_m";
    const str_hash matrix_v_property_hash = "u_matrix_v";
    const str_hash matrix_p_property_hash = "u_matrix_p";
    const str_hash matrix_vp_property_hash = "u_matrix_vp";
    const str_hash game_time_property_hash = "u_game_time";
    const str_hash sprite_texture_sampler_hash = "u_texture";

    const render::blending_state blend_normal = render::blending_state().enable(true)
        .factor(render::blending_factor::src_alpha, render::blending_factor::one_minus_src_alpha);
    const render::blending_state blend_additive = render::blending_state().enable(true)
        .factor(render::blending_factor::src_alpha, render::blending_factor::one);
    const render::blending_state blend_multiply = render::blending_state().enable(true)
        .factor(render::blending_factor::dst_color, render::blending_factor::one_minus_src_alpha);
    const render::blending_state blend_screen = render::blending_state().enable(true)
        .factor(render::blending_factor::one, render::blending_factor::one_minus_src_color);
    
    const render::blending_state blend_normal_pma = render::blending_state().enable(true)
        .factor(render::blending_factor::one, render::blending_factor::one_minus_src_alpha);
    const render::blending_state blend_additive_pma = render::blending_state().enable(true)
        .factor(render::blending_factor::one, render::blending_factor::one);
    const render::blending_state blend_multiply_pma = render::blending_state().enable(true)
        .factor(render::blending_factor::dst_color, render::blending_factor::one_minus_src_alpha);
    const render::blending_state blend_screen_pma = render::blending_state().enable(true)
        .factor(render::blending_factor::one, render::blending_factor::one_minus_src_color);
}

namespace e2d::render_system_impl
{
    //
    // drawer::context
    //

    drawer::context::context(
        const camera& cam,
        const render_technique* rt,
        const const_node_iptr& cam_n,
        engine& engine,
        render& render)
    : render_(render)
    {
        render::renderpass_desc rp;
        const_buffer_ptr cbuf;

        if ( rt && rt->passes().size() ) {
            auto& pass = rt->passes().front();
            rp = pass.desc;

            if ( rp.color_load_op() == render::attachment_load_op::clear ) {
                rp.color_clear(cam.background());
            }
            
            if ( pass.templ ) {
                render::property_map props = pass.properties;

                const m4f& cam_w = cam_n
                    ? cam_n->world_matrix()
                    : m4f::identity();
                const std::pair<m4f,bool> cam_w_inv = math::inversed(cam_w);

                const m4f& m_v = cam_w_inv.second
                    ? cam_w_inv.first
                    : m4f::identity();
                const m4f& m_p = cam.projection();

                props.assign(matrix_v_property_hash, m_v)
                    .assign(matrix_p_property_hash, m_p)
                    .assign(matrix_vp_property_hash, m_v * m_p)
                    .assign(game_time_property_hash, engine.time());

                cbuf = render_.create_const_buffer(
                    pass.templ,
                    const_buffer::scope::render_pass);
                render_.update_buffer(cbuf, props);
            }
        } else {
            rp.color_clear(cam.background())
              .color_store()
              .depth_clear(1.0f)
              .depth_discard();
        }
        
        rp.target(cam.target())
          .viewport(cam.viewport());
        
        render_.begin_pass(rp, cbuf);
    }

    drawer::context::~context() noexcept {
        render_.end_pass();
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
            const model_renderer* mdl_r = node_e.find_component<model_renderer>();
            if ( mdl_r ) {
                draw(node, *node_r, *mdl_r);
            }
            const sprite_renderer* spr_r = node_e.find_component<sprite_renderer>();
            if ( spr_r ) {
                draw(node, *node_r, *spr_r);
            }
            const spine_renderer* spine_r = node_e.find_component<spine_renderer>();
            if ( spine_r ) {
                draw(node, *node_r, *spine_r);
            }
        }
    }

    void drawer::context::draw(
        const const_node_iptr& node,
        const renderer& node_r,
        const model_renderer& mdl_r)
    {
        if ( !node || !node_r.enabled() ) {
            return;
        }

        if ( !mdl_r.model() || !mdl_r.model()->content().mesh() ) {
            return;
        }
            
        const model& mdl = mdl_r.model()->content();
        const mesh& msh = mdl.mesh()->content();
            
        E2D_ASSERT(msh.indices_submesh_count() == node_r.materials().size());
        const std::size_t submesh_count = math::min(
            msh.indices_submesh_count(),
            node_r.materials().size());

        if ( !submesh_count ) {
            return;
        }

        render_.batcher().flush();

        // TODO: use temporary const_buffer
        if ( mdl_r.constants() ) {
            render_.update_buffer(
                mdl_r.constants(),
                render::property_map()
                    .assign(matrix_m_property_hash, node->world_matrix()));
        }

        render::bind_vertex_buffers_command vb_cmd;
        for ( std::size_t i = 0; i < mdl.vertices_count(); ++i ) {
            vb_cmd.add(mdl.vertices(i), mdl.attribute(i));
        }
        render_.execute(vb_cmd);

        for ( std::size_t i = 0, index_offset = 0; i < submesh_count; ++i ) {
            const u32 index_count = math::numeric_cast<u32>(msh.indices(i).size());
            const material_asset::ptr& mat = node_r.materials()[i];
            E2D_ASSERT(mat);
            if ( mat ) {
                render_.set_material(mat->content());
                render_.execute(render::draw_indexed_command()
                    .constants(mdl_r.constants())
                    .topo(mdl.topo())
                    .indices(mdl.indices())
                    .index_count(index_count)
                    .index_offset(index_offset));
            }
            index_offset += index_count * mdl.indices()->decl().bytes_per_index();
        }
    }

    void drawer::context::draw(
        const const_node_iptr& node,
        const renderer& node_r,
        const sprite_renderer& spr_r)
    {
        if ( !node || !node_r.enabled() ) {
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

        const m4f& sm = node->world_matrix();
        const color32& tc = spr_r.tint();

        const render::sampler_min_filter min_filter = spr_r.filtering()
            ? render::sampler_min_filter::linear
            : render::sampler_min_filter::nearest;

        const render::sampler_mag_filter mag_filter = spr_r.filtering()
            ? render::sampler_mag_filter::linear
            : render::sampler_mag_filter::nearest;

        auto batch = render_.batcher().alloc_batch<vertex_v3f_t2f_c32b>(4, 6,
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
    
    void drawer::context::draw(
        const const_node_iptr& node,
        const renderer& node_r,
        const spine_renderer& spine_r)
    {
        constexpr int stride = 2;
        std::vector<float> temp_vertices;   // TODO: optimize

        if ( !node || !node_r.enabled() || node_r.materials().empty() ) {
            return;
        }

        spSkeleton* skeleton = spine_r.skeleton().operator->();
        spSkeletonClipping* clipper = spine_r.clipper().operator->();
        spVertexEffect* effect = spine_r.effect().operator->();
        const material_asset::ptr& src_mat = node_r.materials().front();
        const bool use_premultiplied_alpha = spine_r.model()->content().premultiplied_alpha();

        if ( !skeleton || !clipper || !src_mat || math::approximately(skeleton->color.a, 0.0f) ) {
            return;
        }

        if ( effect ) {
            effect->begin(effect, skeleton);
        }
        
        u16 quad_indices[6] = { 0, 1, 2, 2, 3, 0 };
        const m4f& sm = node->world_matrix();

        for ( int i = 0; i < skeleton->slotsCount; ++i ) {
            spSlot* slot = skeleton->drawOrder[i];
            spAttachment* attachment = slot->attachment;

            if ( !attachment ) {
                continue;
            }
            if ( math::approximately(slot->color.a, 0.0f) ) {
                spSkeletonClipping_clipEnd(clipper, slot);
                continue;
            }

            int vertex_count = 0;
            float* uvs = nullptr;
            u16* indices = nullptr;
            int index_count = 0;
            const spColor* attachment_color;
            texture_ptr texture;

            if ( attachment->type == SP_ATTACHMENT_REGION ) {
                spRegionAttachment* region = reinterpret_cast<spRegionAttachment*>(attachment);
                attachment_color = &region->color;

                if ( math::approximately(attachment_color->a, 0.0f) ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }
                if ( temp_vertices.size() < 4*stride ) {
                    temp_vertices.resize(4*stride);
                }
                spRegionAttachment_computeWorldVertices(region, slot->bone, temp_vertices.data(), 0, stride);
                vertex_count = 4*stride;
                uvs = region->uvs;
                indices = quad_indices;
                index_count = u32(std::size(quad_indices));
                if ( texture_asset* asset = static_cast<texture_asset*>(static_cast<spAtlasRegion*>(region->rendererObject)->page->rendererObject) ) {
                    texture = asset->content();
                }
            } else if ( attachment->type == SP_ATTACHMENT_MESH ) {
                spMeshAttachment* mesh = reinterpret_cast<spMeshAttachment*>(attachment);
                attachment_color = &mesh->color;

                if ( math::approximately(attachment_color->a, 0.0f) ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }
                vertex_count = mesh->super.worldVerticesLength;
                if ( vertex_count > int(temp_vertices.size()) ) {
                    temp_vertices.resize(vertex_count);
                }
                spVertexAttachment_computeWorldVertices(&mesh->super, slot, 0, mesh->super.worldVerticesLength, temp_vertices.data(), 0, stride);
                uvs = mesh->uvs;
                indices = mesh->triangles;
                index_count = mesh->trianglesCount;
                if ( texture_asset* asset = static_cast<texture_asset*>(static_cast<spAtlasRegion*>(mesh->rendererObject)->page->rendererObject) ) {
                    texture = asset->content();
                }
            } else if ( attachment->type == SP_ATTACHMENT_CLIPPING ) {
                spClippingAttachment* clip = reinterpret_cast<spClippingAttachment*>(attachment);
                spSkeletonClipping_clipStart(clipper, slot, clip);
                continue;
            } else {
                continue;
            }

            const color32 vert_color(
                color(skeleton->color.r, skeleton->color.g, skeleton->color.b, skeleton->color.a) *
                color(slot->color.r, slot->color.g, slot->color.b, slot->color.a) *
                color(attachment_color->r, attachment_color->g, attachment_color->b, attachment_color->a));

            render::material mat_a = src_mat->content();
            switch ( slot->data->blendMode ) {
                case SP_BLEND_MODE_NORMAL :
                    mat_a.blending(use_premultiplied_alpha ? blend_normal_pma : blend_normal);
                    break;
                case SP_BLEND_MODE_ADDITIVE :
                    mat_a.blending(use_premultiplied_alpha ? blend_additive_pma : blend_additive);
                    break;
                case SP_BLEND_MODE_MULTIPLY :
                    mat_a.blending(use_premultiplied_alpha ? blend_multiply_pma : blend_multiply);
                    break;
                case SP_BLEND_MODE_SCREEN :
                    mat_a.blending(use_premultiplied_alpha ? blend_screen_pma : blend_screen);
                    break;
                default :
                    E2D_ASSERT_MSG(false, "unexpected blend mode for slot");
                    break;
            }
            
            const float* vertices = temp_vertices.data();
            if ( spSkeletonClipping_isClipping(clipper) ) {
                spSkeletonClipping_clipTriangles(
                    clipper,
                    temp_vertices.data(), vertex_count,
                    indices, index_count,
                    uvs,
                    stride);
                vertices = clipper->clippedVertices->items;
                vertex_count = clipper->clippedVertices->size;
                uvs = clipper->clippedUVs->items;
                indices = clipper->clippedTriangles->items;
                index_count = clipper->clippedTriangles->size;
            }

            if ( index_count ) {
                auto batch = render_.batcher().alloc_batch<vertex_v3f_t2f_c32b>(
                    vertex_count >> 1,
                    index_count,
                    render::topology::triangles,
                    mat_a.sampler(sprite_texture_sampler_hash, render::sampler_state()
                        .texture(texture)
                        .min_filter(render::sampler_min_filter::linear)
                        .mag_filter(render::sampler_mag_filter::linear)));

                for ( size_t j = 0, cnt = vertex_count >> 1; j < cnt; ++j ) {
                    auto& vert = batch.vertices[j];
                    vert.v = v3f(v4f(vertices[j*2], vertices[j*2+1], 0.0f, 1.0f) * sm);
                    vert.t = v2f(uvs[j*2], uvs[j*2+1]);
                    vert.c = vert_color;
                }
                for ( size_t j = 0; j < index_count; ++j ) {
                    batch.indices = indices[j];
                    ++batch.indices;
                }
            }
            spSkeletonClipping_clipEnd(clipper, slot);
        }

        spSkeletonClipping_clipEnd2(clipper);

        if ( effect ) {
            effect->end(effect);
        }
    }

    void drawer::context::flush() {
        render_.batcher().flush();
    }

    //
    // drawer
    //

    drawer::drawer(engine& e, render& r)
    : engine_(e)
    , render_(r) {}
}

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
#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/draw_order.hpp>

#include <enduro2d/high/assets/texture_asset.hpp>

#include "render_system_impl.hpp"

#include <spine/Skeleton.h>
#include <spine/VertexEffect.h>
#include <spine/SkeletonClipping.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>


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
    void draw_spine(
        render_queue::command_encoder& cmd_encoder,
        const draw_order& draw_idx,
        const spine_player& spine_r,
        const renderer& node_r,
        const actor& actor)
    {
        static std::vector<float> temp_vertices(1000u, 0.f);
        
        if ( !actor.node() || !node_r.enabled() ) {
            return;
        }

        spSkeleton* skeleton = spine_r.skeleton().get();
        spSkeletonClipping* clipper = spine_r.clipper().get();

        if ( !skeleton || !clipper ) {
            return;
        }

        if ( math::is_near_zero(skeleton->color.a) ) {
            return;
        }
        
        const m4f& world_m = actor.node()->world_matrix();

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
                
                if ( index_count ) {
                    auto batch = cmd_encoder.alloc_batch<vertex_v3f_t2f_c32b>(
                        draw_idx.index(),
                        batch_vertex_count,
                        index_count,
                        render::topology::triangles,
                        std::make_shared<const render::material>(render::material(mat_a->content())
                            .sampler(texture_sampler_hash, render::sampler_state()
                                .texture(tex_p)
                                .filter(tex_min_f, tex_mag_f)))
                        );

                    for ( size_t j = 0; j < batch_vertex_count; ++j ) {
                        auto& vert = batch.vertices[j];
                        vert.v = v3f(v4f(vertices[j * 2], vertices[j * 2 + 1], 0.f, 1.f) * world_m);
                        vert.t = v2f(uvs[j * 2], uvs[j * 2 + 1]);
                        vert.c = vert_color;
                    }
                    for ( size_t j = 0; j < index_count; ++j ) {
                        batch.indices = indices[j];
                        ++batch.indices;
                    }
                }
            } catch (...) {
                spSkeletonClipping_clipEnd(clipper, slot);
                spSkeletonClipping_clipEnd2(clipper);
                throw;
            }

            spSkeletonClipping_clipEnd(clipper, slot);
        }

        spSkeletonClipping_clipEnd2(clipper);
    }
}

namespace e2d
{
    void spine_render_system::process(ecs::registry& owner, ecs::event_ref event) {
        auto& encoder = event.cast<render_system::render_with_camera_evt>().rq;

        owner.for_joined_components<draw_order, spine_player, renderer, actor>([encoder](
            const ecs::const_entity&,
            const draw_order& draw_idx,
            const spine_player& spine_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_spine(*encoder, draw_idx, spine_r, node_r, actor);
        });
    }
}

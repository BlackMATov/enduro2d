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

#include <spine/Skeleton.h>
#include <spine/VertexEffect.h>
#include <spine/SkeletonClipping.h>
#include <spine/RegionAttachment.h>
#include <spine/MeshAttachment.h>

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
    void draw_spine(
        render::batchr& the_batcher,
        const spine_renderer& spine_r,
        const renderer& node_r,
        const actor& actor)
    {
        static std::vector<vertex_v3f_t2f_c32b> temp_vertices;

        static_assert(sizeof(vertex_v3f_t2f_c32b) % sizeof(float) == 0, "invalid stride");
        constexpr int stride = sizeof(vertex_v3f_t2f_c32b) / sizeof(float);

        if ( !actor.node() || !node_r.enabled() ) {
            return;
        }

        if ( node_r.materials().empty() ) {
            return;
        }

        spSkeleton* skeleton = spine_r.skeleton().operator->();
        spSkeletonClipping* clipper = spine_r.clipper().operator->();
        spVertexEffect* effect = spine_r.effect().operator->();
        const material_asset::ptr& mat_a = node_r.materials().front();

        if ( !skeleton || !clipper || !mat_a ) {
            return;
        }

        const u16 quad_indices[6] = { 0, 1, 2, 2, 3, 0 };

        if ( skeleton->color.a == 0 ) {
            return;
        }

        if ( effect ) {
            effect->begin(effect, skeleton);
        }
        
        const m3f sm = m3f(v3f(actor.node()->world_matrix()[0]),
                           v3f(actor.node()->world_matrix()[1]),
                           v3f(actor.node()->world_matrix()[3]));

        for ( int i = 0; i < skeleton->slotsCount; ++i ) {
            spSlot* slot = skeleton->drawOrder[i];
            spAttachment* attachment = slot->attachment;
            if ( !attachment ) {
                continue;
            }
            if ( slot->color.a == 0 ) {
                spSkeletonClipping_clipEnd(clipper, slot);
                continue;
            }

            size_t vertex_count = 0;
            const float* uvs = nullptr;
            const u16* indices = nullptr;
            int index_count = 0;
            const spColor* attachment_color;
            texture_ptr texture;

            if ( attachment->type == SP_ATTACHMENT_REGION ) {
                spRegionAttachment* region = reinterpret_cast<spRegionAttachment*>(attachment);
                attachment_color = &region->color;

                if ( attachment_color->a == 0 ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }
                if ( 4 > temp_vertices.size() ) {
                    temp_vertices.resize(4);
                }
                spRegionAttachment_computeWorldVertices(region, slot->bone, &temp_vertices.data()->v.x, 0, stride);
                vertex_count = 4;
                uvs = region->uvs;
                indices = quad_indices;
                index_count = 6;
                if ( texture_asset* asset = static_cast<texture_asset*>(static_cast<spAtlasRegion*>(region->rendererObject)->page->rendererObject) ) {
                    texture = asset->content();
                }
            } else if ( attachment->type == SP_ATTACHMENT_MESH ) {
                spMeshAttachment* mesh = reinterpret_cast<spMeshAttachment*>(attachment);
                attachment_color = &mesh->color;

                if ( attachment_color->a == 0 ) {
                    spSkeletonClipping_clipEnd(clipper, slot);
                    continue;
                }
                vertex_count = mesh->super.worldVerticesLength >> 1;
                if ( vertex_count > temp_vertices.size() ) {
                    temp_vertices.resize(vertex_count);
                }
                spVertexAttachment_computeWorldVertices(&mesh->super, slot, 0, mesh->super.worldVerticesLength, &temp_vertices.data()->v.x, 0, stride);
                uvs = mesh->uvs;
                indices = mesh->triangles;
                index_count = mesh->trianglesCount;
                if ( texture_asset* asset = static_cast<texture_asset*>(static_cast<spAtlasRegion*>(mesh->rendererObject)->page->rendererObject) ) {
                    texture = asset->content();
                }
            } else if ( attachment->type == SP_ATTACHMENT_CLIPPING ) {
                spClippingAttachment* clip = reinterpret_cast<spClippingAttachment*>(attachment);
                spSkeletonClipping_clipStart(clipper, slot, clip);
                E2D_ASSERT(false);
                continue;
            } else {
                continue;
            }

            const color32 vert_color(
                color(skeleton->color.r, skeleton->color.g, skeleton->color.b, skeleton->color.a) *
                color(slot->color.r, slot->color.g, slot->color.b, slot->color.a) *
                color(attachment_color->r, attachment_color->g, attachment_color->b, attachment_color->a));

            switch ( slot->data->blendMode ) {
                case SP_BLEND_MODE_NORMAL :
                    break;
                case SP_BLEND_MODE_ADDITIVE :
                    break;
                case SP_BLEND_MODE_MULTIPLY :
                    break;
                case SP_BLEND_MODE_SCREEN :
                    break;
                default :
                    break;
            }

            if ( effect ) {
                E2D_ASSERT(false);
            } else {
                for ( int j = 0; j < vertex_count; ++j ) {
                    auto& vert = temp_vertices[j];
                    v2f v(vert.v.x, vert.v.y);
                    vert.v = v3f(v.x, v.y, 1.0f) * sm;
                    vert.t = v2f(uvs[j*2], uvs[j*2+1]);
                    vert.c = vert_color;
                }
            }

            spSkeletonClipping_clipEnd(clipper, slot);
        }

        spSkeletonClipping_clipEnd2(clipper);

        if ( effect ) {
            effect->end(effect);
        }
    }
}*/

namespace e2d
{
    void spine_render_system::process(ecs::registry& owner, ecs::event_ref event) {
        //auto& camera = event.cast<render_system::render_with_camera_evt>();

        /*owner.for_joined_components<draw_order, sprite_renderer, renderer, actor>([](
            const ecs::const_entity&,
            const draw_order& order,
            const spine_player& spine_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_spine(the_batcher, spine_r, node_r, actor);
        });*/
    }
}

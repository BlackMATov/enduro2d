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
#include <enduro2d/high/components/model_renderer.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/draw_order.hpp>

#include <enduro2d/high/node.hpp>
/*
namespace
{
    using namespace e2d;
    
    const str_hash matrix_m_property_hash = "u_matrix_m";

    void draw_mesh(
        render& the_render,
        const model_renderer& mdl_r,
        const renderer& node_r,
        const actor& actor)
    {
        if ( !actor.node() || !node_r.enabled() ) {
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

        // TODO: use temporary const_buffer
        if ( mdl_r.constants() ) {
            the_render.update_buffer(
                mdl_r.constants(),
                render::property_map()
                    .property(matrix_m_property_hash, actor.node()->world_matrix()));
        }

        render::bind_vertex_buffers_command vb_cmd;
        for ( std::size_t i = 0; i < mdl.vertices_count(); ++i ) {
            vb_cmd.add(mdl.vertices(), mdl.attribute(i), mdl.vertex_offset(i));
        }
        the_render.execute(vb_cmd);

        for ( std::size_t i = 0, index_offset = 0; i < submesh_count; ++i ) {
            const u32 index_count = math::numeric_cast<u32>(msh.indices(i).size());
            const material_asset::ptr& mat = node_r.materials()[i];
            E2D_ASSERT(mat);
            if ( mat ) {
                the_render.set_material(mat->content());
                the_render.execute(render::draw_indexed_command()
                    .constants(mdl_r.constants())
                    .topo(mdl.topo())
                    .indices(mdl.indices())
                    .index_count(index_count)
                    .index_offset(index_offset));
            }
            index_offset += index_count * mdl.indices()->decl().bytes_per_index();
        }
    }
    
    void for_all_components(ecs::registry& owner) {
        render& the_render = the<render>();
        owner.for_joined_components<model_renderer, renderer, actor>([&the_render](
            const ecs::const_entity&,
            const model_renderer& mdl_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_mesh(the_render, mdl_r, node_r, actor);
        });
    }

    void for_all_cameras(ecs::registry& owner) {
        static vector<std::pair<ecs::const_entity,camera>> temp_components;
        try {
            temp_components.reserve(owner.component_count<camera>());
            owner.for_each_component<camera>([](const ecs::const_entity& e, const camera& cam){
                temp_components.emplace_back(e, cam);
            });
            std::sort(
                temp_components.begin(),
                temp_components.end(),
                [](const auto& l, const auto& r){
                    return l.second.depth() < r.second.depth();
                });
            for ( auto& p : temp_components ) {
                for_all_components(owner);
            }
        } catch (...) {
            temp_components.clear();
            throw;
        }
        temp_components.clear();
    }
}*/

namespace e2d
{
    void model_render_system::process(ecs::registry& owner, ecs::event_ref event) {
        //auto& camera = event.cast<render_system::render_with_camera_evt>();

        /*owner.for_joined_components<draw_order, sprite_renderer, renderer, actor>([](
            const ecs::const_entity&,
            const draw_order& order,
            const model_renderer& mdl_r,
            const renderer& node_r,
            const actor& actor)
        {
            draw_mesh(the_batcher, mdl_r, node_r, actor);
        });*/
    }
}

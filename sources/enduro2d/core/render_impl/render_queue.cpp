/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/render_queue.hpp>

namespace e2d
{
    //
    // render_queue
    //
    
    render_queue::render_queue(
        render& r,
        memory_manager& mem_mngr,
        resource_cache& res_cache)
    : render_(r)
    , mem_mngr_(mem_mngr)
    , res_cache_(res_cache) {}

    render_queue::command_encoder_ptr render_queue::create_pass(
        const render::renderpass_desc& desc,
        const render::sampler_block& samplers,
        const const_buffer_ptr& constants)
    {
        auto& dst = passes_.emplace_back();
        dst.commands = std::make_shared<command_encoder>(mem_mngr_, res_cache_);
        dst.desc = desc;
        dst.constants = constants;
        dst.samplers = samplers;

        return dst.commands;
    }

    void render_queue::submit() {
        for ( auto& pass : passes_ ) {
            render_.begin_pass(pass.desc, pass.constants, pass.samplers);
            pass.commands->flush_(render_);
            render_.end_pass();
        }
        passes_.clear();
    }
}

namespace e2d
{
    //
    // memory_manager
    //

    render_queue::memory_manager::memory_manager()
    {}

    /*const_buffer_ptr render_queue::memory_manager::alloc_cbuffer(
        const cbuffer_template_ptr& templ)
    {
    }*/

    u8* render_queue::memory_manager::alloc(size_t size, size_t align) {
        for ( auto& chunk : chunks_ ) {
            size_t offset = math::align_ceil(size_t(chunk.memory.data()) + chunk.offset, align) - size_t(chunk.memory.data());
            if ( size <= (chunk.memory.size() - offset) ) {
                chunk.offset = offset + size;
                return chunk.memory.data() + offset;
            }
        }
        size_t mem_size = size*2 < chunk_size_ ? chunk_size_ : size*2;
        auto& chunk = chunks_.emplace_back();
        chunk.memory.resize(mem_size);
        size_t offset = math::align_ceil(size_t(chunk.memory.data()) + chunk.offset, align) - size_t(chunk.memory.data());
        chunk.offset = offset + size;
        return chunk.memory.data() + offset;
    }

    void render_queue::memory_manager::discard() {
        for ( auto& chunk : chunks_ ) {
            chunk.offset = 0;
        }
    }
}

namespace e2d
{
    //
    // resource_cache
    //

    render_queue::resource_cache::resource_cache(render& r)
    : render_(r)
    {}
}

namespace e2d
{
    //
    // command_encoder::draw_batch_
    //
    
    bool render_queue::command_encoder::draw_batch_::operator==(const draw_batch_& r) const noexcept {
        return mtr == r.mtr
            && topo == r.topo
            && attribs == r.attribs
            && index_offset == r.index_offset
            && buffer_index == r.buffer_index
            && index_count == r.index_count
            && scissor.has_value() == r.scissor.has_value()
            && (!scissor.has_value() || scissor.value() == r.scissor.value());
    }

    //
    // command_encoder
    //

    render_queue::command_encoder::command_encoder(
        render_queue::memory_manager& mem_mngr,
        render_queue::resource_cache& res_cache)
    : mem_mngr_(mem_mngr)
    , res_cache_(res_cache) {}

    void render_queue::command_encoder::draw_mesh(
        sort_key key,
        const material_ptr& mtr,
        const render::bind_vertex_buffers_command& vertex_buffers,
        const render::draw_indexed_command& draw_cmd,
        const std::optional<b2u>& scissor)
    {
        meshes_.insert({key, draw_mesh_{mtr, vertex_buffers, draw_cmd, scissor}});
    }
    
    void render_queue::command_encoder::upload_batch_data_(render& r, gpu_buffers& buffers) {
        static constexpr size_t max_vertex_count = 1u << 15;
        static constexpr size_t vertex_buffer_size = max_vertex_count * vertex_stride_;
        static constexpr size_t index_buffer_size = max_vertex_count * 3 * index_stride_;
        static constexpr size_t max_vertices = std::numeric_limits<batch_index_t>::max();

        buffer vertices(vertex_buffer_size);
        buffer indices(index_buffer_size);
        size_t vertex_offset = 0;
        size_t index_offset = 0;

        const auto create_buffer = [&](){
            if ( vertex_offset && index_offset ) {
                buffers.emplace_back(
                    r.create_vertex_buffer(
                        buffer_view(vertices.data(), vertex_offset),
                        vertex_buffer::usage::static_draw),
                    r.create_index_buffer(
                        buffer_view(indices.data(), index_offset),
                        index_declaration::index_type::unsigned_short,
                        index_buffer::usage::static_draw));
                index_offset = 0;
                vertex_offset = 0;
            }
        };

        for ( auto&[key, batch] : batches_ ) {
            const size_t stride = batch.attribs->decl().bytes_per_vertex();
            const size_t vert_off = (vertex_offset + stride - 1) / stride;
            const size_t vdata_size = batch.vertex_data_size;
            const size_t idata_size = batch.index_data_size;
            const size_t vert_count = vdata_size / stride;

            if ( vertex_offset + vdata_size > vertex_buffer_size ||
                 index_offset + idata_size > index_buffer_size ||
                 vert_off + vert_count >= max_vertices )
            {
                create_buffer();
            }
            
            vertex_offset = math::align_ceil(vertex_offset, stride);
            std::memcpy(vertices.data() + vertex_offset, batch.vertices(), vdata_size);

            const batch_index_t idx_off = math::numeric_cast<batch_index_t>(vert_off);
            const size_t cnt = idata_size / index_stride_;

            auto* dst = reinterpret_cast<batch_index_t*>(indices.data() + index_offset);
            auto* src = batch.indices();

            for ( size_t i = 0; i < cnt; ++i ) {
                dst[i] = src[i] + idx_off;
            }
            // TODO: triangle strip support

            batch.index_offset = index_offset;
            batch.buffer_index = math::numeric_cast<u32>(buffers.size());
            batch.index_count = math::numeric_cast<u32>(cnt);

            vertex_offset += vdata_size;
            index_offset += idata_size;
        }
        create_buffer();
    }
    
    void render_queue::command_encoder::batches_to_meshes_(const gpu_buffers& buffers) {
        auto batch_iter = batches_.begin();
        auto first_batch = batch_iter;
        auto mesh_iter = meshes_.begin();
        u32 index_count = 0;
        
        const auto flush_batch = [&]() {
            if ( index_count == 0 ) {
                return;
            }
            mesh_iter = meshes_.insert({first_batch->first, draw_mesh_()});
            auto& dst = mesh_iter->second;
            auto& src = first_batch->second;

            dst.mtr = src.mtr;
            dst.draw_cmd.indices(std::get<index_buffer_ptr>(buffers[src.buffer_index]));
            dst.draw_cmd.index_offset(src.index_offset);
            dst.draw_cmd.index_count(index_count);
            dst.draw_cmd.topo(src.topo);
            dst.vertex_buffers.add(std::get<vertex_buffer_ptr>(buffers[src.buffer_index]), src.attribs);
            dst.scissor = src.scissor;

            index_count = 0;
        };

        for (;;) {
            u32 batch_idx = batch_iter != batches_.end() ? batch_iter->first : ~0u;
            u32 mesh_idx = mesh_iter != meshes_.end() ? mesh_iter->first : ~0u;

            if ( batch_iter == batches_.end() && mesh_iter == meshes_.end() ) {
                flush_batch();
                return;
            }

            if ( batch_idx < mesh_idx ) {
                if ( !(*first_batch == *batch_iter) )
                {
                    flush_batch();
                    first_batch = batch_iter;
                }

                index_count += batch_iter->second.index_count;
                ++batch_iter;
            } else {
                flush_batch();
                ++mesh_iter;
            }
        }
    }

    void render_queue::command_encoder::flush_(render& r) {
        gpu_buffers buffers;
        upload_batch_data_(r, buffers);
        batches_to_meshes_(buffers);

        for ( auto&[key, mesh] : meshes_ ) {
            r.execute(render::scissor_command(mesh.scissor));
            r.execute(render::material_command(mesh.mtr));
            r.execute(mesh.vertex_buffers);
            r.execute(mesh.draw_cmd);
        }
    }
}

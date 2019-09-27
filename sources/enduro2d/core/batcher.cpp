/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/batcher.hpp>

namespace e2d
{   
    batcher::batch_::batch_(const material &mtr)
    : mtr(mtr) {}
    
    size_t batcher::buffer_::available(size_t align) const noexcept {
        size_t off = math::align_ceil(offset, align);
        return off < content.size()
             ? math::align_ceil(content.size() - off, align)
             : 0;
    }

    batcher::batcher(debug& d, render& r)
    : debug_(d)
    , render_(r) {}

    batcher::~batcher() noexcept = default;
    
    vertex_attribs_ptr batcher::create_vertex_attribs_(vertex_declaration decl) const {
        size_t stride = math::align_ceil(decl.bytes_per_vertex(), vertex_stride_);
        decl.skip_bytes(stride - decl.bytes_per_vertex());
        return render_.create_vertex_attribs(decl);
    }

    batcher::batch_& batcher::append_batch_(
        const material& mtr,
        topology topo,
        vertex_attribs_ptr attribs,
        size_t vert_stride,
        size_t min_vb_size,
        size_t min_ib_size)
    {
        // try to reuse last batch
        if ( batches_.size() ) {
            auto& last = batches_.back();
            auto& vb = vertex_buffers_[last.vb_index];
            auto& ib = index_buffers_[last.ib_index];

            if ( last.mtr == mtr &&
                 last.attribs == attribs &&
                 last.topo == topo &&
                 vb.available(vert_stride) >= min_vb_size &&
                 ib.available(index_stride_) >= min_ib_size )
            {
                return last;
            }
        }

        // create new batch
        batch_& result = batches_.emplace_back(mtr);

        if ( vertex_buffers_.empty() ||
             vertex_buffers_.back().available(vert_stride) < min_vb_size )
        {
            auto& vb = vertex_buffers_.emplace_back();
            vb.content.resize(vertex_buffer_size_);
        }

        if ( index_buffers_.empty() ||
             index_buffers_.back().available(index_stride_) < min_ib_size )
        {
            auto& ib = index_buffers_.emplace_back();
            ib.content.resize(index_buffer_size_);
        }
        
        result.attribs = attribs;
        result.topo = topo;
        result.vb_index = u32(vertex_buffers_.size()-1);
        result.ib_index = u32(index_buffers_.size()-1);
        result.idx_offset = index_buffers_.back().offset;

        return result;
    }

    void batcher::flush() {
        if ( !dirty_ ) {
            return;
        }
        dirty_ = false;

        vertex_attribs_ptr curr_attribs;
        shader_ptr curr_shader;
        u32 curr_vb_index = ~0u;

        std::vector<vertex_buffer_ptr> vert_buffers(vertex_buffers_.size());
        for ( std::size_t i = 0; i  < vertex_buffers_.size(); ++i ) {
            vert_buffers[i] = render_.create_vertex_buffer(
                vertex_buffers_[i].content,
                vertex_buffer::usage::static_draw);
        }

        std::vector<index_buffer_ptr> index_buffers(index_buffers_.size());
        for ( std::size_t i = 0; i < index_buffers_.size(); ++i ) {
            index_buffers[i] = render_.create_index_buffer(
                index_buffers_[i].content,
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::static_draw);
        }

        for ( auto& batch : batches_ ) {

            if ( curr_vb_index != batch.vb_index ||
                 curr_attribs != batch.attribs ||
                 curr_shader != batch.mtr.shader() )
            {
                curr_vb_index = batch.vb_index;
                curr_attribs = batch.attribs;
                curr_shader = batch.mtr.shader();

                render_.execute(render::bind_vertex_buffers_command()
                    .bind(0, vert_buffers[curr_vb_index], curr_attribs));
            }
            
            render_.set_material(batch.mtr);

            render_.execute(render::draw_indexed_command()
                .index_count(batch.idx_count)
                .index_offset(batch.idx_offset)
                .indices(index_buffers[batch.ib_index])
                .topo(batch.topo));
        }

        // TODO: optimize
        vertex_buffers_.clear();
        index_buffers_.clear();
        batches_.clear();
    }
}

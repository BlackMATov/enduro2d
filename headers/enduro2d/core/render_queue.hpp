/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include <deque>
#include <typeindex>

namespace e2d
{
    //
    // render_queue
    //

    class render_queue final {
    public:
        class memory_manager;
        class command_encoder;
        class resource_cache;

        using command_encoder_ptr = std::shared_ptr<command_encoder>;
    public:
        render_queue(
            render& r,
            memory_manager& mem_mngr,
            resource_cache& res_cache);

        [[nodiscard]] command_encoder_ptr create_pass(
            const render::renderpass_desc& desc,
            const render::sampler_block& samplers,
            const const_buffer_ptr& constants);

        void submit();
    private:
        class render_pass_ final {
        public:
            render::renderpass_desc desc;
            render::sampler_block samplers;
            const_buffer_ptr constants;
            command_encoder_ptr commands;
        };
        std::deque<render_pass_> passes_;

        render& render_;
        memory_manager& mem_mngr_;
        resource_cache& res_cache_;
    };


    //
    // render_queue::command_encoder
    //

    class render_queue::command_encoder final {
    public:
        using batch_index_t = u16;
        using sort_key = u32;
        using material_ptr = render::material_cptr;
        using topology = render::topology;

        static constexpr size_t vertex_stride_ = 16;
        static constexpr size_t index_stride_ = sizeof(batch_index_t);

        template < typename T >
        class vertex_iterator final {
        public:
            vertex_iterator() = default;
            vertex_iterator(u8* data, size_t size);
            template < typename V >
            void operator=(const V& r) noexcept;
            void operator=(const T& r) noexcept;
            vertex_iterator<T>& operator++() noexcept;
            [[nodiscard]] vertex_iterator<T> operator++(int) noexcept;
            [[nodiscard]] T& operator[](size_t index) const noexcept;
            [[nodiscard]] size_t size() const noexcept;
        private:
            u8* data_ = nullptr;
            size_t size_ = 0;
            static constexpr size_t stride_ = math::align_ceil(sizeof(T), vertex_stride_);
        };
        
        class index_iterator final {
        public:
            index_iterator() = default;
            index_iterator(u8* data, size_t size);
            void operator = (const batch_index_t& r) noexcept;
            index_iterator& operator ++() noexcept;
            [[nodiscard]] index_iterator operator ++(int) noexcept;
            [[nodiscard]] size_t size() const noexcept;
            [[nodiscard]] batch_index_t* raw() const noexcept;
        private:
            batch_index_t* indices_ = nullptr;
            size_t size_ = 0;
        };

    public:
        command_encoder(
            render_queue::memory_manager& mem_mngr,
            render_queue::resource_cache& res_cache);

        template < typename BatchType >
        void add_batch(
            sort_key key,
            const material_ptr& mtr,
            const BatchType& batch,
            const std::optional<b2u>& scissor = {});
        
        template < typename VertexType >
        struct allocated_batch {
            vertex_iterator<VertexType> vertices;
            index_iterator indices;
        };
        template < typename VertexType >
        [[nodiscard]] allocated_batch<VertexType> alloc_batch(
            sort_key key,
            size_t vertex_count,
            size_t index_count,
            topology topo,
            const material_ptr& mtr,
            const std::optional<b2u>& scissor = {});

        void draw_mesh(
            sort_key key,
            const material_ptr& mtr,
            const render::bind_vertex_buffers_command& vertex_buffers,
            const render::draw_indexed_command& draw_cmd,
            const std::optional<b2u>& scissor = {});
    private:
        friend class render_queue;
        void flush_(render& r);

        using gpu_buffers = vector<std::tuple<vertex_buffer_ptr, index_buffer_ptr>>;
        void upload_batch_data_(render& r, gpu_buffers& buffers);
        void batches_to_meshes_(const gpu_buffers& buffers);
    private:
        struct draw_batch_ {
            material_ptr mtr;
            topology topo;
            vertex_attribs_ptr attribs;
            union {
                struct {
                    u8* data; // vertices, indices
                    u32 index_data_size;
                    u32 vertex_data_size;
                };
                struct {
                    size_t index_offset;
                    u32 buffer_index;
                    u32 index_count;
                };
            };
            std::optional<b2u> scissor;

            const u8* vertices() const noexcept { return data; }
            const batch_index_t* indices() const noexcept { return reinterpret_cast<const batch_index_t*>(data + vertex_data_size); }

            bool operator==(const draw_batch_& r) const noexcept;
        };

        struct draw_mesh_ {
            material_ptr mtr;
            render::bind_vertex_buffers_command vertex_buffers;
            render::draw_indexed_command draw_cmd;
            std::optional<b2u> scissor;
        };
    private:
        flat_multimap<sort_key, draw_batch_> batches_;
        flat_multimap<sort_key, draw_mesh_> meshes_;
        render_queue::memory_manager& mem_mngr_;
        render_queue::resource_cache& res_cache_;
    };

    //
    // render_queue::memory_manager
    //

    class render_queue::memory_manager final {
    public:
        memory_manager();

        [[nodiscard]] const_buffer_ptr alloc_cbuffer(
            const cbuffer_template_ptr& templ);

        [[nodiscard]] u8* alloc(size_t size, size_t align);

        void discard();
        
    private:
        static constexpr u32 chunk_size_ = 4 << 20; // Mb
        static constexpr u32 chunk_align_ = sizeof(void*);

        struct chunk_ {
            buffer memory;
            size_t offset = 0;
        };
        std::vector<chunk_> chunks_;
    };

    //
    // render_queue::resource_cache
    //

    class render_queue::resource_cache final {
    public:
        resource_cache(render& r);

        template < typename VertexType >
        [[nodiscard]] vertex_attribs_ptr create_vertex_attribs();
    private:
        render& render_;
        flat_map<std::type_index, vertex_attribs_ptr> va_cache_;
    };
}

namespace e2d
{
}

namespace e2d
{
    //
    // vertex_iterator
    //

    template < typename T >
    inline render_queue::command_encoder::vertex_iterator<T>::vertex_iterator(u8* data, size_t size)
    : data_(data)
    , size_(size) {
        E2D_ASSERT(data_ && size_);
    }
    
    template < typename T >
    inline T& render_queue::command_encoder::vertex_iterator<T>::operator [](size_t index) const noexcept {
        E2D_ASSERT(index * stride_ < size_);
        return *reinterpret_cast<T*>(data_ + (index * stride_));
    }
    
    template < typename T >
    template < typename V >
    inline void render_queue::command_encoder::vertex_iterator<T>::operator = (const V& r) noexcept {
        operator[](0) = r;
    }
    
    template < typename T >
    inline void render_queue::command_encoder::vertex_iterator<T>::operator = (const T& r) noexcept {
        operator[](0) = r;
    }

    template < typename T >
    inline render_queue::command_encoder::vertex_iterator<T>&
    render_queue::command_encoder::vertex_iterator<T>::operator ++() noexcept {
        E2D_ASSERT(size_ >= stride_);
        size_ -= stride_;
        data_ += stride_;
        return *this;
    }
    
    template < typename T >
    inline render_queue::command_encoder::vertex_iterator<T>
    render_queue::command_encoder::vertex_iterator<T>::operator ++(int) noexcept {
        auto result = *this;
        operator++();
        return result;
    }
    
    template < typename T >
    inline size_t render_queue::command_encoder::vertex_iterator<T>::size() const noexcept {
        return size_ / stride_;
    }

    //
    // index_iterator
    //

    inline render_queue::command_encoder::index_iterator::index_iterator(u8* data, size_t size)
    : indices_(reinterpret_cast<batch_index_t*>(data))
    , size_(size / sizeof(batch_index_t)) {
        E2D_ASSERT(indices_ && size_);
    }

    inline void render_queue::command_encoder::index_iterator::operator = (const batch_index_t& r) noexcept {
        E2D_ASSERT(size_ > 0);
        *indices_ = r;
    }

    inline render_queue::command_encoder::index_iterator&
    render_queue::command_encoder::index_iterator::operator ++() noexcept {
        E2D_ASSERT(size_ > 0);
        --size_;
        ++indices_;
        return *this;
    }

    inline render_queue::command_encoder::index_iterator
    render_queue::command_encoder::index_iterator::operator ++(int) noexcept {
        auto result = *this;
        operator++();
        return result;
    }
    
    inline size_t render_queue::command_encoder::index_iterator::size() const noexcept {
        return size_;
    }
   
    inline render_queue::command_encoder::batch_index_t*
    render_queue::command_encoder::index_iterator::raw() const noexcept {
        return indices_;
    }

    //
    // command_encoder
    //
    
    template < typename BatchType >
    void render_queue::command_encoder::add_batch(
        sort_key key,
        const material_ptr& mtr,
        const BatchType& src_batch,
        const std::optional<b2u>& scissor)
    {
        const size_t vert_stride = math::align_ceil(sizeof(typename BatchType::vertex_type), vertex_stride_);
        const size_t vb_size = src_batch.vertex_count() * vert_stride;
        const size_t ib_size = src_batch.index_count() + index_stride_;
        vertex_attribs_ptr attribs = res_cache_.create_vertex_attribs<typename BatchType::vertex_type>();

        draw_batch_ dst_batch;
        dst_batch.mtr = mtr;
        dst_batch.topo = src_batch.topology();
        dst_batch.data = mem_mngr_.alloc(vb_size + ib_size, math::max(vertex_stride_, index_stride_));
        dst_batch.vertex_data_size = vb_size;
        dst_batch.index_data_size = ib_size;
        dst_batch.attribs = attribs;
        dst_batch.scissor = scissor;
        
        auto vert_iter = vertex_iterator<typename BatchType::vertex_type>(dst_batch.data, vb_size);
        auto idx_iter = index_iterator(dst_batch.data + vb_size, ib_size);
        src_batch.get_vertices(vert_iter);
        src_batch.get_indices(idx_iter);

        batches_.insert({key, dst_batch});
    }
        
    template < typename VertexType >
    render_queue::command_encoder::allocated_batch<VertexType>
    render_queue::command_encoder::alloc_batch(
        sort_key key,
        size_t vertex_count,
        size_t index_count,
        topology topo,
        const material_ptr& mtr,
        const std::optional<b2u>& scissor)
    {
        E2D_ASSERT(topo != topology::triangles || (index_count >= 3 && index_count % 3 == 0));
        E2D_ASSERT(topo != topology::triangles_strip || index_count >= 3);
        
        const size_t vert_stride = math::align_ceil(sizeof(VertexType), vertex_stride_);
        const size_t vb_size = vertex_count * vert_stride;
        const size_t ib_size = index_count * index_stride_;
        vertex_attribs_ptr attribs = res_cache_.create_vertex_attribs<VertexType>();
        
        draw_batch_ dst_batch;
        dst_batch.mtr = mtr;
        dst_batch.topo = topo;
        dst_batch.data = mem_mngr_.alloc(vb_size + ib_size, math::max(vertex_stride_, index_stride_));
        dst_batch.vertex_data_size = math::numeric_cast<u32>(vb_size);
        dst_batch.index_data_size = math::numeric_cast<u32>(ib_size);
        dst_batch.attribs = attribs;
        dst_batch.scissor = scissor;
        batches_.insert({key, dst_batch});
        
        allocated_batch<VertexType> result;
        result.vertices = vertex_iterator<VertexType>(dst_batch.data, vb_size);
        result.indices = index_iterator(dst_batch.data + vb_size, ib_size);
        return result;
    }

    //
    // render_queue::resource_cache
    //
    
    template < typename VertexType >
    vertex_attribs_ptr render_queue::resource_cache::create_vertex_attribs() {
        constexpr auto vertex_stride = render_queue::command_encoder::vertex_stride_;

        std::type_index id = typeid(VertexType);
        auto iter = va_cache_.find(id);
        if ( iter != va_cache_.end() ) {
            return iter->second;
        }

        vertex_declaration decl = VertexType::decl();
        size_t stride = math::align_ceil(decl.bytes_per_vertex(), vertex_stride);
        decl.skip_bytes(stride - decl.bytes_per_vertex());
        iter = va_cache_.insert({id, render_.create_vertex_attribs(decl)}).first;
        return iter->second;
    }
}

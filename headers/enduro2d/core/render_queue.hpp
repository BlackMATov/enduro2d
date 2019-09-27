/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include <deque>

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
        [[nodiscard]] command_encoder_ptr create_pass(
            const render::renderpass_desc& desc,
            const render::sampler_block& samplers,
            const const_buffer_ptr& constants);

        void submit();
    private:
        class render_pass_ final {
        public:
        private:
            render_pass_();
            render::renderpass_desc desc_;
            render::sampler_block samplers_;
            const_buffer_ptr constants_;
            command_encoder_ptr commands_;
        };
        std::deque<render_pass_> passes_;

        render& render_;
        memory_manager& mem_mngr_;
        //resource_cache& cache_;
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
        struct chunk_ {
            buffer data;
            size_t offset;
        };
        std::vector<chunk_> chunks_;
    };

    //
    // render_queue::command_encoder
    //

    class render_queue::command_encoder final {
    public:
        static constexpr size_t vertex_stride_ = 16;
        
        using batch_index_t = u16;
        using sort_key = u32;
        using material_ptr = render::material_cptr;
        using topology = render::topology;

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
        command_encoder();

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
            const material_ptr& mtr);
    private:
        class draw_batch_ {
            material_ptr mtr;
            topology topo;
            const_buffer_ptr constants;
            vertex_attribs_ptr attribs;
            u8* data; // vertices, indices
            u32 vertex_data_size;
            u32 index_data_size;
        };

        class draw_mesh_ {
            material_ptr mtr;
            const_buffer_ptr constants;
            render::bind_vertex_buffers_command vertex_buffers;
            index_buffer_ptr index_buffer_;
            size_t index_offset; // in bytes
            u32 index_count;
            render::topology topo;
        };
    private:
        flat_multimap<sort_key, draw_batch_> batches_;
        flat_multimap<sort_key, draw_mesh_> meshes_;
        //render_queue::memory_manager& mem_mngr_;
        //render_queue::resource_cache& cache_;
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
        const BatchType& batch,
        const std::optional<b2u>& scissor)
    {
        // TODO
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
        // TODO
    }
}

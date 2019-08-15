/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include <typeindex>

namespace e2d
{
    class render::batchr final {
    private:
        static constexpr size_t vertex_stride_ = 16;
    public:
        using batch_index_t = u16;

        template < typename T >
        class vertex_iterator final {
        public:
            vertex_iterator() = default;
            vertex_iterator(u8* data, size_t size);
            void operator = (const T& r) noexcept;
            vertex_iterator<T>& operator ++() noexcept;
            [[nodiscard]] vertex_iterator<T> operator ++(int) noexcept;
            [[nodiscard]] T& operator [](size_t index) const noexcept;
            [[nodiscard]] size_t size() const noexcept;
        private:
            u8* data_ = nullptr;
            size_t size_ = 0;
            static constexpr size_t stride_ = 
                ((sizeof(T) + vertex_stride_-1) / vertex_stride_) * vertex_stride_;
        };

        class index_iterator final {
            friend class batchr;
        public:
            index_iterator() = default;
            index_iterator(u8* data, size_t size, batch_index_t offset);
            void operator = (const batch_index_t& r) noexcept;
            index_iterator& operator ++() noexcept;
            [[nodiscard]] index_iterator operator ++(int) noexcept;
            [[nodiscard]] size_t size() const noexcept;
        private:
            batch_index_t* indices_ = nullptr;
            size_t size_ = 0;
            batch_index_t offset_ = 0;
        };

        template < typename VertexType >
        class rectangle_batch {
        public:
            using vertex_type = VertexType;
        public:
            rectangle_batch() = default;
            rectangle_batch(const b2f& pos, const b2f& uv, color32 col);
            void get_indices(index_iterator iter) const noexcept;
            void get_vertices(vertex_iterator<VertexType> iter) const noexcept;
            static topology topology() noexcept;
            static u32 index_count() noexcept;
            static u32 vertex_count() noexcept;
        public:
            b2f pos;
            b2f uv;
            color32 col;
        };

        template < typename VertexType >
        class nine_patch_batch {
        public:
            using vertex_type = VertexType;
        public:
            void get_indices(index_iterator iter) const noexcept;
            void get_vertices(vertex_iterator<VertexType> iter) const noexcept;
            static topology topology() noexcept;
            static u32 index_count() noexcept;
            static u32 vertex_count() noexcept;
        public:
            color32 col;
        };
        
        template < typename VertexType >
        class circle_batch {
        public:
            using vertex_type = VertexType;
        public:
            circle_batch() = default;
            circle_batch(const b2f& pos, const b2f& uv, color32 col, u32 segments);
            void get_indices(index_iterator iter) const noexcept;
            void get_vertices(vertex_iterator<VertexType> iter) const noexcept;
            static topology topology() noexcept;
            static u32 index_count() noexcept;
            static u32 vertex_count() noexcept;
        public:
            b2f pos;
            b2f uv;
            u32 segment_count = 8;
            color32 col;
        };

    private:
        class batch_ final {
        public:
            batch_(const material &mtr);
        public:
            material mtr;
            vertex_attribs_ptr attribs;
            topology topo = topology::triangles;
            size_t idx_offset = 0; // in bytes
            u32 idx_count = 0;
            u8 vb_index = 0xFF;
            u8 ib_index = 0xFF;
            // TODO: add scissor
        };

        class buffer_ final {
        public:
            size_t available(size_t align) const noexcept;
        public:
            buffer content;
            size_t offset;
        };

        static constexpr size_t index_stride_ = sizeof(batch_index_t);
        static constexpr size_t max_vertex_count_ = 1u << 15;
        static constexpr size_t vertex_buffer_size_ = max_vertex_count_ * vertex_stride_;
        static constexpr size_t index_buffer_size_ = max_vertex_count_ * 3 * index_stride_;

    public:
        batchr(debug& d, render& r);
        ~batchr() noexcept;

        template < typename BatchType >
        void add_batch(const material& mtr, const BatchType& batch);
        
        template < typename VertexType >
        struct allocated_batch {
            vertex_iterator<VertexType> vertices;
            index_iterator indices;
        };
        template < typename VertexType >
        [[nodiscard]] allocated_batch<VertexType> alloc_batch(
            size_t vertex_count,
            size_t index_count,
            topology topo,
            const material& mtr);

        void flush();

    private:
        template < typename BatchType >
        void break_strip_(
            const BatchType& src,
            vertex_iterator<typename BatchType::vertex_type> vert_iter,
            index_iterator idx_iter) const noexcept;
        
        template < typename BatchType >
        void continue_list_(
            const BatchType& src,
            vertex_iterator<typename BatchType::vertex_type> vert_iter,
            index_iterator idx_iter) const noexcept;

        batch_& append_batch_(
            const material& mtr,
            topology topo,
            vertex_attribs_ptr attribs,
            size_t vert_stride,
            size_t min_vb_size,
            size_t min_ib_size);

        template < typename VertexType >
        vertex_attribs_ptr create_vertex_attribs_();

    private:
        debug& debug_;
        render& render_;
        std::vector<batch_> batches_;
        std::vector<buffer_> vertex_buffers_;
        std::vector<buffer_> index_buffers_;
        flat_map<std::type_index, vertex_attribs_ptr> va_cache_;
        bool dirty_ = false;
    };
}

namespace e2d
{
    //
    // batchr::vertex_iterator
    //

    template < typename T >
    inline render::batchr::vertex_iterator<T>::vertex_iterator(u8* data, size_t size)
    : data_(data)
    , size_(size) {
        E2D_ASSERT(data_ && size_);
    }
    
    template < typename T >
    inline T& render::batchr::vertex_iterator<T>::operator [](size_t index) const noexcept {
        E2D_ASSERT(index * stride_ < size_);
        return *reinterpret_cast<T*>(data_ + (index * stride_));
    }
    
    template < typename T >
    inline void render::batchr::vertex_iterator<T>::operator = (const T& r) noexcept {
        operator[](0) = r;
    }

    template < typename T >
    inline render::batchr::vertex_iterator<T>& render::batchr::vertex_iterator<T>::operator ++() noexcept {
        E2D_ASSERT(size_ >= stride_);
        size_ -= stride_;
        data_ += stride_;
        return *this;
    }
    
    template < typename T >
    inline render::batchr::vertex_iterator<T> render::batchr::vertex_iterator<T>::operator ++(int) noexcept {
        auto result = *this;
        operator++();
        return result;
    }
    
    template < typename T >
    inline size_t render::batchr::vertex_iterator<T>::size() const noexcept {
        return size_ / stride_;
    }

    //
    // batchr::index_iterator
    //

    inline render::batchr::index_iterator::index_iterator(u8* data, size_t size, batch_index_t offset)
    : indices_(reinterpret_cast<batch_index_t*>(data))
    , size_(size / sizeof(batch_index_t))
    , offset_(offset) {
        E2D_ASSERT(indices_ && size_);
    }

    inline void render::batchr::index_iterator::operator = (const batch_index_t& r) noexcept {
        E2D_ASSERT(size_ > 0);
        *indices_ = r + offset_;
    }

    inline render::batchr::index_iterator& render::batchr::index_iterator::operator ++() noexcept {
        E2D_ASSERT(size_ > 0);
        --size_;
        ++indices_;
        return *this;
    }

    inline render::batchr::index_iterator render::batchr::index_iterator::operator ++(int) noexcept {
        auto result = *this;
        operator++();
        return result;
    }
    
    inline size_t render::batchr::index_iterator::size() const noexcept {
        return size_;
    }

    //
    // batchr::rectangle_batch
    //
    
    template < typename VertexType >
    render::batchr::rectangle_batch<VertexType>::rectangle_batch(
        const b2f& pos,
        const b2f& uv,
        color32 col)
    : pos(pos)
    , uv(uv)
    , col(col) {}

    template < typename VertexType >
    void render::batchr::rectangle_batch<VertexType>::get_indices(
        index_iterator iter) const noexcept
    {
        iter   = 0;  ++iter = 1;  ++iter = 2;
        ++iter = 1;  ++iter = 2;  ++iter = 3;
    }
    
    template < typename VertexType >
    void render::batchr::rectangle_batch<VertexType>::get_vertices(
        vertex_iterator<VertexType> iter) const noexcept
    {
        iter   = VertexType(pos.position,                         uv.position,                        col);
        ++iter = VertexType(pos.position + v2f(0.0f, pos.size.y), uv.position + v2f(0.0f, uv.size.y), col);
        ++iter = VertexType(pos.position + v2f(pos.size.x, 0.0f), uv.position + v2f(uv.size.x, 0.0f), col);
        ++iter = VertexType(pos.position + pos.size,              uv.position + uv.size,              col);
    }
    
    template < typename VertexType >
    render::topology render::batchr::rectangle_batch<VertexType>::topology() noexcept {
        return topology::triangles;
    }
    
    template < typename VertexType >
    u32 render::batchr::rectangle_batch<VertexType>::index_count() noexcept {
        return 6;
    }
    
    template < typename VertexType >
    u32 render::batchr::rectangle_batch<VertexType>::vertex_count() noexcept {
        return 4;
    }

    //
    // batchr
    //

    template < typename BatchType >
    void render::batchr::add_batch(const material& mtr, const BatchType& src_batch) {
        const bool is_strip = src_batch.topology() != topology::triangles;
        const size_t vert_stride = math::align_ceil(sizeof(typename BatchType::vertex_type), vertex_stride_);
        const size_t vb_size = src_batch.vertex_count() * vert_stride;
        const size_t ib_size = (src_batch.index_count() + (is_strip ? 2 : 0)) * index_stride_;
        vertex_attribs_ptr attribs = create_vertex_attribs_<typename BatchType::vertex_type>();
        batch_& dst_batch = append_batch_(mtr, src_batch.topology(), attribs, vert_stride, vb_size, ib_size);

        auto& vb = vertex_buffers_[dst_batch.vb_index];
        auto& ib = index_buffers_[dst_batch.ib_index];
        vb.offset = math::align_ceil(vb.offset, vert_stride);
        
        batch_index_t idx_offset = math::numeric_cast<batch_index_t>((vb.offset + vert_stride-1) / vert_stride);
        auto vert_iter = vertex_iterator<typename BatchType::vertex_type>(vb.content.data() + vb.offset, vb_size, vert_stride);
        auto idx_iter = index_iterator(ib.content.data() + ib.offset, ib_size, idx_offset);
        const bool first_strip = is_strip && !dst_batch.idx_count;
        const bool break_strip = is_strip && dst_batch.idx_count;

        if ( break_strip ) {
            break_strip_(src_batch, vert_iter, idx_iter);
        } else {
            continue_list_(src_batch, vert_iter, idx_iter);
        }
        
        vb.offset += vb_size;
        ib.offset += ib_size - (first_strip ? 2*index_stride_ : 0);
        dst_batch.idx_count += src_batch.index_count() + (break_strip ? 2 : 0);
        dirty_ = true;
    }
    
    template < typename VertexType >
    render::batchr::allocated_batch<VertexType> render::batchr::alloc_batch(
        size_t vertex_count,
        size_t index_count,
        topology topo,
        const material& mtr)
    {
        const size_t vert_stride = math::align_ceil(sizeof(VertexType), vertex_stride_);
        const size_t vb_size = vertex_count * vert_stride;
        const size_t ib_size = index_count * index_stride_;
        vertex_attribs_ptr attribs = create_vertex_attribs_<VertexType>();
        batch_& dst_batch = append_batch_(mtr, topo, attribs, vert_stride, vb_size, ib_size);
        
        auto& vb = vertex_buffers_[dst_batch.vb_index];
        auto& ib = index_buffers_[dst_batch.ib_index];
        vb.offset = math::align_ceil(vb.offset, vert_stride);

        allocated_batch<VertexType> result;
        batch_index_t idx_offset = math::numeric_cast<batch_index_t>((vb.offset + vert_stride-1) / vert_stride);
        result.vertices = vertex_iterator<VertexType>(vb.content.data() + vb.offset, vb_size);
        result.indices = index_iterator(ib.content.data() + ib.offset, ib_size, idx_offset);
        
        vb.offset += vb_size;
        ib.offset += ib_size;
        dst_batch.idx_count += u32(index_count);
        dirty_ = true;

        return result;
    }

    template < typename BatchType >
    void render::batchr::break_strip_(
        const BatchType& src,
        vertex_iterator<typename BatchType::vertex_type> vert_iter,
        index_iterator idx_iter) const noexcept
    {
        batch_index_t* indices = idx_iter.indices_;
        ++idx_iter;
        ++idx_iter;

        src.get_vertices(vert_iter);
        src.get_indices(idx_iter);

        indices[0] = indices[-1];
        indices[1] = indices[2];
    }
        
    template < typename BatchType >
    void render::batchr::continue_list_(
        const BatchType& src,
        vertex_iterator<typename BatchType::vertex_type> vert_iter,
        index_iterator idx_iter) const noexcept
    {
        src.get_vertices(vert_iter);
        src.get_indices(idx_iter);
    }

    template < typename VertexType >
    vertex_attribs_ptr render::batchr::create_vertex_attribs_() {
        std::type_index id = typeid(VertexType);
        auto iter = va_cache_.find(id);
        if ( iter != va_cache_.end() ) {
            return iter->second;
        }

        vertex_declaration decl = VertexType::decl();
        size_t stride = math::align_ceil(decl.bytes_per_vertex(), vertex_stride_);
        decl.skip_bytes(stride - decl.bytes_per_vertex());
        iter = va_cache_.insert({id, render_.create_vertex_attribs(decl)}).first;
        return iter->second;
    }
}

namespace e2d
{
    //
    // vertex_declaration
    //

    template < typename T >
    vertex_declaration& vertex_declaration::add_attribute(str_hash name) noexcept {
        E2D_UNUSED(name);
        static_assert(sizeof(T) == 0, "not implemented for this type");
        return *this;
    }

    #define DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(t, rows, columns, type)\
        template <>\
        inline vertex_declaration& vertex_declaration::add_attribute<t>(str_hash name) noexcept {\
            return add_attribute(name, (rows), (columns), attribute_type::type, false);\
        }

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(i8, 1, 1, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<i8>, 1, 2, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<i8>, 1, 3, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<i8>, 1, 4, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<i8>, 2, 2, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<i8>, 3, 3, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<i8>, 4, 4, signed_byte)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(u8, 1, 1, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<u8>, 1, 2, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<u8>, 1, 3, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<u8>, 1, 4, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<u8>, 2, 2, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<u8>, 3, 3, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<u8>, 4, 4, unsigned_byte)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(i16, 1, 1, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<i16>, 1, 2, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<i16>, 1, 3, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<i16>, 1, 4, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<i16>, 2, 2, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<i16>, 3, 3, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<i16>, 4, 4, signed_short)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(u16, 1, 1, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<u16>, 1, 2, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<u16>, 1, 3, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<u16>, 1, 4, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<u16>, 2, 2, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<u16>, 3, 3, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<u16>, 4, 4, unsigned_short)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(f32, 1, 1, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<f32>, 1, 2, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<f32>, 1, 3, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<f32>, 1, 4, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<f32>, 2, 2, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<f32>, 3, 3, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<f32>, 4, 4, floating_point)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(color, 1, 4, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(color32, 1, 4, unsigned_byte)

    #undef DEFINE_ADD_ATTRIBUTE_SPECIALIZATION

    //
    // render::property_map
    //

    inline
    render::property_map::property_value*
    render::property_map::find(str_hash key) noexcept {
        const auto iter = values_.find(key);
        return iter != values_.end()
            ? &iter->second
            : nullptr;
    }

    inline
    const render::property_map::property_value*
    render::property_map::find(str_hash key) const noexcept {
        const auto iter = values_.find(key);
        return iter != values_.end()
            ? &iter->second
            : nullptr;
    }
    
    template< typename T >
    inline const T* render::property_map::find(str_hash key) const noexcept {
        const property_value* prop = find(key);
        return prop
            ? stdex::get_if<T>(prop)
            : nullptr;
    }

    inline render::property_map& render::property_map::assign(str_hash key, property_value&& value) {
        values_[key] = std::move(value);
        return *this;
    }

    inline render::property_map& render::property_map::assign(str_hash key, const property_value& value) {
        values_[key] = value;
        return *this;
    }

    inline void render::property_map::clear() noexcept {
        values_.clear();
    }

    inline std::size_t render::property_map::size() const noexcept {
        return values_.size();
    }

    template < typename F >
    inline void render::property_map::foreach(F&& f) const {
        for ( const auto& [name, value] : values_ ) {
            f(name, value);
        }
    }

    inline render::property_map& render::property_map::merge(const property_map& other) {
        if ( this != &other ) {
            other.foreach([this](str_hash name, const property_value& value){
                assign(name, value);
            });
        }
        return *this;
    }

    inline bool render::property_map::equals(const property_map& other) const noexcept {
        return this == &other
            ? true
            : values_ == other.values_;
    }

    //
    // render::command_block
    //

    template < std::size_t N >
    render::command_block<N>& render::command_block<N>::add_command(command_value&& value) {
        E2D_ASSERT(command_count_ < commands_.size());
        commands_[command_count_] = std::move(value);
        ++command_count_;
        return *this;
    }

    template < std::size_t N >
    render::command_block<N>& render::command_block<N>::add_command(const command_value& value) {
        E2D_ASSERT(command_count_ < commands_.size());
        commands_[command_count_] = value;
        ++command_count_;
        return *this;
    }

    template < std::size_t N >
    const render::command_value& render::command_block<N>::command(std::size_t index) const noexcept {
        E2D_ASSERT(index < command_count_);
        return commands_[index];
    }

    template < std::size_t N >
    std::size_t render::command_block<N>::command_count() const noexcept {
        return command_count_;
    }

    //
    // render::change_state_command_
    //
    
    template < typename T >
    render::change_state_command_<T>::change_state_command_(const T& state)
    : state_(state) {}

    template < typename T >
    const std::optional<T>& render::change_state_command_<T>::state() const noexcept {
        return state_;
    }

    //
    // render
    //

    template < std::size_t N >
    render& render::execute(const command_block<N>& commands) {
        E2D_ASSERT(is_in_main_thread());
        for ( std::size_t i = 0, e = commands.command_count(); i < e; ++i ) {
            execute(commands.command(i));
        }
        return *this;
    }
}

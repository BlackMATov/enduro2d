/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render.hpp"

#if defined(E2D_RENDER_MODE) && E2D_RENDER_MODE == E2D_RENDER_MODE_NONE

namespace e2d
{
    //
    // shader::internal_state
    //

    class shader::internal_state final : private e2d::noncopyable {
    public:
        internal_state() noexcept = default;
        ~internal_state() noexcept = default;
    };

    //
    // texture::internal_state
    //

    class texture::internal_state final : private e2d::noncopyable {
    public:
        internal_state() noexcept = default;
        ~internal_state() noexcept = default;
    };

    //
    // index_buffer::internal_state
    //

    class index_buffer::internal_state final : private e2d::noncopyable {
    public:
        internal_state() noexcept = default;
        ~internal_state() noexcept = default;
    };

    //
    // vertex_buffer::internal_state
    //

    class vertex_buffer::internal_state final : private e2d::noncopyable {
    public:
        internal_state() noexcept = default;
        ~internal_state() noexcept = default;
    };

    //
    // render_target::internal_state
    //

    class render_target::internal_state final : private e2d::noncopyable {
    public:
        internal_state() noexcept = default;
        ~internal_state() noexcept = default;
    };

    //
    // render::internal_state
    //

    class render::internal_state final : private e2d::noncopyable {
    public:
        debug& debug_;
        window& window_;
    public:
        internal_state(debug& debug, window& window) noexcept
        : debug_(debug)
        , window_(window) {}
        ~internal_state() noexcept = default;
    };

    //
    // shader
    //

    const shader::internal_state& shader::state() const noexcept {
        return *state_;
    }

    shader::shader(internal_state_uptr state)
    : state_(std::move(state)) {}
    shader::~shader() noexcept = default;

    //
    // texture
    //

    const texture::internal_state& texture::state() const noexcept {
        return *state_;
    }

    texture::texture(internal_state_uptr state)
    : state_(std::move(state)) {}
    texture::~texture() noexcept = default;

    const v2u& texture::size() const noexcept {
        static v2u size;
        return size;
    }

    const pixel_declaration& texture::decl() const noexcept {
        static pixel_declaration decl;
        return decl;
    }

    //
    // index_buffer
    //

    const index_buffer::internal_state& index_buffer::state() const noexcept {
        return *state_;
    }

    index_buffer::index_buffer(internal_state_uptr state)
    : state_(std::move(state)) {}
    index_buffer::~index_buffer() noexcept = default;

    std::size_t index_buffer::buffer_size() const noexcept {
        return 0u;
    }

    //
    // vertex_buffer
    //

    const vertex_buffer::internal_state& vertex_buffer::state() const noexcept {
        return *state_;
    }

    vertex_buffer::vertex_buffer(internal_state_uptr state)
    : state_(std::move(state)) {}
    vertex_buffer::~vertex_buffer() noexcept = default;

    std::size_t vertex_buffer::buffer_size() const noexcept {
        return 0u;
    }

    //
    // vertex_attribs
    //

    const vertex_attribs::internal_state& vertex_attribs::state() const noexcept {
        return *state_;
    }

    vertex_attribs::vertex_attribs(internal_state_uptr state)
    : state_(std::move(state)) {}
    vertex_attribs::~vertex_attribs() noexcept = default;

    const vertex_declaration& vertex_attribs::decl() const noexcept {
        static vertex_declaration decl;
        return decl;
    }

    //
    // const_buffer
    //

    const const_buffer::internal_state& const_buffer::state() const noexcept {
        return *state_;
    }

    const_buffer::const_buffer(internal_state_uptr state)
    : state_(std::move(state)) {}
    const_buffer::~const_buffer() noexcept = default;

    std::size_t const_buffer::buffer_size() const noexcept {
        return 0u;
    }

    //
    // render_target
    //

    render_target::render_target(internal_state_uptr state)
    : state_(std::move(state)) {}
    render_target::~render_target() noexcept = default;

    const v2u& render_target::size() const noexcept {
        static v2u size;
        return size;
    }

    const texture_ptr& render_target::color() const noexcept {
        static texture_ptr color;
        return color;
    }

    const texture_ptr& render_target::depth() const noexcept {
        static texture_ptr depth;
        return depth;
    }

    //
    // render
    //

    render::render(debug& d, window& w)
    : state_(new internal_state(d, w)) {}
    render::~render() noexcept = default;

    shader_ptr render::create_shader(
        const shader_source& source)
    {
        E2D_UNUSED(source);
        return nullptr;
    }

    texture_ptr render::create_texture(const image& image) {
        E2D_UNUSED(image);
        return nullptr;
    }

    texture_ptr render::create_texture(const input_stream_uptr& image_stream) {
        E2D_UNUSED(image_stream);
        return nullptr;
    }

    texture_ptr render::create_texture(const v2u& size, const pixel_declaration& decl) {
        E2D_UNUSED(size, decl);
        return nullptr;
    }

    index_buffer_ptr render::create_index_buffer(
        buffer_view indices,
        const index_declaration& decl,
        index_buffer::usage usage)
    {
        E2D_UNUSED(indices, decl, usage);
        return nullptr;
    }
    
    index_buffer_ptr create_index_buffer(
        size_t size,
        const index_declaration& decl,
        index_buffer::usage usage)
    {
        E2D_UNUSED(size, decl, usage);
        return nullptr;
    }

    vertex_buffer_ptr render::create_vertex_buffer(
        buffer_view vertices,
        vertex_buffer::usage usage)
    {
        E2D_UNUSED(vertices, usage);
        return nullptr;
    }
    
    vertex_buffer_ptr create_vertex_buffer(
        size_t size,
        vertex_buffer::usage usage)
    {
        E2D_UNUSED(size, usage);
        return nullptr;
    }
        
    vertex_attribs_ptr create_vertex_attribs(
        const vertex_declaration& decl)
    {
        E2D_UNUSED(decl);
        return nullptr;
    }

    const_buffer_ptr create_const_buffer(
        const shader_ptr& shader,
        const_buffer::scope scope)
    {
        E2D_UNUSED(shader, scope);
        return nullptr;
    }

    render_target_ptr render::create_render_target(
        const v2u& size,
        const pixel_declaration& color_decl,
        const pixel_declaration& depth_decl,
        render_target::external_texture external_texture)
    {
        E2D_UNUSED(size, color_decl, depth_decl, external_texture);
        return nullptr;
    }
    
    render& render::begin_pass(
        const renderpass_desc& desc,
        const const_buffer_ptr& constants,
        const sampler_block& samplers)
    {
        E2D_UNUSED(desc, constants, samplers);
        return *this;
    }

    render& render::end_pass() {
        return *this;
    }
        
    render& render::present() {
        return *this;
    }
    
    render& render::execute(const bind_vertex_buffers_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const bind_pipeline_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const bind_const_buffer_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const bind_textures_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const scissor_command& command) {
        E2D_UNUSED(command);
        return *this;
    }
    
    render& render::execute(const blending_state_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const culling_state_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const stencil_state_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const depth_state_command& command) {
        E2D_UNUSED(command);
        return *this;
    }
    
    render& render::execute(const blend_constant_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const draw_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::execute(const draw_indexed_command& command) {
        E2D_UNUSED(command);
        return *this;
    }

    render& render::update_buffer(
        const index_buffer_ptr& ibuffer,
        buffer_view indices,
        std::size_t offset)
    {
        E2D_UNUSED(ibuffer, indices, offset);
        return *this;
    }

    render& render::update_buffer(
        const vertex_buffer_ptr& vbuffer,
        buffer_view vertices,
        std::size_t offset)
    {
        E2D_UNUSED(vbuffer, vertices, offset);
        return *this;
    }
    
    render& render::update_buffer(
        const const_buffer_ptr& cbuffer,
        const property_map<property_value>& properties)
    {
        E2D_UNUSED(cbuffer, properties);
        return *this;
    }

    render& render::update_texture(
        const texture_ptr& tex,
        const image& img,
        v2u offset)
    {
        E2D_UNUSED(tex, img, offset);
        return *this;
    }

    render& render::update_texture(
        const texture_ptr& tex,
        buffer_view pixels,
        const b2u& region)
    {
        E2D_UNUSED(tex, pixels, region);
        return *this;
    }

    const render::device_caps& render::device_capabilities() const noexcept {
        static device_caps caps;
        return caps;
    }
    
    const render::statistics& render::frame_statistic() const noexcept {
        static statistics stats;
        return stats;
    }

    bool render::is_pixel_supported(const pixel_declaration& decl) const noexcept {
        E2D_UNUSED(decl);
        return false;
    }

    bool render::is_index_supported(const index_declaration& decl) const noexcept {
        E2D_UNUSED(decl);
        return false;
    }

    bool render::is_vertex_supported(const vertex_declaration& decl) const noexcept {
        E2D_UNUSED(decl);
        return false;
    }
    
    render::batchr& render::batcher() noexcept {
        static batchr b(the<debug>(), *this);
        return b;
    }
}

#endif

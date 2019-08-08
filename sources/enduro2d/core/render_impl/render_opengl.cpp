/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render.hpp"
#include "render_opengl_base.hpp"
#include "render_opengl_impl.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES

namespace
{
    using namespace e2d;

    const char* vertex_shader_header_cstr(render::api_profile profile) noexcept {
        switch ( profile ) {
        case e2d::render::api_profile::unknown:
            return "";
        case e2d::render::api_profile::opengles2:
        case e2d::render::api_profile::opengles3:
            return R"glsl(
                precision highp int;
                precision highp float;
            )glsl";
        case e2d::render::api_profile::opengl2_compat:
            return R"glsl(
                #version 120
                #define highp
                #define mediump
                #define lowp
            )glsl";
        case e2d::render::api_profile::opengl4_compat:
            return R"glsl(
                #version 410 core
                #define texture2D texture
                #define varying out
                #define attribute in
            )glsl";
        default:
            E2D_ASSERT_MSG(false, "unexpected render API profile");
            return "";
        }
    }

    const char* fragment_shader_header_cstr(render::api_profile profile) noexcept {
        switch ( profile ) {
        case e2d::render::api_profile::unknown:
            return "";
        case e2d::render::api_profile::opengles2:
        case e2d::render::api_profile::opengles3:
            return R"glsl(
                precision mediump int;
                precision mediump float;
            )glsl";
        case e2d::render::api_profile::opengl2_compat:
            return R"glsl(
                #version 120
                #define highp
                #define mediump
                #define lowp
            )glsl";
        case e2d::render::api_profile::opengl4_compat:
            return R"glsl(
                #version 410 core
                #define texture2D texture
                #define varying in
            )glsl";
        default:
            E2D_ASSERT_MSG(false, "unexpected render API profile");
            return "";
        }
    }
}

namespace
{
    using namespace e2d;
    using namespace e2d::opengl;

}

namespace e2d
{
    //
    // shader
    //

    const shader::internal_state& shader::state() const noexcept {
        return *state_;
    }

    shader::shader(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    shader::~shader() noexcept = default;

    //
    // texture
    //

    const texture::internal_state& texture::state() const noexcept {
        return *state_;
    }

    texture::texture(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    texture::~texture() noexcept = default;

    const v2u& texture::size() const noexcept {
        return state_->size();
    }

    const pixel_declaration& texture::decl() const noexcept {
        return state_->decl();
    }

    //
    // index_buffer
    //

    const index_buffer::internal_state& index_buffer::state() const noexcept {
        return *state_;
    }

    index_buffer::index_buffer(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    index_buffer::~index_buffer() noexcept = default;

    std::size_t index_buffer::buffer_size() const noexcept {
        return state_->size();
    }

    std::size_t index_buffer::index_count() const noexcept {
        E2D_ASSERT(state_->size() % state_->decl().bytes_per_index() == 0);
        return state_->size() / state_->decl().bytes_per_index();
    }

    const index_declaration& index_buffer::decl() const noexcept {
        return state_->decl();
    }

    //
    // vertex_buffer
    //

    const vertex_buffer::internal_state& vertex_buffer::state() const noexcept {
        return *state_;
    }

    vertex_buffer::vertex_buffer(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    vertex_buffer::~vertex_buffer() noexcept = default;

    std::size_t vertex_buffer::buffer_size() const noexcept {
        return state_->size();
    }
    
    //
    // vertex_attribs
    //
    
    const vertex_attribs::internal_state& vertex_attribs::state() const noexcept {
        return *state_;
    }

    vertex_attribs::vertex_attribs(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    vertex_attribs::~vertex_attribs() noexcept = default;
    
    const vertex_declaration& vertex_attribs::decl() const noexcept {
        return state_->decl();
    }

    //
    // const_buffer
    //
    
    const const_buffer::internal_state& const_buffer::state() const noexcept {
        return *state_;
    }

    const_buffer::const_buffer(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    const_buffer::~const_buffer() noexcept = default;
    
    std::size_t const_buffer::buffer_size() const noexcept {
        return state_->size();
    }

    const_buffer::scope const_buffer::binding_scope() const noexcept {
        return state_->binding_scope();
    }

    bool const_buffer::is_compatible_with(const shader_ptr& shader) const noexcept {
        return state_->is_compatible_with(shader);
    }

    //
    // render_target
    //

    const render_target::internal_state& render_target::state() const noexcept {
        return *state_;
    }

    render_target::render_target(internal_state_uptr state)
    : state_(std::move(state)) {
        E2D_ASSERT(state_);
    }
    render_target::~render_target() noexcept = default;

    const v2u& render_target::size() const noexcept {
        return state_->size();
    }

    const texture_ptr& render_target::color() const noexcept {
        return state_->color();
    }

    const texture_ptr& render_target::depth() const noexcept {
        return state_->depth();
    }

    //
    // render
    //

    render::render(debug& ndebug, window& nwindow)
    : state_(new internal_state(ndebug, nwindow)) {
        E2D_ASSERT(main_thread() == nwindow.main_thread());
    }
    render::~render() noexcept = default;

    shader_ptr render::create_shader(
        str_view vertex_source,
        str_view fragment_source)
    {
        E2D_ASSERT(is_in_main_thread());

        gl_shader_id vs = gl_compile_shader(
            state_->dbg(),
            vertex_shader_header_cstr(device_capabilities().profile),
            vertex_source,
            GL_VERTEX_SHADER);

        if ( vs.empty() ) {
            return nullptr;
        }

        gl_shader_id fs = gl_compile_shader(
            state_->dbg(),
            fragment_shader_header_cstr(device_capabilities().profile),
            fragment_source,
            GL_FRAGMENT_SHADER);

        if ( fs.empty() ) {
            return nullptr;
        }

        gl_program_id ps = gl_link_program(
            state_->dbg(),
            std::move(vs),
            std::move(fs));

        if ( ps.empty() ) {
            return nullptr;
        }

        return std::make_shared<shader>(
            std::make_unique<shader::internal_state>(
                state_->dbg(), std::move(ps)));
    }

    shader_ptr render::create_shader(
        const input_stream_uptr& vertex,
        const input_stream_uptr& fragment)
    {
        E2D_ASSERT(is_in_main_thread());

        str vertex_source, fragment_source;
        return streams::try_read_tail(vertex_source, vertex)
            && streams::try_read_tail(fragment_source, fragment)
            ? create_shader(vertex_source, fragment_source)
            : nullptr;
    }

    texture_ptr render::create_texture(
        const image& image)
    {
        E2D_ASSERT(is_in_main_thread());

        const pixel_declaration decl =
            convert_image_data_format_to_pixel_declaration(image.format());

        if ( !is_pixel_supported(decl) ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: unsupported pixel declaration\n"
                "--> Pixel type: %0",
                pixel_declaration::pixel_type_to_cstr(decl.type()));
            return nullptr;
        }

        if ( decl.is_depth() && !device_capabilities().depth_texture_supported ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: depth textures is unsupported\n"
                "--> Pixel type: %0",
                pixel_declaration::pixel_type_to_cstr(decl.type()));
            return nullptr;
        }

        if ( math::maximum(image.size()) > device_capabilities().max_texture_size ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: unsupported texture size: %0\n"
                "--> Max size: %1",
                image.size(), device_capabilities().max_texture_size);
            return nullptr;
        }

        if ( !device_capabilities().npot_texture_supported ) {
            if ( !math::is_power_of_2(image.size().x) || !math::is_power_of_2(image.size().y) ) {
                state_->dbg().error("RENDER: Failed to create texture:\n"
                    "--> Info: non power of two texture is unsupported\n"
                    "--> Size: %0",
                    image.size());
            }
        }

        gl_texture_id id = gl_texture_id::create(state_->dbg(), GL_TEXTURE_2D);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: failed to create texture id");
            return nullptr;
        }

        with_gl_bind_texture(state_->dbg(), id, [this, &id, &image, &decl]() noexcept {
            if ( decl.is_compressed() ) {
                GL_CHECK_CODE(state_->dbg(), glCompressedTexImage2D(
                    id.target(),
                    0,
                    convert_pixel_type_to_internal_format_e(decl.type()),
                    math::numeric_cast<GLsizei>(image.size().x),
                    math::numeric_cast<GLsizei>(image.size().y),
                    0,
                    math::numeric_cast<GLsizei>(image.data().size()),
                    image.data().data()));
            } else {
                GL_CHECK_CODE(state_->dbg(), glTexImage2D(
                    id.target(),
                    0,
                    convert_pixel_type_to_internal_format(decl.type()),
                    math::numeric_cast<GLsizei>(image.size().x),
                    math::numeric_cast<GLsizei>(image.size().y),
                    0,
                    convert_image_data_format_to_external_format(image.format()),
                    convert_image_data_format_to_external_data_type(image.format()),
                    image.data().data()));
            }
        });

        return std::make_shared<texture>(
            std::make_unique<texture::internal_state>(
                state_->dbg(), std::move(id), image.size(), decl));
    }

    texture_ptr render::create_texture(
        const input_stream_uptr& image_stream)
    {
        E2D_ASSERT(is_in_main_thread());

        image image;
        if ( !images::try_load_image(image, image_stream) ) {
            return nullptr;
        }
        return create_texture(image);
    }

    texture_ptr render::create_texture(
        const v2u& size,
        const pixel_declaration& decl)
    {
        E2D_ASSERT(is_in_main_thread());

        if ( !is_pixel_supported(decl) ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: unsupported pixel declaration\n"
                "--> Pixel type: %0",
                pixel_declaration::pixel_type_to_cstr(decl.type()));
            return nullptr;
        }

        if ( decl.is_depth() && !device_capabilities().depth_texture_supported ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: depth textures is unsupported\n"
                "--> Pixel type: %0",
                pixel_declaration::pixel_type_to_cstr(decl.type()));
            return nullptr;
        }

        if ( math::maximum(size) > device_capabilities().max_texture_size ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: unsupported texture size: %0\n"
                "--> Max size: %1",
                size, device_capabilities().max_texture_size);
            return nullptr;
        }

        if ( !device_capabilities().npot_texture_supported ) {
            if ( !math::is_power_of_2(size.x) || !math::is_power_of_2(size.y) ) {
                state_->dbg().error("RENDER: Failed to create texture:\n"
                    "--> Info: non power of two texture is unsupported\n"
                    "--> Size: %0",
                    size);
            }
        }

        gl_texture_id id = gl_texture_id::create(state_->dbg(), GL_TEXTURE_2D);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create texture:\n"
                "--> Info: failed to create texture id");
            return nullptr;
        }

        with_gl_bind_texture(state_->dbg(), id, [this, &id, &size, &decl]() noexcept {
            if ( decl.is_compressed() ) {
                buffer empty_data(decl.bits_per_pixel() * size.x * size.y / 8);
                GL_CHECK_CODE(state_->dbg(), glCompressedTexImage2D(
                    id.target(),
                    0,
                    convert_pixel_type_to_internal_format_e(decl.type()),
                    math::numeric_cast<GLsizei>(size.x),
                    math::numeric_cast<GLsizei>(size.y),
                    0,
                    math::numeric_cast<GLsizei>(empty_data.size()),
                    empty_data.data()));
            } else {
                GL_CHECK_CODE(state_->dbg(), glTexImage2D(
                    id.target(),
                    0,
                    convert_pixel_type_to_internal_format(decl.type()),
                    math::numeric_cast<GLsizei>(size.x),
                    math::numeric_cast<GLsizei>(size.y),
                    0,
                    convert_pixel_type_to_external_format(decl.type()),
                    convert_pixel_type_to_external_data_type(decl.type()),
                    nullptr));
            }
        #if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL
            GL_CHECK_CODE(state_->dbg(), glTexParameteri(
                id.target(),
                GL_TEXTURE_MAX_LEVEL,
                0));
            GL_CHECK_CODE(state_->dbg(), glTexParameteri(
                id.target(),
                GL_TEXTURE_BASE_LEVEL,
                0));
        #endif
        });

        return std::make_shared<texture>(
            std::make_unique<texture::internal_state>(
                state_->dbg(), std::move(id), size, decl));
    }

    index_buffer_ptr render::create_index_buffer(
        buffer_view indices,
        const index_declaration& decl,
        index_buffer::usage usage)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(indices.size() % decl.bytes_per_index() == 0);

        if ( !is_index_supported(decl) ) {
            state_->dbg().error("RENDER: Failed to create index buffer:\n"
                "--> Info: unsupported index declaration\n"
                "--> Index type: %0",
                index_declaration::index_type_to_cstr(decl.type()));
            return nullptr;
        }

        gl_buffer_id id = gl_buffer_id::create(state_->dbg(), GL_ELEMENT_ARRAY_BUFFER);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create index buffer:\n"
                "--> Info: failed to create index buffer id");
            return nullptr;
        }

        with_gl_bind_buffer(state_->dbg(), id, [this, &id, &indices, &usage]() {
            GL_CHECK_CODE(state_->dbg(), glBufferData(
                id.target(),
                math::numeric_cast<GLsizeiptr>(indices.size()),
                indices.data(),
                convert_buffer_usage(usage)));
        });

        return std::make_shared<index_buffer>(
            std::make_unique<index_buffer::internal_state>(
                state_->dbg(), std::move(id), indices.size(), decl));
    }

    index_buffer_ptr render::create_index_buffer(
        size_t size,
        const index_declaration& decl,
        index_buffer::usage usage)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(size % decl.bytes_per_index() == 0);
        E2D_ASSERT(usage != index_buffer::usage::static_draw);

        if ( !is_index_supported(decl) ) {
            state_->dbg().error("RENDER: Failed to create index buffer:\n"
                "--> Info: unsupported index declaration\n"
                "--> Index type: %0",
                index_declaration::index_type_to_cstr(decl.type()));
            return nullptr;
        }

        gl_buffer_id id = gl_buffer_id::create(state_->dbg(), GL_ELEMENT_ARRAY_BUFFER);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create index buffer:\n"
                "--> Info: failed to create index buffer id");
            return nullptr;
        }

        with_gl_bind_buffer(state_->dbg(), id, [this, &id, size, &usage]() {
            GL_CHECK_CODE(state_->dbg(), glBufferData(
                id.target(),
                math::numeric_cast<GLsizeiptr>(size),
                nullptr,
                convert_buffer_usage(usage)));
        });

        return std::make_shared<index_buffer>(
            std::make_unique<index_buffer::internal_state>(
                state_->dbg(), std::move(id), size, decl));
    }

    vertex_buffer_ptr render::create_vertex_buffer(
        buffer_view vertices,
        vertex_buffer::usage usage)
    {
        E2D_ASSERT(is_in_main_thread());

        gl_buffer_id id = gl_buffer_id::create(state_->dbg(), GL_ARRAY_BUFFER);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create vertex buffer:\n"
                "--> Info: failed to create vertex buffer id");
            return nullptr;
        }

        with_gl_bind_buffer(state_->dbg(), id, [this, &id, &vertices, &usage]() {
            GL_CHECK_CODE(state_->dbg(), glBufferData(
                id.target(),
                math::numeric_cast<GLsizeiptr>(vertices.size()),
                vertices.data(),
                convert_buffer_usage(usage)));
        });

        return std::make_shared<vertex_buffer>(
            std::make_unique<vertex_buffer::internal_state>(
                state_->dbg(), std::move(id), vertices.size()));
    }
    
    vertex_buffer_ptr render::create_vertex_buffer(
        size_t size,
        vertex_buffer::usage usage)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(usage != vertex_buffer::usage::static_draw);

        gl_buffer_id id = gl_buffer_id::create(state_->dbg(), GL_ARRAY_BUFFER);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create vertex buffer:\n"
                "--> Info: failed to create vertex buffer id");
            return nullptr;
        }

        with_gl_bind_buffer(state_->dbg(), id, [this, &id, size, &usage]() {
            GL_CHECK_CODE(state_->dbg(), glBufferData(
                id.target(),
                math::numeric_cast<GLsizeiptr>(size),
                nullptr,
                convert_buffer_usage(usage)));
        });

        return std::make_shared<vertex_buffer>(
            std::make_unique<vertex_buffer::internal_state>(
                state_->dbg(), std::move(id), size));
    }
    
    vertex_attribs_ptr render::create_vertex_attribs(
        const vertex_declaration& decl)
    {
        E2D_ASSERT(is_in_main_thread());

        if ( !is_vertex_supported(decl) ) {
            state_->dbg().error("RENDER: Failed to create vertex buffer:\n"
                "--> Info: unsupported vertex declaration");
            return nullptr;
        }

        return state_->create_vertex_attribs(decl);
    }
        
    const_buffer_ptr render::create_const_buffer(
        const shader_ptr& shader,
        const_buffer::scope scope)
    {
        E2D_ASSERT(is_in_main_thread());
        
        // TODO
        return nullptr;
    }

    render_target_ptr render::create_render_target(
        const v2u& size,
        const pixel_declaration& color_decl,
        const pixel_declaration& depth_decl,
        render_target::external_texture external_texture)
    {
        E2D_ASSERT(is_in_main_thread());

        E2D_ASSERT(
            depth_decl.is_depth() &&
            color_decl.is_color() &&
            !color_decl.is_compressed());

        if ( !device_capabilities().render_target_supported ) {
            state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                "--> Info: render target is unsupported");
            return nullptr;
        }

        if ( math::maximum(size) > device_capabilities().max_renderbuffer_size ) {
            state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                "--> Info: unsupported render target size: %0\n"
                "--> Max size: %1",
                size, device_capabilities().max_renderbuffer_size);
            return nullptr;
        }

        if ( !device_capabilities().npot_texture_supported ) {
            if ( !math::is_power_of_2(size.x) || !math::is_power_of_2(size.y) ) {
                state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                    "--> Info: non power of two render target is unsupported\n"
                    "--> Size: %0",
                    size);
            }
        }

        gl_framebuffer_id id = gl_framebuffer_id::create(state_->dbg(), GL_FRAMEBUFFER);
        if ( id.empty() ) {
            state_->dbg().error("RENDER: Failed to create framebuffer:\n",
                "--> Info: failed to create framebuffer id");
            return nullptr;
        }

        bool need_color =
            !!(utils::enum_to_underlying(external_texture)
            & utils::enum_to_underlying(render_target::external_texture::color));

        bool need_depth =
            !!(utils::enum_to_underlying(external_texture)
            & utils::enum_to_underlying(render_target::external_texture::depth));

        texture_ptr color;
        texture_ptr depth;

        gl_renderbuffer_id color_rb(state_->dbg());
        gl_renderbuffer_id depth_rb(state_->dbg());

        if ( need_color ) {
            color = create_texture(size, color_decl);
            if ( !color ) {
                state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                    "--> Info: failed to create color texture");
                return nullptr;
            }
            gl_attach_texture(state_->dbg(), id, color->state().id(), GL_COLOR_ATTACHMENT0);
        } else {
            color_rb = gl_compile_renderbuffer(
                state_->dbg(),
                size,
                convert_pixel_type_to_internal_format_e(color_decl.type()));
            if ( color_rb.empty() ) {
                state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                    "--> Info: failed to create color renderbuffer");
                return nullptr;
            }
            gl_attach_renderbuffer(state_->dbg(), id, color_rb, GL_COLOR_ATTACHMENT0);
        }

        if ( need_depth ) {
            depth = create_texture(size, depth_decl);
            if ( !depth ) {
                state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                    "--> Info: failed to create depth texture");
                return nullptr;
            }
            gl_attach_texture(state_->dbg(), id, depth->state().id(), GL_DEPTH_ATTACHMENT);
            if ( depth_decl.is_stencil() ) {
                gl_attach_texture(state_->dbg(), id, depth->state().id(), GL_STENCIL_ATTACHMENT);
            }
        } else {
            depth_rb = gl_compile_renderbuffer(
                state_->dbg(),
                size,
                convert_pixel_type_to_internal_format_e(depth_decl.type()));
            if ( depth_rb.empty() ) {
                state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                    "--> Info: failed to create depth renderbuffer");
                return nullptr;
            }
            gl_attach_renderbuffer(state_->dbg(), id, depth_rb, GL_DEPTH_ATTACHMENT);
            if ( depth_decl.is_stencil() ) {
                gl_attach_renderbuffer(state_->dbg(), id, depth_rb, GL_STENCIL_ATTACHMENT);
            }
        }

        GLenum fb_status = GL_FRAMEBUFFER_COMPLETE;
        if ( !gl_check_framebuffer(state_->dbg(), id, &fb_status) ) {
            state_->dbg().error("RENDER: Failed to create framebuffer:\n"
                "--> Info: framebuffer is incomplete\n"
                "--> Status: %0",
                gl_framebuffer_status_to_cstr(fb_status));
            return nullptr;
        }

        return std::make_shared<render_target>(
            std::make_unique<render_target::internal_state>(
                state_->dbg(),
                std::move(id),
                size,
                std::move(color),
                std::move(depth),
                std::move(color_rb),
                std::move(depth_rb)));
    }
    
    render& render::begin_pass(
        const renderpass_desc& desc,
        const const_buffer_ptr& cbuffer,
        const sampler_block& samplers)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(!cbuffer || cbuffer->binding_scope() == const_buffer::scope::render_pass);

        state_->begin_render_pass(desc);
        state_->bind_const_buffer(cbuffer);
        state_->bind_textures(sampler_block::scope::render_pass, samplers);
        return *this;
    }

    render& render::end_pass() {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(state_->inside_render_pass());

        state_->end_render_pass();
        return *this;
    }
        
    render& render::present() {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(!state_->inside_render_pass());
        // TODO: check unpresented changes

        state_->on_present();
        state_->wnd().swap_buffers();
        return *this;
    }
        
    render& render::execute(const bind_vertex_buffers_command& command) {
        E2D_ASSERT(is_in_main_thread());
        for ( size_t i = 0; i < command.binding_count(); ++i ) {
            state_->bind_vertex_buffer(
                i,
                command.vertices(i),
                command.attributes(i),
                command.vertex_offset(i));
        }
        return *this;
    }

    render& render::execute(const bind_pipeline_command& command) {
        E2D_ASSERT(is_in_main_thread());
        state_->set_shader_program(command.shader());
        return *this;
    }

    render& render::execute(const bind_const_buffer_command& command) {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(command.buffer());
        E2D_ASSERT(command.buffer()->binding_scope() != const_buffer::scope::render_pass);

        state_->bind_const_buffer(command.buffer());
        return *this;
    }

    render& render::execute(const bind_textures_command& command) {
        E2D_ASSERT(is_in_main_thread());
        state_->bind_textures(sampler_block::scope::material, command.samplers());
        return *this;
    }

    render& render::execute(const scissor_command& command) {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(state_->inside_render_pass());
        // TODO
        return *this;
    }

    render& render::execute(const draw_command& command) {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(command.vertex_count() > 0);
        E2D_ASSERT(state_->inside_render_pass());

        state_->bind_const_buffer(command.cbuffer());
        state_->draw(
            command.topo(),
            command.first_vertex(),
            command.vertex_count());

        return *this;
    }

    render& render::execute(const draw_indexed_command& command) {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(command.index_count() > 0);
        E2D_ASSERT(state_->inside_render_pass());
        
        state_->bind_index_buffer(command.indices());
        state_->bind_const_buffer(command.cbuffer());
        state_->draw_indexed(
            command.topo(),
            command.index_count(),
            command.index_offset());
        return *this;
    }

    render& render::update_buffer(
        const index_buffer_ptr& ibuffer,
        buffer_view indices,
        std::size_t offset)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(ibuffer);
        const std::size_t buffer_offset = offset * ibuffer->state().decl().bytes_per_index();
        E2D_ASSERT(indices.size() + buffer_offset <= ibuffer->state().size());
        E2D_ASSERT(indices.size() % ibuffer->state().decl().bytes_per_index() == 0);
        opengl::with_gl_bind_buffer(ibuffer->state().dbg(), ibuffer->state().id(),
            [&ibuffer, &indices, &buffer_offset]() noexcept {
                GL_CHECK_CODE(ibuffer->state().dbg(), glBufferSubData(
                    ibuffer->state().id().target(),
                    math::numeric_cast<GLintptr>(buffer_offset),
                    math::numeric_cast<GLsizeiptr>(indices.size()),
                    indices.data()));
            });
        return *this;
    }

    render& render::update_buffer(
        const vertex_buffer_ptr& vbuffer,
        buffer_view vertices,
        std::size_t offset)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(vbuffer);
        E2D_ASSERT(vertices.size() + offset <= vbuffer->state().size());
        opengl::with_gl_bind_buffer(vbuffer->state().dbg(), vbuffer->state().id(),
            [&vbuffer, &vertices, offset]() noexcept {
                GL_CHECK_CODE(vbuffer->state().dbg(), glBufferSubData(
                    vbuffer->state().id().target(),
                    math::numeric_cast<GLintptr>(offset),
                    math::numeric_cast<GLsizeiptr>(vertices.size()),
                    vertices.data()));
            });
        return *this;
    }
    
    render& render::update_buffer(
        const const_buffer_ptr& cbuffer,
        const shader_ptr& shader,
        const property_map& properties)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(cbuffer);
        E2D_ASSERT(shader);
        if ( state_->device_capabilities_ext().uniform_buffer_supported ) {
            // TODO
            E2D_ASSERT(false);
        } else {

            properties.foreach([&shader, &cbuffer](str_hash name, const auto& value) noexcept{
                using T = std::remove_const_t<std::remove_reference_t<decltype(value)>>;

                shader->state().with_uniform_location(name, [&cbuffer, &value](auto& info) noexcept{
                    /*
                    float* dst = cbuffer->state().data();
                    size_t buffer_size = cbuffer->state().size();
                    const size_t off = info.offset / sizeof(float);
                    
                    E2D_ASSERT(info.scope == cbuffer->binding_scope());
                    E2D_ASSERT(off + sizeof(value) <= buffer_size);

                    if constexpr( std::is_same_v<T, f32> ) {
                        std::memcpy(dst + off, &value, sizeof(value));
                    } else if constexpr( std::is_same_v<T, v2f> ) {
                        std::memcpy(dst + off, value.data(), sizeof(value));
                    } else if constexpr( std::is_same_v<T, v3f> || std::is_same_v<T, v4f> || std::is_same_v<T, m4f> ) {
                        E2D_ASSERT(off % 4 == 0);
                        std::memcpy(dst + off, value.data(), sizeof(value));
                    } else if constexpr( std::is_same_v<T, m2f> ) {
                        E2D_ASSERT(off % 4 == 0);
                        std::memcpy(dst + off, value.data(), sizeof(value));
                        std::memcpy(dst + off, value.data(), sizeof(value));
                    } else if constexpr( std::is_same_v<T, m3f> ) {
                        E2D_ASSERT(off % 4 == 0);
                        std::memcpy(dst + off, value.data(), sizeof(value));
                        std::memcpy(dst + off + 4, value.data(), sizeof(value));
                        std::memcpy(dst + off + 8, value.data(), sizeof(value));
                    } else {
                        static_assert(false, "unexpected property type");
                    }*/
                });
            });
        }
        return *this;
    }

    render& render::update_texture(
        const texture_ptr& tex,
        const image& img,
        v2u offset)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(tex);

        const pixel_declaration decl =
            convert_image_data_format_to_pixel_declaration(img.format());
        if ( tex->decl() != decl ) {
            state_->dbg().error("RENDER: Failed to update texture:\n"
                "--> Info: incompatible pixel formats\n"
                "--> Texture format: %0\n"
                "--> Image format: %1",
                pixel_declaration::pixel_type_to_cstr(tex->decl().type()),
                pixel_declaration::pixel_type_to_cstr(decl.type()));
            throw bad_render_operation();
        }

        return update_texture(tex, img.data(), b2u(offset, img.size()));
    }

    render& render::update_texture(
        const texture_ptr& tex,
        buffer_view pixels,
        const b2u& region)
    {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(tex);
        E2D_ASSERT(region.position.x < tex->size().x && region.position.y < tex->size().y);
        E2D_ASSERT(region.position.x + region.size.x <= tex->size().x);
        E2D_ASSERT(region.position.y + region.size.y <= tex->size().y);
        E2D_ASSERT(pixels.size() == region.size.y * ((region.size.x * tex->decl().bits_per_pixel()) / 8u));

        if ( tex->decl().is_compressed() ) {
            const v2u block_size = tex->decl().compressed_block_size();
            E2D_ASSERT(region.position.x % block_size.x == 0 && region.position.y % block_size.y == 0);
            E2D_ASSERT(region.size.x % block_size.x == 0 && region.size.y % block_size.y == 0);
            opengl::with_gl_bind_texture(state_->dbg(), tex->state().id(),
                [&tex, &pixels, &region]() noexcept {
                    GL_CHECK_CODE(tex->state().dbg(), glCompressedTexSubImage2D(
                        tex->state().id().target(),
                        0,
                        math::numeric_cast<GLint>(region.position.x),
                        math::numeric_cast<GLint>(region.position.y),
                        math::numeric_cast<GLsizei>(region.size.x),
                        math::numeric_cast<GLsizei>(region.size.y),
                        convert_pixel_type_to_internal_format_e(tex->state().decl().type()),
                        math::numeric_cast<GLsizei>(pixels.size()),
                        pixels.data()));
                });
        } else {
            opengl::with_gl_bind_texture(state_->dbg(), tex->state().id(),
                [&tex, &pixels, &region]() noexcept {
                    GL_CHECK_CODE(tex->state().dbg(), glTexSubImage2D(
                        tex->state().id().target(),
                        0,
                        math::numeric_cast<GLint>(region.position.x),
                        math::numeric_cast<GLint>(region.position.y),
                        math::numeric_cast<GLsizei>(region.size.x),
                        math::numeric_cast<GLsizei>(region.size.y),
                        convert_pixel_type_to_external_format(tex->state().decl().type()),
                        convert_pixel_type_to_external_data_type(tex->state().decl().type()),
                        pixels.data()));
                });
        }
        return *this;
    }

    const render::device_caps& render::device_capabilities() const noexcept {
        E2D_ASSERT(is_in_main_thread());
        return state_->device_capabilities();
    }
    
    const render::statistics& render::frame_statistic() const noexcept {
        E2D_ASSERT(is_in_main_thread());
        return state_->last_stats();
    }

    bool render::is_pixel_supported(const pixel_declaration& decl) const noexcept {
        E2D_ASSERT(is_in_main_thread());
        const auto& caps = device_capabilities();
        const auto& caps_ext = state_->device_capabilities_ext();
        switch ( decl.type() ) {
            case pixel_declaration::pixel_type::depth16:
                return caps.depth_texture_supported
                    && caps_ext.depth16_supported;
            case pixel_declaration::pixel_type::depth24:
                return caps.depth_texture_supported
                    && caps_ext.depth24_supported;
            case pixel_declaration::pixel_type::depth24_stencil8:
                return caps.depth_texture_supported
                    && caps_ext.depth24_stencil8_supported;
            case pixel_declaration::pixel_type::depth32:
                return caps.depth_texture_supported
                    && caps_ext.depth32_supported;
            case pixel_declaration::pixel_type::depth32_stencil8:
                return caps.depth_texture_supported
                    && caps_ext.depth32_stencil8_supported;
            case pixel_declaration::pixel_type::g8:
            case pixel_declaration::pixel_type::ga8:
            case pixel_declaration::pixel_type::rgb8:
            case pixel_declaration::pixel_type::rgba8:
                return true;
            case pixel_declaration::pixel_type::rgb_dxt1:
            case pixel_declaration::pixel_type::rgba_dxt1:
            case pixel_declaration::pixel_type::rgba_dxt3:
            case pixel_declaration::pixel_type::rgba_dxt5:
                return caps.dxt_compression_supported;
            case pixel_declaration::pixel_type::rgb_pvrtc2:
            case pixel_declaration::pixel_type::rgb_pvrtc4:
            case pixel_declaration::pixel_type::rgba_pvrtc2:
            case pixel_declaration::pixel_type::rgba_pvrtc4:
                return caps.pvrtc_compression_supported;
            case pixel_declaration::pixel_type::rgba_pvrtc2_v2:
            case pixel_declaration::pixel_type::rgba_pvrtc4_v2:
                return caps.pvrtc2_compression_supported;
            default:
                E2D_ASSERT_MSG(false, "unexpected pixel type");
                return false;
        }
    }
    
    bool render::get_suitable_depth_texture_pixel_type(pixel_declaration& decl) const noexcept {
        E2D_ASSERT(is_in_main_thread());
        const auto& caps = device_capabilities();
        if ( caps.depth_texture_supported ) {
            return false;
        }
        const auto& caps_ext = state_->device_capabilities_ext();
        if ( caps_ext.depth32_supported ) {
            decl = pixel_declaration::pixel_type::depth32;
            return true;
        }
        if ( caps_ext.depth24_supported ) {
            decl = pixel_declaration::pixel_type::depth24;
            return true;
        }
        if ( caps_ext.depth16_supported ) {
            decl = pixel_declaration::pixel_type::depth16;
            return true;
        }
        return false;
    }
    
    bool render::get_suitable_depth_stencil_texture_pixel_type(pixel_declaration& decl) const noexcept {
        E2D_ASSERT(is_in_main_thread());
        const auto& caps = device_capabilities();
        if ( caps.depth_texture_supported ) {
            return false;
        }
        const auto& caps_ext = state_->device_capabilities_ext();
        if ( caps_ext.depth32_stencil8_supported ) {
            decl = pixel_declaration::pixel_type::depth32_stencil8;
            return true;
        }
        if ( caps_ext.depth24_stencil8_supported ) {
            decl = pixel_declaration::pixel_type::depth24_stencil8;
            return true;
        }
        if ( caps_ext.depth16_stencil8_supported ) {
            decl = pixel_declaration::pixel_type::depth16_stencil8;
            return true;
        }
        return false;
    }

    bool render::is_index_supported(const index_declaration& decl) const noexcept {
        E2D_ASSERT(is_in_main_thread());
        const device_caps& caps = device_capabilities();
        switch ( decl.type() ) {
            case index_declaration::index_type::unsigned_short:
                return true;
            case index_declaration::index_type::unsigned_int:
                return caps.element_index_uint;
            default:
                E2D_ASSERT_MSG(false, "unexpected index type");
                return false;
        }
    }

    bool render::is_vertex_supported(const vertex_declaration& decl) const noexcept {
        E2D_ASSERT(is_in_main_thread());
        return decl.attribute_count() <= device_capabilities().max_vertex_attributes;
    }
}

#endif
#endif

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES

#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL
#  define GLEW_STATIC
#  include <GL/glew.h>
#elif E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>

// GL_OES_depth24
#  define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES

// GL_OES_packed_depth_stencil
#  define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#  define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
#  define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

#if defined(E2D_BUILD_MODE) && E2D_BUILD_MODE == E2D_BUILD_MODE_DEBUG
#   define GL_FLUSH_ERRORS(dbg)\
        for ( GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError() ) {\
            (dbg).log(err == GL_OUT_OF_MEMORY\
                ? debug::level::fatal\
                : debug::level::error,\
                "RENDER: GL_FLUSH_ERRORS():\n"\
                "--> File: %0\n"\
                "--> Line: %1\n"\
                "--> Code: %2",\
                __FILE__, __LINE__, e2d::opengl::gl_error_code_to_cstr(err));\
            E2D_ASSERT_MSG(false, "RENDER: GL_FLUSH_ERRORS()");\
            if ( err == GL_OUT_OF_MEMORY ) std::terminate();\
        }
#   define GL_CHECK_CODE(dbg, code)\
        GL_FLUSH_ERRORS(dbg);\
        code;\
        for ( GLenum err = glGetError(); err != GL_NO_ERROR; err = glGetError() ) {\
            (dbg).log(err == GL_OUT_OF_MEMORY\
                ? debug::level::fatal\
                : debug::level::error,\
                "RENDER: GL_CHECK(%0):\n"\
                "--> File: %1\n"\
                "--> Line: %2\n"\
                "--> Code: %3",\
                #code, __FILE__, __LINE__, e2d::opengl::gl_error_code_to_cstr(err));\
            E2D_ASSERT_MSG(false, #code);\
            if ( err == GL_OUT_OF_MEMORY ) std::terminate();\
        }
#else
#   define GL_FLUSH_ERRORS(dbg) E2D_UNUSED(dbg);
#   define GL_CHECK_CODE(dbg, code) E2D_UNUSED(dbg); code;
#endif

namespace e2d::opengl
{
    class gl_buffer_id final : private noncopyable {
        gl_buffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept;
    public:
        static gl_buffer_id create(debug& debug, GLenum target) noexcept;
        static gl_buffer_id current(debug& debug, GLenum target) noexcept;
    public:
        explicit gl_buffer_id(debug& debug) noexcept;
        ~gl_buffer_id() noexcept;
        gl_buffer_id(gl_buffer_id&& other) noexcept;
        gl_buffer_id& operator=(gl_buffer_id&& other) noexcept;
        gl_buffer_id& swap(gl_buffer_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
        GLenum target() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        GLenum target_ = 0;
        bool owned_ = false;
    };

    class gl_shader_id final : private noncopyable {
        gl_shader_id(debug& debug, GLuint id, GLenum type, bool owned) noexcept;
    public:
        static gl_shader_id create(debug& debug, GLenum type) noexcept;
    public:
        explicit gl_shader_id(debug& debug) noexcept;
        ~gl_shader_id() noexcept;
        gl_shader_id(gl_shader_id&& other) noexcept;
        gl_shader_id& operator=(gl_shader_id&& other) noexcept;
        gl_shader_id& swap(gl_shader_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
        GLenum type() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        GLenum type_ = 0;
        bool owned_ = false;
    };

    class gl_program_id final : private noncopyable {
        gl_program_id(debug& debug, GLuint id, bool owned) noexcept;
    public:
        static gl_program_id create(debug& debug) noexcept;
        static gl_program_id current(debug& debug) noexcept;
    public:
        explicit gl_program_id(debug& debug) noexcept;
        ~gl_program_id() noexcept;
        gl_program_id(gl_program_id&& other) noexcept;
        gl_program_id& operator=(gl_program_id&& other) noexcept;
        gl_program_id& swap(gl_program_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        bool owned_ = false;
    };

    class gl_texture_id final : private noncopyable {
        gl_texture_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept;
    public:
        static gl_texture_id create(debug& debug, GLenum target) noexcept;
        static gl_texture_id current(debug& debug, GLenum target) noexcept;
    public:
        explicit gl_texture_id(debug& debug) noexcept;
        ~gl_texture_id() noexcept;
        gl_texture_id(gl_texture_id&& other) noexcept;
        gl_texture_id& operator=(gl_texture_id&& other) noexcept;
        gl_texture_id& swap(gl_texture_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
        GLenum target() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        GLenum target_ = 0;
        bool owned_ = false;
    };

    class gl_framebuffer_id final : private noncopyable {
        gl_framebuffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept;
    public:
        static gl_framebuffer_id create(debug& debug, GLenum target) noexcept;
        static gl_framebuffer_id current(debug& debug, GLenum target) noexcept;
    public:
        explicit gl_framebuffer_id(debug& debug) noexcept;
        ~gl_framebuffer_id() noexcept;
        gl_framebuffer_id(gl_framebuffer_id&& other) noexcept;
        gl_framebuffer_id& operator=(gl_framebuffer_id&& other) noexcept;
        gl_framebuffer_id& swap(gl_framebuffer_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
        GLenum target() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        GLenum target_ = 0;
        bool owned_ = false;
    };

    class gl_renderbuffer_id final : private noncopyable {
        gl_renderbuffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept;
    public:
        static gl_renderbuffer_id create(debug& debug, GLenum target) noexcept;
        static gl_renderbuffer_id current(debug& debug, GLenum target) noexcept;
    public:
        explicit gl_renderbuffer_id(debug& debug) noexcept;
        ~gl_renderbuffer_id() noexcept;
        gl_renderbuffer_id(gl_renderbuffer_id&& other) noexcept;
        gl_renderbuffer_id& operator=(gl_renderbuffer_id&& other) noexcept;
        gl_renderbuffer_id& swap(gl_renderbuffer_id& other) noexcept;
        void reset() noexcept;
        bool empty() const noexcept;
        GLuint operator*() const noexcept;
        GLenum target() const noexcept;
    private:
        debug& debug_;
        GLuint id_ = 0;
        GLenum target_ = 0;
        bool owned_ = false;
    };

    bool operator==(const gl_buffer_id& l, const gl_buffer_id& r) noexcept;
    bool operator==(const gl_shader_id& l, const gl_shader_id& r) noexcept;
    bool operator==(const gl_program_id& l, const gl_program_id& r) noexcept;
    bool operator==(const gl_texture_id& l, const gl_texture_id& r) noexcept;
    bool operator==(const gl_framebuffer_id& l, const gl_framebuffer_id& r) noexcept;
    bool operator==(const gl_renderbuffer_id& l, const gl_renderbuffer_id& r) noexcept;

    bool operator!=(const gl_buffer_id& l, const gl_buffer_id& r) noexcept;
    bool operator!=(const gl_shader_id& l, const gl_shader_id& r) noexcept;
    bool operator!=(const gl_program_id& l, const gl_program_id& r) noexcept;
    bool operator!=(const gl_texture_id& l, const gl_texture_id& r) noexcept;
    bool operator!=(const gl_framebuffer_id& l, const gl_framebuffer_id& r) noexcept;
    bool operator!=(const gl_renderbuffer_id& l, const gl_renderbuffer_id& r) noexcept;
}

namespace e2d::opengl
{
    enum class uniform_type : u8 {
        signed_integer,
        floating_point,

        v2i,
        v3i,
        v4i,

        v2f,
        v3f,
        v4f,

        m2f,
        m3f,
        m4f,

        sampler_2d,
        sampler_cube,

        unknown
    };

    enum class attribute_type : u8 {
        floating_point,

        v2f,
        v3f,
        v4f,

        m2f,
        m3f,
        m4f,

        unknown
    };

    const char* uniform_type_to_cstr(uniform_type ut) noexcept;
    const char* attribute_type_to_cstr(attribute_type at) noexcept;
}

namespace e2d::opengl
{
    GLenum convert_image_data_format_to_external_format(image_data_format f) noexcept;
    GLenum convert_image_data_format_to_external_data_type(image_data_format f) noexcept;

    GLenum convert_pixel_type_to_external_format(pixel_declaration::pixel_type f) noexcept;
    GLenum convert_pixel_type_to_external_data_type(pixel_declaration::pixel_type f) noexcept;

    GLint convert_pixel_type_to_internal_format(pixel_declaration::pixel_type f) noexcept;
    GLenum convert_pixel_type_to_internal_format_e(pixel_declaration::pixel_type f) noexcept;
    pixel_declaration convert_image_data_format_to_pixel_declaration(image_data_format f) noexcept;

    GLenum convert_index_type(index_declaration::index_type it) noexcept;
    GLenum convert_attribute_type(vertex_declaration::attribute_type at) noexcept;

    GLint convert_uniform_type(uniform_type ut) noexcept;
    GLint convert_attribute_type(attribute_type at) noexcept;

    GLint convert_sampler_wrap(render::sampler_wrap w) noexcept;
    GLint convert_sampler_filter(render::sampler_min_filter f) noexcept;
    GLint convert_sampler_filter(render::sampler_mag_filter f) noexcept;

    GLenum convert_buffer_usage(index_buffer::usage u) noexcept;
    GLenum convert_buffer_usage(vertex_buffer::usage u) noexcept;
    GLenum convert_topology(render::topology t) noexcept;
    GLenum convert_stencil_op(render::stencil_op sa) noexcept;
    GLenum convert_compare_func(render::compare_func cf) noexcept;
    GLenum convert_culling_mode(render::culling_mode cm) noexcept;
    GLenum convert_culling_face(render::culling_face cf) noexcept;
    GLenum convert_blending_factor(render::blending_factor bf) noexcept;
    GLenum convert_blending_equation(render::blending_equation be) noexcept;

    uniform_type glsl_type_to_uniform_type(GLenum t) noexcept;
    attribute_type glsl_type_to_attribute_type(GLenum t) noexcept;

    const char* glsl_type_to_cstr(GLenum t) noexcept;
    const char* gl_error_code_to_cstr(GLenum e) noexcept;
    const char* gl_framebuffer_status_to_cstr(GLenum s) noexcept;
    GLenum gl_target_to_get_target(GLenum t) noexcept;
}

namespace e2d::opengl
{
    void gl_trace_info(debug& debug) noexcept;
    void gl_trace_limits(debug& debug) noexcept;
    void gl_fill_device_caps(debug& debug, render::device_caps& caps) noexcept;
    bool gl_has_extension(debug& debug, str_view name) noexcept;

    gl_shader_id gl_compile_shader(
        debug& debug,
        str_view header,
        str_view source,
        GLenum type) noexcept;

    gl_program_id gl_link_program(
        debug& debug,
        gl_shader_id vs,
        gl_shader_id fs) noexcept;

    bool gl_check_framebuffer(
        debug& debug,
        const gl_framebuffer_id& fb,
        GLenum* out_status) noexcept;

    void gl_attach_texture(
        debug& debug,
        const gl_framebuffer_id& fb,
        const gl_texture_id& tex,
        GLenum attachment) noexcept;

    void gl_attach_renderbuffer(
        debug& debug,
        const gl_framebuffer_id& fb,
        const gl_renderbuffer_id& rb,
        GLenum attachment) noexcept;

    gl_renderbuffer_id gl_compile_renderbuffer(
        debug& debug,
        const v2u& size,
        GLenum format);
}

namespace e2d::opengl
{
    struct uniform_info {
        str_hash name;
        GLint size = 0;
        GLint location = -1;
        uniform_type type = uniform_type::floating_point;
    public:
        uniform_info(
            str_hash nname,
            GLint nsize,
            GLint nlocation,
            uniform_type ntype) noexcept
        : name(std::move(nname))
        , size(nsize)
        , location(nlocation)
        , type(ntype) {}
    };

    struct attribute_info {
        str_hash name;
        GLint size = 0;
        GLint location = -1;
        attribute_type type = attribute_type::floating_point;
    public:
        attribute_info(
            str_hash nname,
            GLint nsize,
            GLint nlocation,
            attribute_type ntype) noexcept
        : name(std::move(nname))
        , size(nsize)
        , location(nlocation)
        , type(ntype) {}
    };

    void grab_program_uniforms(
        debug& debug,
        GLuint program,
        vector<uniform_info>& uniforms);

    void grab_program_attributes(
        debug& debug,
        GLuint program,
        vector<attribute_info>& attributes);
}

namespace e2d::opengl
{
    //
    // with_gl_use_program
    //

    template < typename F, typename... Args >
    void with_gl_use_program(debug& debug, GLuint program, F&& f, Args&&... args) {
        GLint prev_program = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(
            GL_CURRENT_PROGRAM, &prev_program));
        GL_CHECK_CODE(debug, glUseProgram(
            program));
        try {
            stdex::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } catch (...) {
            GL_CHECK_CODE(debug, glUseProgram(
                math::numeric_cast<GLuint>(prev_program)));
            throw;
        }
        GL_CHECK_CODE(debug, glUseProgram(
            math::numeric_cast<GLuint>(prev_program)));
    }

    template < typename F, typename... Args >
    void with_gl_use_program(debug& debug, const gl_program_id& program, F&& f, Args&&... args) {
        with_gl_use_program(debug, *program, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //
    // with_gl_bind_buffer
    //

    template < typename F, typename... Args >
    void with_gl_bind_buffer(debug& debug, GLenum target, GLuint buffer, F&& f, Args&&... args) {
        GLint prev_buffer = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(
            gl_target_to_get_target(target), &prev_buffer));
        GL_CHECK_CODE(debug, glBindBuffer(
            target, buffer));
        try {
            stdex::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } catch (...) {
            GL_CHECK_CODE(debug, glBindBuffer(
                target, math::numeric_cast<GLuint>(prev_buffer)));
            throw;
        }
        GL_CHECK_CODE(debug, glBindBuffer(
            target, math::numeric_cast<GLuint>(prev_buffer)));
    }

    template < typename F, typename... Args >
    void with_gl_bind_buffer(debug& debug, const gl_buffer_id& buffer, F&& f, Args&&... args) {
        with_gl_bind_buffer(debug, buffer.target(), *buffer, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //
    // with_gl_bind_texture
    //

    template < typename F, typename... Args >
    void with_gl_bind_texture(debug& debug, GLenum target, GLuint texture, F&& f, Args&&... args) {
        GLint prev_texture = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(
            gl_target_to_get_target(target), &prev_texture));
        GL_CHECK_CODE(debug, glBindTexture(
            target, texture));
        try {
            stdex::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } catch (...) {
            GL_CHECK_CODE(debug, glBindTexture(
                target, math::numeric_cast<GLuint>(prev_texture)));
            throw;
        }
        GL_CHECK_CODE(debug, glBindTexture(
            target, math::numeric_cast<GLuint>(prev_texture)));
    }

    template < typename F, typename... Args >
    void with_gl_bind_texture(debug& debug, const gl_texture_id& texture, F&& f, Args&&... args) {
        with_gl_bind_texture(debug, texture.target(), *texture, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //
    // with_gl_bind_framebuffer
    //

    template < typename F, typename... Args >
    void with_gl_bind_framebuffer(debug& debug, GLenum target, GLuint framebuffer, F&& f, Args&&... args) {
        GLint prev_framebuffer = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(
            gl_target_to_get_target(target), &prev_framebuffer));
        GL_CHECK_CODE(debug, glBindFramebuffer(
            target, framebuffer));
        try {
            stdex::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } catch (...) {
            GL_CHECK_CODE(debug, glBindFramebuffer(
                target, math::numeric_cast<GLuint>(prev_framebuffer)));
            throw;
        }
        GL_CHECK_CODE(debug, glBindFramebuffer(
            target, math::numeric_cast<GLuint>(prev_framebuffer)));
    }

    template < typename F, typename... Args >
    void with_gl_bind_framebuffer(debug& debug, const gl_framebuffer_id& framebuffer, F&& f, Args&&... args) {
        with_gl_bind_framebuffer(debug, framebuffer.target(), *framebuffer, std::forward<F>(f), std::forward<Args>(args)...);
    }

    //
    // with_gl_bind_renderbuffer
    //

    template < typename F, typename... Args >
    void with_gl_bind_renderbuffer(debug& debug, GLenum target, GLuint renderbuffer, F&& f, Args&&... args) {
        GLint prev_renderbuffer = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(
            gl_target_to_get_target(target), &prev_renderbuffer));
        GL_CHECK_CODE(debug, glBindRenderbuffer(
            target, renderbuffer));
        try {
            stdex::invoke(std::forward<F>(f), std::forward<Args>(args)...);
        } catch (...) {
            GL_CHECK_CODE(debug, glBindRenderbuffer(
                target, math::numeric_cast<GLuint>(prev_renderbuffer)));
            throw;
        }
        GL_CHECK_CODE(debug, glBindRenderbuffer(
            target, math::numeric_cast<GLuint>(prev_renderbuffer)));
    }

    template < typename F, typename... Args >
    void with_gl_bind_renderbuffer(debug& debug, const gl_renderbuffer_id& renderbuffer, F&& f, Args&&... args) {
        with_gl_bind_renderbuffer(debug, renderbuffer.target(), *renderbuffer, std::forward<F>(f), std::forward<Args>(args)...);
    }
}

#endif
#endif

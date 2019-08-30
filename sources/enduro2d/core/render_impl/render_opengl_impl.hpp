/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include "render_opengl_base.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES

namespace e2d
{
    //
    // shader::internal_state
    //

    class shader::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_program_id id);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_program_id& id() const noexcept;
    public:
        template < typename F >
        void with_uniform_location(str_hash name, F&& f) const;
        template < typename F >
        void with_attribute_location(str_hash name, F&& f) const;
    private:
        debug& debug_;
        opengl::gl_program_id id_;
        hash_map<str_hash, opengl::uniform_info> uniforms_;
        hash_map<str_hash, opengl::attribute_info> attributes_;
    };

    template < typename F >
    void shader::internal_state::with_uniform_location(str_hash name, F&& f) const {
        const auto iter = uniforms_.find(name);
        if ( iter != uniforms_.end() ) {
            stdex::invoke(std::forward<F>(f), iter->second);
        }
    }

    template < typename F >
    void shader::internal_state::with_attribute_location(str_hash name, F&& f) const {
        const auto iter = attributes_.find(name);
        if ( iter != attributes_.end() ) {
            stdex::invoke(std::forward<F>(f), iter->second);
        }
    }

    //
    // texture::internal_state
    //

    class texture::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_texture_id id,
            const v2u& size,
            const pixel_declaration& decl);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_texture_id& id() const noexcept;
        const v2u& size() const noexcept;
        const pixel_declaration& decl() const noexcept;
    private:
        debug& debug_;
        opengl::gl_texture_id id_;
        v2u size_;
        pixel_declaration decl_;
    };

    //
    // index_buffer::internal_state
    //

    class index_buffer::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_buffer_id id,
            std::size_t size,
            const index_declaration& decl);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_buffer_id& id() const noexcept;
        std::size_t size() const noexcept;
        const index_declaration& decl() const noexcept;
    private:
        debug& debug_;
        opengl::gl_buffer_id id_;
        std::size_t size_ = 0;
        index_declaration decl_;
    };

    //
    // vertex_buffer::internal_state
    //

    class vertex_buffer::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_buffer_id id,
            std::size_t size,
            const vertex_declaration& decl);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_buffer_id& id() const noexcept;
        std::size_t size() const noexcept;
        const vertex_declaration& decl() const noexcept;
    private:
        debug& debug_;
        opengl::gl_buffer_id id_;
        std::size_t size_ = 0;
        vertex_declaration decl_;
    };

    //
    // render_target::internal_state
    //

    class render_target::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_framebuffer_id id,
            const v2u& size,
            texture_ptr color,
            texture_ptr depth,
            opengl::gl_renderbuffer_id color_rb,
            opengl::gl_renderbuffer_id depth_rb);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_framebuffer_id& id() const noexcept;
        const v2u& size() const noexcept;
        const texture_ptr& color() const noexcept;
        const texture_ptr& depth() const noexcept;
        const opengl::gl_renderbuffer_id& color_rb() const noexcept;
        const opengl::gl_renderbuffer_id& depth_rb() const noexcept;
    private:
        debug& debug_;
        opengl::gl_framebuffer_id id_;
        v2u size_;
        texture_ptr color_;
        texture_ptr depth_;
        opengl::gl_renderbuffer_id color_rb_;
        opengl::gl_renderbuffer_id depth_rb_;
    };

    //
    // render::internal_state
    //

    class render::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            window& window);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        window& wnd() const noexcept;
        const device_caps& device_capabilities() const noexcept;
        const render_target_ptr& render_target() const noexcept;
    public:
        internal_state& reset_states() noexcept;
        internal_state& set_states(const state_block& sb) noexcept;
        
        internal_state& reset_shader_program() noexcept;
        internal_state& set_shader_program(const shader_ptr& sp) noexcept;
        
        internal_state& reset_render_target() noexcept;
        internal_state& set_render_target(const render_target_ptr& rt) noexcept;
    private:
        void set_depth_state_(const depth_state& ds) noexcept;
        void set_stencil_state_(const stencil_state& ss) noexcept;
        void set_culling_state_(const culling_state& cs) noexcept;
        void set_blending_state_(const blending_state& bs) noexcept;
        void set_capabilities_state_(const capabilities_state& cs) noexcept;
        void create_debug_output_() noexcept;
        static void GLAPIENTRY debug_output_callback_(
            GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar* message,
            const void* userParam);
    private:
        debug& debug_;
        window& window_;
        device_caps device_caps_;
        state_block state_block_;
        shader_ptr shader_program_;
        render_target_ptr render_target_;
        opengl::gl_program_id default_sp_;
        opengl::gl_framebuffer_id default_fb_;
    };
}

#endif
#endif

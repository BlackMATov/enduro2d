/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include "render_opengl_base.hpp"

#if defined(E2D_RENDER_MODE) && E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL

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
            image_data_format format);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_texture_id& id() const noexcept;
        const v2u& size() const noexcept;
        image_data_format format() const noexcept;
    private:
        debug& debug_;
        opengl::gl_texture_id id_;
        v2u size_;
        image_data_format format_;
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
    public:
        internal_state& set_states(const state_block& rs) noexcept;
        internal_state& set_depth_state(const depth_state& ds) noexcept;
        internal_state& set_stencil_state(const stencil_state& ss) noexcept;
        internal_state& set_culling_state(const culling_state& cs) noexcept;
        internal_state& set_blending_state(const blending_state& bs) noexcept;
        internal_state& set_capabilities_state(const capabilities_state& cs) noexcept;
    private:
        debug& debug_;
        window& window_;
    };
}

#endif
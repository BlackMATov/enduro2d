/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "render.hpp"
#include "render_opengl_base.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || \
    E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES || \
    E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES3

namespace e2d
{
    //
    // shader::internal_state
    //

    class shader::internal_state final : private e2d::noncopyable {
    public:
        struct block_info {
            cbuffer_template_cptr templ;
            u32 index = ~0u; // uniform array location or uniform buffer binding
            bool is_buffer = false;
        };
    public:
        internal_state(
            debug& debug,
            opengl::gl_program_id id,
            const shader_source& source,
            bool uniform_buffer_supported);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_program_id& id() const noexcept;
    public:
        template < typename F >
        void with_uniform_location(str_hash name, const_buffer::scope scope, F&& f) const;
        template < typename F >
        void with_attribute_location(str_hash name, F&& f) const;
        template < typename F >
        void with_sampler_location(str_hash name, F&& f) const;
        block_info get_block_info(const_buffer::scope scope) const noexcept;
        void set_constants(const_buffer::scope scope, const const_buffer_ptr& cb) const noexcept;
    private:
        debug& debug_;
        opengl::gl_program_id id_;
    private:
        using attribute_type = shader_source::value_type;
        using sampler_type = shader_source::sampler_type;
        struct attribute_info {
            str_hash name;
            u8 index;
            attribute_type type;
        };
        struct sampler_info {
            str_hash name;
            u8 unit;
            sampler_type type;
            render::sampler_block::scope scope;
        };
        struct cbuffer {
            mutable std::weak_ptr<const_buffer> buffer;
            mutable u32 version = 0;
            block_info info;
        };
        std::array<cbuffer, u32(const_buffer::scope::last_)> blocks_;
        flat_map<str_hash, attribute_info> attributes_;
        flat_map<str_hash, sampler_info> samplers_;
    };

    template < typename F >
    void shader::internal_state::with_uniform_location(str_hash name, const_buffer::scope scope, F&& f) const {
        auto& templ = blocks_[size_t(scope)].templ;
        if ( templ ) {
            const auto iter = templ->uniforms().find(name);
            if ( iter != templ->uniforms().end() ) {
                stdex::invoke(std::forward<F>(f), iter->second);
            }
        }
        E2D_ASSERT_MSG(false, "uniform is not exists");
    }

    template < typename F >
    void shader::internal_state::with_attribute_location(str_hash name, F&& f) const {
        const auto iter = attributes_.find(name);
        if ( iter != attributes_.end() ) {
            stdex::invoke(std::forward<F>(f), iter->second);
        }
    }

    template < typename F >
    void shader::internal_state::with_sampler_location(str_hash name, F&& f) const {
        const auto iter = samplers_.find(name);
        if ( iter != samplers_.end() ) {
            stdex::invoke(std::forward<F>(f), iter->second);
        } else {
            E2D_ASSERT_MSG(false, "sampler is not exists");
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
            std::size_t size);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_buffer_id& id() const noexcept;
        std::size_t size() const noexcept;
    private:
        debug& debug_;
        opengl::gl_buffer_id id_;
        std::size_t size_ = 0;
    };

    //
    // vertex_attribs::internal_state
    //

    class vertex_attribs::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            const vertex_declaration& decl);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const vertex_declaration& decl() const noexcept;
        std::size_t hash() const noexcept;
    private:
        debug& debug_;
        std::size_t hash_;
        vertex_declaration decl_;
    };
    
    //
    // const_buffer::internal_state
    //

    class const_buffer::internal_state final : private e2d::noncopyable {
    public:
        internal_state(
            debug& debug,
            opengl::gl_buffer_id id,
            std::size_t offset,
            scope scope,
            const cbuffer_template_cptr& templ);
        ~internal_state() noexcept = default;
    public:
        debug& dbg() const noexcept;
        const opengl::gl_buffer_id& id() const noexcept;
        std::size_t size() const noexcept;
        std::size_t offset() const noexcept;
        float* data() const noexcept;
        scope binding_scope() const noexcept;
        u32 version() const noexcept;
        const cbuffer_template_cptr& block_template() const noexcept;
        bool is_compatible_with(const shader_ptr& shader) const noexcept;
        void on_content_update(u32 frame_id) const noexcept;
    private:
        debug& debug_;
        opengl::gl_buffer_id id_;
        std::size_t offset_ = 0; // of buffer is part of another buffer
        scope binding_scope_ = scope::last_;
        cbuffer_template_cptr templ_;
        mutable std::unique_ptr<float[]> content_;
        mutable u32 version_ = 0;
        mutable u32 last_update_frame_id_ = 0;
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
        const opengl::gl_device_caps& device_capabilities_ext() const noexcept;

        statistics& stats() noexcept;
        const statistics& last_stats() const noexcept;

        u32 frame_id() const noexcept;
        bool inside_render_pass() const noexcept;

        const str& vertex_shader_header() const noexcept;
        const str& fragment_shader_header() const noexcept;

        void on_present() noexcept;

        vertex_attribs_ptr create_vertex_attribs(
            const vertex_declaration& decl);
    public:
        internal_state& reset_states() noexcept;
        internal_state& set_states(const state_block& sb) noexcept;
        
        internal_state& reset_shader_program() noexcept;
        internal_state& set_shader_program(const shader_ptr& sp) noexcept;
        
        void begin_render_pass(const renderpass_desc& rp) noexcept;
        void end_render_pass() noexcept;
        
        void bind_index_buffer(
            const index_buffer_ptr& ibuffer) noexcept;
        void bind_vertex_buffer(
            std::size_t index,
            const vertex_buffer_ptr& vbuffer,
            const vertex_attribs_ptr& attribs,
            std::size_t offset) noexcept;
        void bind_const_buffer(
            const const_buffer_ptr& cbuffer) noexcept;
        void bind_textures(
            sampler_block::scope scope,
            const sampler_block& samplers) noexcept;

        void draw(topology topo, u32 first, u32 count) noexcept;
        void draw_indexed(topology topo, u32 count, size_t offset) noexcept;

        void set_blending_state(const std::optional<blending_state>& state) noexcept;
        void set_culling_state(const std::optional<culling_state>& state) noexcept;
        void set_depth_state(const std::optional<depth_state>& state) noexcept;
        void set_stencil_state(const std::optional<stencil_state>& state) noexcept;
        void set_scissor(bool enable, const b2u& rect = b2u()) noexcept;

        void insert_message(str_view msg) noexcept;
    private:
        void set_depth_state_(const depth_state& ds) noexcept;
        void set_stencil_state_(const stencil_state& ss) noexcept;
        void set_culling_state_(const culling_state& rs) noexcept;
        void set_blending_state_(const blending_state& bs) noexcept;
        void set_render_target_(const render_target_ptr& rt) noexcept;
        void reset_render_target_() noexcept;
        void commit_changes_() noexcept;
        void bind_vertex_attributes_() noexcept;
        void bind_cbuffers_() noexcept;
        void bind_cbuffer_(u32 index, const const_buffer_ptr& cb) noexcept;
        void bind_textures_() noexcept;
        void bind_sampler_block_(const sampler_block& block) noexcept;
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
        enum class dirty_flag_bits : u32 {
            none = 0,
            vertex_attribs = 1 << 0,

            pass_cbuffer = 1 << 2,
            mtr_cbuffer = 1 << 3,
            draw_cbuffer = 1 << 4,
            cbuffers = pass_cbuffer | mtr_cbuffer | draw_cbuffer,

            pass_textures = 1 << 5,
            mtr_textures = 1 << 6,
            textures = pass_textures | mtr_textures,

            pipeline = vertex_attribs | cbuffers | textures,
        };
        using enabled_attribs_t = std::bitset<render_cfg::max_attribute_count>;
    private:
        debug& debug_;
        window& window_;

        device_caps device_caps_;
        opengl::gl_device_caps device_caps_ext_;
        opengl::gl_program_id default_sp_;
        opengl::gl_framebuffer_id default_fb_;

        // graphics pipeline states
        state_block render_pass_state_block_;
        state_block state_block_;
        shader_ptr shader_program_;
        
        // render pass states
        render_target_ptr render_target_;
        b2u render_area_;
        attachment_store_op color_store_op_ = attachment_store_op::store;
        attachment_store_op depth_store_op_ = attachment_store_op::discard;
        attachment_store_op stencil_store_op_ = attachment_store_op::discard;
        bool inside_render_pass_ = false;

        // graphics resource bindings
        struct vb_binding {
            vertex_buffer_ptr buffer;
            vertex_attribs_ptr attribs;
            std::size_t offset;
        };
        std::array<vb_binding, render_cfg::max_vertex_buffer_count> vertex_buffers_;
        std::array<const_buffer_ptr, u32(const_buffer::scope::last_)> cbuffers_;
        std::array<sampler_block, u32(sampler_block::scope::last_)> samplers_;
        //std::array<std::pair<GLuint, GLenum>, 16> textures_;
        index_buffer_ptr index_buffer_;
        enabled_attribs_t enabled_attribs_;
        dirty_flag_bits dirty_flags_ = dirty_flag_bits::none;

        // statistic
        statistics current_stat_;
        statistics last_frame_stat_;
        u32 frame_id_ = 1;

        // utils
        str vertex_shader_header_;
        str fragment_shader_header_;

        struct vert_attrib_less {
            bool operator()(const vertex_attribs_ptr& l, const vertex_attribs_ptr& r) const noexcept;
        };
        flat_set<vertex_attribs_ptr, vert_attrib_less> vertex_attrib_cache_;
    };
}

#endif
#endif

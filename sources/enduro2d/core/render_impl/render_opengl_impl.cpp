/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_opengl_impl.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || \
    E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES || \
    E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES3

namespace
{
    using namespace e2d;
    using namespace e2d::opengl;

    constexpr u32 cb_pass_index = u32(const_buffer::scope::render_pass);
    constexpr u32 cb_material_index = u32(const_buffer::scope::material);
    constexpr u32 cb_command_index = u32(const_buffer::scope::draw_command);
    
    template < typename T >
    size_t hash_of(const T& x) noexcept {
        return std::hash<T>()(x);
    }
    
    size_t hash_of(str_hash x) noexcept {
        return x.hash();
    }

    size_t hash_of(const vertex_declaration::attribute_info& x) noexcept {
        size_t h = 0;
        h = utils::hash_combine(h, hash_of(x.stride));
        h = utils::hash_combine(h, hash_of(x.name));
        h = utils::hash_combine(h, hash_of(x.rows));
        h = utils::hash_combine(h, hash_of(x.columns));
        h = utils::hash_combine(h, hash_of(x.type));
        h = utils::hash_combine(h, hash_of(x.normalized));
        return h;
    }

    size_t hash_of(const vertex_declaration& x) noexcept {
        size_t h = hash_of(x.attribute_count());
        for ( size_t i = 0; i < x.attribute_count(); ++i ) {
            h = utils::hash_combine(h, hash_of(x.attribute(i)));
        }
        h = utils::hash_combine(h, hash_of(x.bytes_per_vertex()));
        return h;
    }
    
    template < typename T >
    void set_flag_inplace(T& le, T re) noexcept {
        auto l = utils::enum_to_underlying(le);
        auto r = utils::enum_to_underlying(re);
        le = static_cast<T>(l | r);
    }

    template < typename T >
    bool check_flag_and_reset(T& le, T re) noexcept {
        auto l = utils::enum_to_underlying(le);
        auto r = utils::enum_to_underlying(re);
        bool res = (l & r) == r;
        le = static_cast<T>(l & ~r);
        return res;
    }

    const_buffer::scope convert_to_const_buffer_scope(
        shader_source::scope_type scope) noexcept
    {
        #define DEFINE_CASE(x,y) case shader_source::scope_type::x: return const_buffer::scope::y;
        switch ( scope ) {
            DEFINE_CASE(render_pass, render_pass);
            DEFINE_CASE(material, material);
            DEFINE_CASE(draw_command, draw_command);
            default:
                E2D_ASSERT_MSG(false, "unexpected const buffer scope type");
                return const_buffer::scope::last_;
        }
        #undef DEFINE_CASE
    }

    render::sampler_block::scope convert_to_sampler_block_scope(
        shader_source::scope_type scope) noexcept
    {
        #define DEFINE_CASE(x,y) case shader_source::scope_type::x: return render::sampler_block::scope::y;
        switch ( scope ) {
            DEFINE_CASE(render_pass, render_pass);
            DEFINE_CASE(material, material);
            default:
                E2D_ASSERT_MSG(false, "unexpected sampler block scope type");
                return render::sampler_block::scope::last_;
        }
        #undef DEFINE_CASE
    }

    void get_unifom_block_info(
        debug& debug,
        const char* name,
        const gl_program_id& id,
        shader::internal_state::block_info& block,
        u32 binding_index,
        const cbuffer_template_cptr& templ,
        bool uniform_buffer_supported) noexcept
    {
        GLint loc;
        GL_CHECK_CODE(debug, loc = glGetUniformLocation(*id, name));
        if ( loc >= 0 ) {
            E2D_ASSERT(templ);
            block.templ = templ;
            block.index = math::numeric_cast<u32>(loc);
            block.is_buffer = false;
            return;
        }
        
        if ( uniform_buffer_supported ) {
            GL_CHECK_CODE(debug, loc = glGetUniformBlockIndex(*id, name));
            if ( loc >= 0 ) {
                GL_CHECK_CODE(debug, glUniformBlockBinding(*id, loc, binding_index));
                E2D_ASSERT(templ);
                block.templ = templ;
                block.index = binding_index;
                block.is_buffer = true;
                return;
            }
        }
        
        E2D_ASSERT(!templ);
    }
}

namespace e2d
{
    //
    // shader::internal_state
    //

    shader::internal_state::internal_state(
        debug& debug,
        gl_program_id id,
        const shader_source& source,
        bool uniform_buffer_supported)
    : debug_(debug)
    , id_(std::move(id)){
        E2D_ASSERT(!id_.empty());

        // bind vertex attributes
        for ( auto& attr : source.attributes() ) {
            GL_CHECK_CODE(debug_, glBindAttribLocation(
                *id_, attr.index, attr.name.c_str()));
            attributes_.emplace(attr.name,
                attribute_info{str_hash(attr.name), attr.index, attr.type});
        }

        // apply new attribute indices
        GL_CHECK_CODE(debug_, glLinkProgram(*id_));

        // bind uniforms
        with_gl_use_program(debug_, *id_, [this, &source]() noexcept{
            for ( auto& samp : source.samplers() ) {
                GLint loc;
                GL_CHECK_CODE(debug_, loc = glGetUniformLocation(*id_, samp.name.c_str()));
                E2D_ASSERT(loc >= 0); // TODO: log
                GL_CHECK_CODE(debug_, glUniform1i(loc, samp.unit));
                samplers_.emplace(samp.name, sampler_info{
                    str_hash(samp.name),
                    samp.unit,
                    samp.type,
                    convert_to_sampler_block_scope(samp.scope)});
            }
        });

        get_unifom_block_info(debug_,
            shader_source::cb_pass_name,
            id_,
            blocks_[cb_pass_index].info,
            cb_pass_index,
            source.block(shader_source::scope_type::render_pass),
            uniform_buffer_supported);
        get_unifom_block_info(debug_,
            shader_source::cb_material_name,
            id_,
            blocks_[cb_material_index].info,
            cb_material_index,
            source.block(shader_source::scope_type::material),
            uniform_buffer_supported);
        get_unifom_block_info(debug_,
            shader_source::cb_command_name,
            id_,
            blocks_[cb_command_index].info,
            cb_command_index,
            source.block(shader_source::scope_type::draw_command),
            uniform_buffer_supported);
    }

    debug& shader::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_program_id& shader::internal_state::id() const noexcept {
        return id_;
    }

    shader::internal_state::block_info
    shader::internal_state::get_block_info(const_buffer::scope scope) const noexcept {
        return blocks_[size_t(scope)].info;
    }

    void shader::internal_state::set_constants(
        const_buffer::scope scope,
        const const_buffer_ptr& cbuffer) const noexcept
    {
        auto& curr = blocks_[size_t(scope)];
        if ( !curr.info.templ ) {
            return;
        }
        if ( !cbuffer ) {
            E2D_ASSERT_MSG(false, "const_buffer is null, will be used the last uniform values");
            return;
        }
        if ( !curr.info.is_buffer ) {
            // emulate uniform buffer
            auto& cb = cbuffer->state();
            if ( curr.buffer.lock() != cbuffer ||
                 curr.version != cb.version() )
            {
                GL_CHECK_CODE(debug_, glUniform4fv(
                    curr.info.index,
                    GLsizei(cb.size() / sizeof(v4f)),
                    cb.data()));
                curr.buffer = cbuffer;
                curr.version = cb.version();
            }
        }
    }

    //
    // texture::internal_state
    //

    texture::internal_state::internal_state(
        debug& debug,
        gl_texture_id id,
        const v2u& size,
        const pixel_declaration& decl)
    : debug_(debug)
    , id_(std::move(id))
    , size_(size)
    , decl_(decl){
        E2D_ASSERT(!id_.empty());
    }

    debug& texture::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_texture_id& texture::internal_state::id() const noexcept {
        return id_;
    }

    const v2u& texture::internal_state::size() const noexcept {
        return size_;
    }

    const pixel_declaration& texture::internal_state::decl() const noexcept {
        return decl_;
    }

    //
    // index_buffer::internal_state
    //

    index_buffer::internal_state::internal_state(
        debug& debug,
        gl_buffer_id id,
        std::size_t size,
        const index_declaration& decl)
    : debug_(debug)
    , id_(std::move(id))
    , size_(size)
    , decl_(decl) {
        E2D_ASSERT(!id_.empty());
    }

    debug& index_buffer::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_buffer_id& index_buffer::internal_state::id() const noexcept {
        return id_;
    }

    std::size_t index_buffer::internal_state::size() const noexcept {
        return size_;
    }

    const index_declaration& index_buffer::internal_state::decl() const noexcept {
        return decl_;
    }

    //
    // vertex_buffer::internal_state
    //

    vertex_buffer::internal_state::internal_state(
        debug& debug,
        gl_buffer_id id,
        std::size_t size)
    : debug_(debug)
    , id_(std::move(id))
    , size_(size) {
        E2D_ASSERT(!id_.empty());
    }

    debug& vertex_buffer::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_buffer_id& vertex_buffer::internal_state::id() const noexcept {
        return id_;
    }

    std::size_t vertex_buffer::internal_state::size() const noexcept {
        return size_;
    }
    
    //
    // vertex_attribs::internal_state
    //

    vertex_attribs::internal_state::internal_state(
        debug& debug,
        const vertex_declaration& decl)
    : debug_(debug)
    , hash_(hash_of(decl))
    , decl_(decl) {}

    debug& vertex_attribs::internal_state::dbg() const noexcept {
        return debug_;
    }
    
    const vertex_declaration& vertex_attribs::internal_state::decl() const noexcept {
        return decl_;
    }
    
    std::size_t vertex_attribs::internal_state::hash() const noexcept {
        return hash_;
    }

    bool render::internal_state::vert_attrib_less::operator()(const vertex_attribs_ptr& l, const vertex_attribs_ptr& r) const noexcept {
        return l->state().hash() < r->state().hash();
    }

    //
    // const_buffer::internal_state
    //

    const_buffer::internal_state::internal_state(
        debug& debug,
        gl_buffer_id id,
        std::size_t offset,
        scope scope,
        const cbuffer_template_cptr& templ)
    : debug_(debug)
    , id_(std::move(id))
    , offset_(offset)
    , binding_scope_(scope)
    , content_(new float[math::align_ceil(templ->block_size(), sizeof(v4f))])
    , templ_(templ)
    {
        E2D_ASSERT(templ_);
    }

    debug& const_buffer::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_buffer_id& const_buffer::internal_state::id() const noexcept {
        return id_;
    }

    std::size_t const_buffer::internal_state::size() const noexcept {
        return templ_ ? templ_->block_size() : 0;
    }
    
    std::size_t const_buffer::internal_state::offset() const noexcept {
        return offset_;
    }
    
    const_buffer::scope const_buffer::internal_state::binding_scope() const noexcept {
        return binding_scope_;
    }

    bool const_buffer::internal_state::is_compatible_with(const shader_ptr& shader) const noexcept {
        if ( !shader ) {
            return false;
        }
        return shader->state().get_block_info(binding_scope_).templ == templ_;
    }

    u32 const_buffer::internal_state::version() const noexcept {
        return version_;
    }
    
    const cbuffer_template_cptr& const_buffer::internal_state::block_template() const noexcept {
        return templ_;
    }

    float* const_buffer::internal_state::data() const noexcept {
        return content_.get();
    }

    void const_buffer::internal_state::on_content_update(u32 frame_id) const noexcept {
        E2D_ASSERT_MSG(frame_id > last_update_frame_id_,
            "only one update per frame are allowed for const_buffer,"
            "use another const_buffer if you need more updates");

        last_update_frame_id_ = frame_id;
        ++version_;
    }

    //
    // render_target::internal_state
    //

    render_target::internal_state::internal_state(
        debug& debug,
        opengl::gl_framebuffer_id id,
        const v2u& size,
        texture_ptr color,
        texture_ptr depth,
        opengl::gl_renderbuffer_id color_rb,
        opengl::gl_renderbuffer_id depth_rb)
    : debug_(debug)
    , id_(std::move(id))
    , size_(size)
    , color_(std::move(color))
    , depth_(std::move(depth))
    , color_rb_(std::move(color_rb))
    , depth_rb_(std::move(depth_rb)){
        E2D_ASSERT(!id_.empty());
    }

    debug& render_target::internal_state::dbg() const noexcept {
        return debug_;
    }

    const gl_framebuffer_id& render_target::internal_state::id() const noexcept {
        return id_;
    }

    const v2u& render_target::internal_state::size() const noexcept {
        return size_;
    }

    const texture_ptr& render_target::internal_state::color() const noexcept {
        return color_;
    }

    const texture_ptr& render_target::internal_state::depth() const noexcept {
        return depth_;
    }

    const gl_renderbuffer_id& render_target::internal_state::color_rb() const noexcept {
        return color_rb_;
    }

    const gl_renderbuffer_id& render_target::internal_state::depth_rb() const noexcept {
        return depth_rb_;
    }

    //
    // render::internal_state
    //
    
    render::internal_state::internal_state(debug& debug, window& window)
    : debug_(debug)
    , window_(window)
    , default_sp_(gl_program_id::current(debug))
    , default_fb_(gl_framebuffer_id::current(debug, GL_FRAMEBUFFER))
    {
        //textures_.fill({0, 0});

    #if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL
        if ( glewInit() != GLEW_OK ) {
            throw bad_render_operation();
        }
    #endif

        gl_trace_info(debug_);
        gl_trace_limits(debug_);
        gl_fill_device_caps(debug_, device_caps_, device_caps_ext_);

        gl_build_shader_headers(
            device_caps_, device_caps_ext_,
            vertex_shader_header_, fragment_shader_header_);

        GL_CHECK_CODE(debug_, glPixelStorei(GL_PACK_ALIGNMENT, 1));
        GL_CHECK_CODE(debug_, glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

        create_debug_output_();
        reset_states();
    }

    debug& render::internal_state::dbg() const noexcept {
        return debug_;
    }

    window& render::internal_state::wnd() const noexcept {
        return window_;
    }

    const render::device_caps& render::internal_state::device_capabilities() const noexcept {
        return device_caps_;
    }
    
    const opengl::gl_device_caps& render::internal_state::device_capabilities_ext() const noexcept {
        return device_caps_ext_;
    }
    
    render::statistics& render::internal_state::stats() noexcept {
        return current_stat_;
    }

    const render::statistics& render::internal_state::last_stats() const noexcept {
        return last_frame_stat_;
    }

    u32 render::internal_state::frame_id() const noexcept {
        return frame_id_;
    }
    
    bool render::internal_state::inside_render_pass() const noexcept {
        return inside_render_pass_;
    }
    
    const str& render::internal_state::vertex_shader_header() const noexcept {
        return vertex_shader_header_;
    }

    const str& render::internal_state::fragment_shader_header() const noexcept {
        return fragment_shader_header_;
    }

    void render::internal_state::on_present() noexcept {
        last_frame_stat_ = current_stat_;
        current_stat_ = {};
        ++frame_id_;
    }

    render::internal_state& render::internal_state::reset_states() noexcept {
        set_depth_state_(state_block_.depth());
        set_stencil_state_(state_block_.stencil());
        set_blending_state_(state_block_.blending());
        return *this;
    }

    render::internal_state& render::internal_state::set_states(const state_block& sb) noexcept {
        if ( sb.depth() != state_block_.depth() ) {
            set_depth_state_(sb.depth());
            state_block_.depth(sb.depth());
        }

        if ( sb.stencil() != state_block_.stencil() ) {
            set_stencil_state_(sb.stencil());
            state_block_.stencil(sb.stencil());
        }

        if ( sb.blending() != state_block_.blending() ) {
            set_blending_state_(sb.blending());
            state_block_.blending(sb.blending());
        }

        if ( sb.culling() != state_block_.culling() ) {
            set_culling_state_(sb.culling());
            state_block_.culling(sb.culling());
        }

        return *this;
    }

    render::internal_state& render::internal_state::reset_shader_program() noexcept {
        const gl_program_id& sp_id = shader_program_
            ? shader_program_->state().id()
            : default_sp_;
        GL_CHECK_CODE(debug_, glUseProgram(*sp_id));
        return *this;
    }

    render::internal_state& render::internal_state::set_shader_program(const shader_ptr& sp) noexcept {
        if ( sp == shader_program_ ) {
            return *this;
        }

        const gl_program_id& sp_id = sp
            ? sp->state().id()
            : default_sp_;
        GL_CHECK_CODE(debug_, glUseProgram(*sp_id));

        shader_program_ = sp;
        set_flag_inplace(dirty_flags_, dirty_flag_bits::pipeline);
        return *this;
    }
    
    void render::internal_state::begin_render_pass(const renderpass_desc& rp) noexcept {
        if ( inside_render_pass_ ) {
            end_render_pass();
        }
        inside_render_pass_ = true;
        set_render_target_(rp.target());

        GLenum clear_bits = 0;
        const render_target_ptr& rt = render_target_;
        bool has_color = !rt || rt->state().color() || !rt->state().color_rb().empty();
        bool has_depth = !rt || rt->state().depth() || !rt->state().depth_rb().empty();

        if ( has_color && rp.color_load_op() == attachment_load_op::clear ) {
            GL_CHECK_CODE(debug_, glClearColor(
                rp.color_clear_value().r,
                rp.color_clear_value().g,
                rp.color_clear_value().b,
                rp.color_clear_value().a));
            clear_bits |= GL_COLOR_BUFFER_BIT;
        }
        if ( has_depth && rp.depth_load_op() == attachment_load_op::clear ) {
            gl_clear_depth(debug_, rp.depth_clear_value());
            clear_bits |= GL_DEPTH_BUFFER_BIT;
        }
        if ( has_depth && rp.stencil_load_op() == attachment_load_op::clear ) {
            GL_CHECK_CODE(debug_, glClearStencil(rp.stencil_clear_value()));
            clear_bits |= GL_STENCIL_BUFFER_BIT;
        }
        if ( clear_bits ) {
            GL_CHECK_CODE(debug_, glClear(clear_bits));
        }
        
        render_area_ = rp.viewport();
        color_store_op_ = rp.color_store_op();
        depth_store_op_ = rp.depth_store_op();
        stencil_store_op_ = rp.stencil_store_op();

        gl_depth_range(dbg(), rp.depth_range().x, rp.depth_range().y);
        GL_CHECK_CODE(debug_, glViewport(
            math::numeric_cast<GLint>(rp.viewport().position.x),
            math::numeric_cast<GLint>(rp.viewport().position.y),
            math::numeric_cast<GLsizei>(rp.viewport().size.x),
            math::numeric_cast<GLsizei>(rp.viewport().size.y)));

        set_states(rp.states());
        render_pass_state_block_ = rp.states();

        stats().render_pass_count++;
    }

    void render::internal_state::end_render_pass() noexcept {
        E2D_ASSERT(inside_render_pass_);
        inside_render_pass_ = false;
        
        const bool is_default_fb = !render_target_;
        GLenum attachments[8];
        GLsizei count = 0;

        if ( color_store_op_ == attachment_store_op::discard ) {
            attachments[count++] = is_default_fb ? GL_COLOR : GL_COLOR_ATTACHMENT0;
        }
        // TOD: depth_stencil_attachment for depth_stencil texture
        if ( depth_store_op_ == attachment_store_op::discard ) {
            attachments[count++] = is_default_fb ? GL_DEPTH : GL_DEPTH_ATTACHMENT;
        }
        if ( stencil_store_op_ == attachment_store_op::discard ) {
            attachments[count++] = is_default_fb ? GL_STENCIL : GL_STENCIL_ATTACHMENT;
        }
        if ( count ) {
            if ( device_caps_ext_.framebuffer_invalidate_supported ) {
                GL_CHECK_CODE(debug_, glInvalidateSubFramebuffer(
                    GL_FRAMEBUFFER,
                    count,
                    attachments,
                    math::numeric_cast<GLint>(render_area_.position.x),
                    math::numeric_cast<GLint>(render_area_.position.y),
                    math::numeric_cast<GLsizei>(render_area_.size.x),
                    math::numeric_cast<GLsizei>(render_area_.size.y)));
            }
            else
            if ( device_caps_ext_.framebuffer_discard_supported ) {
                GL_CHECK_CODE(debug_, glScissor(
                    math::numeric_cast<GLint>(render_area_.position.x),
                    math::numeric_cast<GLint>(render_area_.position.y),
                    math::numeric_cast<GLsizei>(render_area_.size.x),
                    math::numeric_cast<GLsizei>(render_area_.size.y)));
                // TODO: enable scissor
                GL_CHECK_CODE(debug_, glDiscardFramebufferEXT(
                    GL_FRAMEBUFFER,
                    count,
                    attachments));
            }
        }
        
        // reset to default
        color_store_op_ = attachment_store_op::store;
        depth_store_op_ = attachment_store_op::discard;
        stencil_store_op_ = attachment_store_op::discard;

        set_render_target_(nullptr);

        // reset vertex attribs
        for ( size_t i = 0; i < render_cfg::max_attribute_count; ++i ) {
            GL_CHECK_CODE(debug_, glDisableVertexAttribArray(GLuint(i)));
        }
        //GL_CHECK_CODE(debug_, glBindBuffer(
        //    GL_ARRAY_BUFFER, 0));
        //GL_CHECK_CODE(debug_, glBindBuffer(
        //    GL_ELEMENT_ARRAY_BUFFER, 0));

        index_buffer_ = nullptr;
        vertex_buffers_ = {};
        enabled_attribs_ = {};
    }
    
    void render::internal_state::set_render_target_(const render_target_ptr& rt) noexcept {
        if ( rt == render_target_ ) {
            return;
        }

        const gl_framebuffer_id& rt_id = rt
            ? rt->state().id()
            : default_fb_;
        GL_CHECK_CODE(debug_, glBindFramebuffer(rt_id.target(), *rt_id));

        render_target_ = rt;
    }
    
    void render::internal_state::reset_render_target_() noexcept {
        const gl_framebuffer_id& rt_id = render_target_
            ? render_target_->state().id()
            : default_fb_;
        GL_CHECK_CODE(debug_, glBindFramebuffer(rt_id.target(), *rt_id));
    }
    
    void render::internal_state::bind_vertex_buffer(
            std::size_t index,
            const vertex_buffer_ptr& vbuffer,
            const vertex_attribs_ptr& attribs,
            std::size_t offset) noexcept
    {
        E2D_ASSERT(!vbuffer == !attribs);

        auto& curr = vertex_buffers_[index];
        if ( curr.buffer != vbuffer ||
             curr.attribs != attribs ||
             curr.offset != offset )
        {
            set_flag_inplace(dirty_flags_, dirty_flag_bits::vertex_attribs);
            curr.buffer = vbuffer;
            curr.attribs = attribs;
            curr.offset = offset;
        }
    }
    
    void render::internal_state::bind_const_buffer(
        const const_buffer_ptr& cbuffer) noexcept
    {
        if ( !cbuffer ) {
            return;
        }

        switch ( cbuffer->binding_scope() ) {
            case const_buffer::scope::render_pass:
                if ( cbuffers_[cb_pass_index] != cbuffer ) {
                    cbuffers_[cb_pass_index] = cbuffer;
                    set_flag_inplace(dirty_flags_, dirty_flag_bits::pass_cbuffer);
                }
                break;
            case const_buffer::scope::material:
                if ( cbuffers_[cb_material_index] != cbuffer ) {
                    cbuffers_[cb_material_index] = cbuffer;
                    set_flag_inplace(dirty_flags_, dirty_flag_bits::mtr_cbuffer);
                }
                break;
            case const_buffer::scope::draw_command:
                if ( cbuffers_[cb_command_index] != cbuffer ) {
                    cbuffers_[cb_command_index] = cbuffer;
                    set_flag_inplace(dirty_flags_, dirty_flag_bits::draw_cbuffer);
                }
                break;
        }
    }
    
    void render::internal_state::bind_index_buffer(
        const index_buffer_ptr& ibuffer) noexcept
    {
        index_buffer_ = ibuffer;
    }
    
    void render::internal_state::bind_textures(
        sampler_block::scope scope,
        const sampler_block& samplers) noexcept
    {
        switch ( scope ) {
            case sampler_block::scope::render_pass:
                samplers_[cb_pass_index] = samplers;
                set_flag_inplace(dirty_flags_, dirty_flag_bits::pass_textures);
                break;
            case sampler_block::scope::material:
                samplers_[cb_material_index] = samplers;
                set_flag_inplace(dirty_flags_, dirty_flag_bits::mtr_textures);
                break;
        }
    }
    
    void render::internal_state::bind_vertex_attributes_() noexcept {
        if ( !check_flag_and_reset(dirty_flags_, dirty_flag_bits::vertex_attribs) ) {
            return;
        }
        enabled_attribs_t new_attribs;
        for ( auto& vb : vertex_buffers_ ) {
            if ( !vb.buffer || !vb.attribs ) {
                continue;
            }
            const gl_buffer_id& buf_id = vb.buffer->state().id();
            GL_CHECK_CODE(debug_, glBindBuffer(buf_id.target(), *buf_id));

            const vertex_declaration& decl = vb.attribs->decl();
            for ( size_t i = 0; i < decl.attribute_count(); ++i ) {
                const auto& vai = decl.attribute(i);
                const size_t off = vb.offset + vai.stride;
                shader_program_->state().with_attribute_location(vai.name,
                    [this, &vai, off, &decl, &new_attribs](const auto& ai) noexcept{
                        const GLuint rows = math::numeric_cast<GLuint>(vai.rows);
                        for ( GLuint row = 0; row < rows; ++row ) {
                            GLuint index = math::numeric_cast<GLuint>(ai.index) + row;
                            new_attribs[index] = true;
                            GL_CHECK_CODE(debug_, glVertexAttribPointer(
                                index,
                                math::numeric_cast<GLint>(vai.columns),
                                convert_attribute_type(vai.type),
                                vai.normalized ? GL_TRUE : GL_FALSE,
                                math::numeric_cast<GLsizei>(decl.bytes_per_vertex()),
                                reinterpret_cast<const GLvoid*>(off + row * vai.row_size())));
                        }
                    });
            }
        }
        for ( size_t i = 0; i < enabled_attribs_.size(); ++i ) {
            if ( enabled_attribs_[i] == new_attribs[i] ) {
                continue;
            }
            if ( new_attribs[i] ) {
                GL_CHECK_CODE(debug_, glEnableVertexAttribArray(GLuint(i)));
            } else {
                GL_CHECK_CODE(debug_, glDisableVertexAttribArray(GLuint(i)));
            }
        }
        enabled_attribs_ = new_attribs;
        
        //GL_CHECK_CODE(debug_, glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    void render::internal_state::bind_cbuffers_() noexcept {
        auto& prog = shader_program_->state();
        if ( check_flag_and_reset(dirty_flags_, dirty_flag_bits::pass_cbuffer) ) {
            prog.set_constants(const_buffer::scope::render_pass, cbuffers_[cb_pass_index]);
        }
        if ( check_flag_and_reset(dirty_flags_, dirty_flag_bits::mtr_cbuffer) ) {
            prog.set_constants(const_buffer::scope::material, cbuffers_[cb_material_index]);
        }
        if ( check_flag_and_reset(dirty_flags_, dirty_flag_bits::draw_cbuffer) ) {
            prog.set_constants(const_buffer::scope::draw_command, cbuffers_[cb_command_index]);
        }
        if ( device_caps_ext_.uniform_buffer_supported ) {
            bind_cbuffer_(cb_pass_index, cbuffers_[cb_pass_index]);
            bind_cbuffer_(cb_material_index, cbuffers_[cb_material_index]);
            bind_cbuffer_(cb_command_index, cbuffers_[cb_command_index]);
        }
    }
    
    void render::internal_state::bind_cbuffer_(u32 index, const const_buffer_ptr& cbuffer) noexcept {
        if ( cbuffer ) {
            auto& cb = cbuffer->state();
            GL_CHECK_CODE(debug_, glBindBufferRange(
                GL_UNIFORM_BUFFER,
                index,
                *cb.id(),
                math::numeric_cast<GLintptr>(cb.offset()),
                math::numeric_cast<GLsizeiptr>(cb.size())));
        }
    }

    void render::internal_state::bind_textures_() noexcept {
        if ( check_flag_and_reset(dirty_flags_, dirty_flag_bits::pass_textures) ) {
            bind_sampler_block_(samplers_[cb_pass_index]);
        }
        if ( check_flag_and_reset(dirty_flags_, dirty_flag_bits::mtr_textures) ) {
            bind_sampler_block_(samplers_[cb_material_index]);
        }
    }
    
    void render::internal_state::bind_sampler_block_(const sampler_block& block) noexcept {
        for ( size_t i = 0; i < block.count(); ++i ) {
            const sampler_state& sampler = block.sampler(i);
            shader_program_->state().with_sampler_location(block.name(i),
                [this, &sampler](const auto& info) noexcept{
                    GL_CHECK_CODE(debug_, glActiveTexture(
                        GL_TEXTURE0 + info.unit));
                    if ( sampler.texture() ) {
                        const gl_texture_id& texture_id = sampler.texture()->state().id();
                        E2D_ASSERT(texture_id.target() == convert_uniform_type_to_texture_target(info.type));
                        GL_CHECK_CODE(debug_, glBindTexture(
                            texture_id.target(), *texture_id));
                        GL_CHECK_CODE(debug_, glTexParameteri(
                            texture_id.target(),
                            GL_TEXTURE_WRAP_S,
                            convert_sampler_wrap(sampler.s_wrap())));
                        GL_CHECK_CODE(debug_, glTexParameteri(
                            texture_id.target(),
                            GL_TEXTURE_WRAP_T,
                            convert_sampler_wrap(sampler.t_wrap())));
                        GL_CHECK_CODE(debug_, glTexParameteri(
                            texture_id.target(),
                            GL_TEXTURE_MIN_FILTER,
                            convert_sampler_filter(sampler.min_filter())));
                        GL_CHECK_CODE(debug_, glTexParameteri(
                            texture_id.target(),
                            GL_TEXTURE_MAG_FILTER,
                            convert_sampler_filter(sampler.mag_filter())));
                    } else {
                        E2D_ASSERT_MSG(false, "TODO: error?");
                        GL_CHECK_CODE(debug_, glBindTexture(GL_TEXTURE_2D, 0));
                    }
                });
        }
    }

    void render::internal_state::commit_changes_() noexcept {
        E2D_ASSERT(shader_program_);

        if ( dirty_flags_ == dirty_flag_bits::none ) {
            return;
        }

        bind_vertex_attributes_();
        bind_cbuffers_();
        bind_textures_();

        E2D_ASSERT(dirty_flags_ == dirty_flag_bits::none);
    }

    void render::internal_state::draw(topology topo, u32 first, u32 count) noexcept {
        commit_changes_();

        GL_CHECK_CODE(debug_, glDrawArrays(
            convert_topology(topo),
            first,
            count));
        
        stats().draw_calls++;
    }

    void render::internal_state::draw_indexed(topology topo, u32 count, size_t offset) noexcept {
        E2D_ASSERT(index_buffer_);
        commit_changes_();

        GL_CHECK_CODE(debug_, glBindBuffer(
            GL_ELEMENT_ARRAY_BUFFER,
            *index_buffer_->state().id()));

        GL_CHECK_CODE(debug_, glDrawElements(
            convert_topology(topo),
            count,
            convert_index_type(index_buffer_->decl().type()),
            reinterpret_cast<void*>(offset)));

        stats().draw_calls++;
    }
    
    void render::internal_state::set_blending_state(const std::optional<blending_state>& state) noexcept {
        const auto& new_state = state.has_value() ? *state : render_pass_state_block_.blending();
        if ( new_state != state_block_.blending() ) {
            set_blending_state_(new_state);
            state_block_.blending(new_state);
        }
    }

    void render::internal_state::set_culling_state(const std::optional<culling_state>& state) noexcept {
        const auto& new_state = state.has_value() ? *state : render_pass_state_block_.culling();
        if ( new_state != state_block_.culling() ) {
            set_culling_state_(new_state);
            state_block_.culling(new_state);
        }
    }

    void render::internal_state::set_depth_state(const std::optional<depth_state>& state) noexcept {
        const auto& new_state = state.has_value() ? *state : render_pass_state_block_.depth();
        if ( new_state != state_block_.depth() ) {
            set_depth_state_(new_state);
            state_block_.depth(new_state);
        }
    }

    void render::internal_state::set_stencil_state(const std::optional<stencil_state>& state) noexcept {
        const auto& new_state = state.has_value() ? *state : render_pass_state_block_.stencil();
        if ( new_state != state_block_.stencil() ) {
            set_stencil_state_(new_state);
            state_block_.stencil(new_state);
        }
    }
    
    void render::internal_state::set_scissor(bool enable, const b2u& rect) noexcept {
        if ( enable ) {
            GL_CHECK_CODE(debug_, glScissor(
                math::numeric_cast<GLint>(rect.position.x),
                math::numeric_cast<GLint>(rect.position.y),
                math::numeric_cast<GLint>(rect.size.x),
                math::numeric_cast<GLint>(rect.size.y)));
            GL_CHECK_CODE(debug_, glEnable(GL_SCISSOR_TEST));
        } else {
            GL_CHECK_CODE(debug_, glDisable(GL_SCISSOR_TEST));
        }
    }

    void render::internal_state::insert_message(str_view msg) noexcept {
        if ( !device_caps_ext_.debug_output_supported || msg.empty() ) {
            return;
        }
        GL_CHECK_CODE(debug_, glDebugMessageInsert(
            GL_DEBUG_SOURCE_APPLICATION,
            GL_DEBUG_TYPE_OTHER,
            0,
            GL_DEBUG_SEVERITY_LOW,
            math::numeric_cast<GLsizei>(msg.length()),
            msg.data()));
    }

    void render::internal_state::set_depth_state_(const depth_state& ds) noexcept {
        GL_CHECK_CODE(debug_, glDepthMask(
            ds.write() ? GL_TRUE : GL_FALSE));

        if ( ds.test() ) {
            GL_CHECK_CODE(debug_, glEnable(GL_DEPTH_TEST));
            GL_CHECK_CODE(debug_, glDepthFunc(
                convert_compare_func(ds.func())));
        } else {
            GL_CHECK_CODE(debug_, glDisable(GL_DEPTH_TEST));
        }
    }

    void render::internal_state::set_stencil_state_(const stencil_state& ss) noexcept {
        if ( ss.test() ) {
            GL_CHECK_CODE(debug_, glEnable(GL_STENCIL_TEST));
            GL_CHECK_CODE(debug_, glStencilMask(
                math::numeric_cast<GLuint>(ss.write())));
            GL_CHECK_CODE(debug_, glStencilFunc(
                convert_compare_func(ss.func()),
                math::numeric_cast<GLint>(ss.ref()),
                math::numeric_cast<GLuint>(ss.mask())));
            GL_CHECK_CODE(debug_, glStencilOp(
                convert_stencil_op(ss.sfail()),
                convert_stencil_op(ss.zfail()),
                convert_stencil_op(ss.pass())));
        } else {
            GL_CHECK_CODE(debug_, glDisable(GL_STENCIL_TEST));
        }
    }

    void render::internal_state::set_culling_state_(const culling_state& cs) noexcept {
        if ( cs.enabled() ) {
            GL_CHECK_CODE(debug_, glEnable(GL_CULL_FACE));
            GL_CHECK_CODE(debug_, glCullFace(
                convert_culling_face(cs.face())));
        } else {
            GL_CHECK_CODE(debug_, glDisable(GL_CULL_FACE));
        }
        GL_CHECK_CODE(debug_, glFrontFace(
            convert_culling_mode(cs.mode())));
    }

    void render::internal_state::set_blending_state_(const blending_state& bs) noexcept {
        if ( bs.enabled() ) {
            GL_CHECK_CODE(debug_, glEnable(GL_BLEND));
            GL_CHECK_CODE(debug_, glBlendFuncSeparate(
                convert_blending_factor(bs.src_rgb_factor()),
                convert_blending_factor(bs.dst_rgb_factor()),
                convert_blending_factor(bs.src_alpha_factor()),
                convert_blending_factor(bs.dst_alpha_factor())));
            GL_CHECK_CODE(debug_, glBlendEquationSeparate(
                convert_blending_equation(bs.rgb_equation()),
                convert_blending_equation(bs.alpha_equation())));
        } else {
            GL_CHECK_CODE(debug_, glDisable(GL_BLEND));
        }
        GL_CHECK_CODE(debug_, glColorMask(
            (utils::enum_to_underlying(bs.color_mask()) & utils::enum_to_underlying(blending_color_mask::r)) != 0,
            (utils::enum_to_underlying(bs.color_mask()) & utils::enum_to_underlying(blending_color_mask::g)) != 0,
            (utils::enum_to_underlying(bs.color_mask()) & utils::enum_to_underlying(blending_color_mask::b)) != 0,
            (utils::enum_to_underlying(bs.color_mask()) & utils::enum_to_underlying(blending_color_mask::a)) != 0));
    }
    
    vertex_attribs_ptr render::internal_state::create_vertex_attribs(const vertex_declaration& decl) {
        auto va = std::make_shared<vertex_attribs>(
            std::make_unique<vertex_attribs::internal_state>(
                debug_,
                decl));
        return *vertex_attrib_cache_.insert(va).first;
    }

    void render::internal_state::create_debug_output_() noexcept {
        if ( !device_caps_ext_.debug_output_supported ) {
            return;
        }

        GL_CHECK_CODE(debug_, glDebugMessageCallback(debug_output_callback_, this));

        // disable notifications
        GL_CHECK_CODE(debug_, glDebugMessageControl(
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DEBUG_SEVERITY_NOTIFICATION,
            0,
            nullptr,
            GL_FALSE));
    }
    
    void render::internal_state::debug_output_callback_(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam)
    {
        E2D_UNUSED(id);

        if ( !userParam ) {
            return;
        }

        str msg;
        switch ( severity ) {
            case GL_DEBUG_SEVERITY_HIGH : msg += "[High]"; break;
            case GL_DEBUG_SEVERITY_MEDIUM : msg += "[Medium]"; break;
            case GL_DEBUG_SEVERITY_LOW : msg += "[Low]"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION : msg += "[Notification]"; break;
        }

        msg += " src: ";
        switch ( source ) {
            case GL_DEBUG_SOURCE_API : msg += "OpenGL"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM : msg += "OS"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER : msg += "GL_Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY : msg += "Third_Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION : msg += "Application"; break;
            case GL_DEBUG_SOURCE_OTHER :
            default : msg += "Other"; break;
        }

        msg += ", type: ";
        switch ( type ) {
            case GL_DEBUG_TYPE_ERROR : msg += "Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : msg += "Deprecated"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR : msg += "Undefined_Behavior"; break;
            case GL_DEBUG_TYPE_PORTABILITY : msg += "Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE : msg += "Performance"; break;
            case GL_DEBUG_TYPE_MARKER : msg += "Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP : msg += "Push_Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP : msg += "Pop_Group"; break;
            case GL_DEBUG_TYPE_OTHER :
            default : msg += "Other"; break;
        }

        msg += "\n";

        if ( message && length ) {
            msg += str_view(message, length);
        }

        static_cast<const internal_state*>(userParam)->debug_.trace(msg);
    }
}

#endif
#endif

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_opengl_base.hpp"

#if defined(E2D_RENDER_MODE)
#if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL || E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES

namespace
{
    using namespace e2d;
    using namespace e2d::opengl;

    void gl_get_string(GLenum name, const char** str) noexcept {
        E2D_ASSERT(str);
        *str = reinterpret_cast<const char*>(glGetString(name));
    }

    void gl_create_shader(GLenum type, GLuint* id) noexcept {
        E2D_ASSERT(id);
        *id = glCreateShader(type);
    }

    void gl_create_program(GLuint* id) noexcept {
        E2D_ASSERT(id);
        *id = glCreateProgram();
    }

    void gl_get_uniform_location(GLuint program, const GLchar* name, GLint* loc) noexcept {
        E2D_ASSERT(loc);
        *loc = glGetUniformLocation(program, name);
    }

    void gl_get_attribute_location(GLuint program, const GLchar* name, GLint* loc) noexcept {
        E2D_ASSERT(loc);
        *loc = glGetAttribLocation(program, name);
    }

    void gl_check_framebuffer_status(GLenum target, GLenum* res) noexcept {
        E2D_ASSERT(res);
        *res = glCheckFramebufferStatus(target);
    }

    bool process_shader_compilation_result(debug& debug, GLuint shader) noexcept {
        E2D_ASSERT(glIsShader(shader));
        GLint success = GL_FALSE;
        GL_CHECK_CODE(debug, glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        GLint log_len = 0;
        GL_CHECK_CODE(debug, glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len));
        if ( log_len > 0 ) {
            GLchar* log_buffer = static_cast<GLchar*>(E2D_ALLOCA(
                sizeof(GLchar) * math::numeric_cast<std::size_t>(log_len)));
            GL_CHECK_CODE(debug, glGetShaderInfoLog(
                shader, log_len, nullptr, log_buffer));
            debug.log(success ? debug::level::warning : debug::level::error,
                "RENDER: shader compilation info:\n--> %0", log_buffer);
        }
        return success == GL_TRUE;
    }

    bool process_program_linking_result(debug& debug, GLuint program) noexcept {
        E2D_ASSERT(glIsProgram(program));
        GLint success = GL_FALSE;
        GL_CHECK_CODE(debug, glGetProgramiv(program, GL_LINK_STATUS, &success));
        GLint log_len = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len));
        if ( log_len > 0 ) {
            GLchar* log_buffer = static_cast<GLchar*>(E2D_ALLOCA(
                sizeof(GLchar) * math::numeric_cast<std::size_t>(log_len)));
            GL_CHECK_CODE(debug, glGetProgramInfoLog(
                program, log_len, nullptr, log_buffer));
            debug.log(success ? debug::level::warning : debug::level::error,
                "RENDER: program linking info:\n--> %0", log_buffer);
        }
        return success == GL_TRUE;
    }

    bool process_program_validation_result(debug& debug, GLuint program) noexcept {
        E2D_ASSERT(glIsProgram(program));
        GL_CHECK_CODE(debug, glValidateProgram(program));
        GLint success = GL_FALSE;
        GL_CHECK_CODE(debug, glGetProgramiv(program, GL_VALIDATE_STATUS, &success));
        GLint log_len = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len));
        if ( log_len > 0 ) {
            GLchar* log_buffer = static_cast<GLchar*>(E2D_ALLOCA(
                sizeof(GLchar) * math::numeric_cast<std::size_t>(log_len)));
            GL_CHECK_CODE(debug, glGetProgramInfoLog(
                program, log_len, nullptr, log_buffer));
            debug.log(success ? debug::level::warning : debug::level::error,
                "RENDER: program validation info:\n--> %0", log_buffer);
        }
        return success == GL_TRUE;
    }

    template < typename... Ext >
    bool gl_has_any_extension(debug& debug, Ext... required) noexcept {
        const GLubyte* all_extensions = nullptr;
        GL_CHECK_CODE(debug, all_extensions = glGetString(GL_EXTENSIONS));
        if ( !all_extensions ) {
            return false;
        }
        str_view extensions(reinterpret_cast<const char*>(all_extensions));
        while ( !extensions.empty() ) {
            const auto sep_pos = extensions.find(' ');
            const str_view current = sep_pos != str_view::npos
                ? extensions.substr(0, sep_pos)
                : extensions;
            for ( const auto& req : {required...} ) {
                if ( req == current ) {
                    return true;
                }
            }
            extensions = sep_pos != str_view::npos
                ? extensions.substr(sep_pos + 1)
                : str_view();
        }
        return false;
    }

    enum class gl_version : u32 {
        gl_bit_ = 1 << 28,
        gles_bit_ = 2 << 28,

        gl_210 = 210 | gl_bit_,
        gl_300 = 300 | gl_bit_,
        gl_410 = 410 | gl_bit_,
        gles_200 = 200 | gles_bit_,
        gles_300 = 300 | gles_bit_
    };

    bool operator>=(gl_version lhs, gl_version rhs) noexcept {
        constexpr u32 mask = u32(gl_version::gl_bit_)
                           | u32(gl_version::gles_bit_);
        return (u32(lhs) & mask) == (u32(rhs) & mask)
            && (u32(lhs) & ~mask) >= (u32(rhs) & ~mask);
    }

    gl_version gl_get_version(debug& debug) noexcept {
        const GLubyte* ver = nullptr;
        GL_CHECK_CODE(debug, ver = glGetString(GL_VERSION));

        GLint major = 2, minor = 1;
        if ( ver ) {
            const str_view ver_str = str_view(reinterpret_cast<const char*>(ver));
            const std::size_t dot = ver_str.find('.');
            if ( dot > 0 && dot + 1 < ver_str.size() ) {
                major = ver[dot - 1] - '0';
                minor = ver[dot + 1] - '0';
            }
        }

    #if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL
        const u32 ver_bit = u32(gl_version::gl_bit_);
    #elif E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES
        const u32 ver_bit = u32(gl_version::gles_bit_);
    #else
    #   error unknown render mode
    #endif

        return gl_version(ver_bit | (major * 100 + minor * 10));
    }
}

namespace e2d::opengl
{
    //
    // gl_buffer_id
    //

    gl_buffer_id::gl_buffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , target_(target)
    , owned_(owned){
        E2D_ASSERT(!id || glIsBuffer(id));
    }

    gl_buffer_id gl_buffer_id::create(debug& debug, GLenum target) noexcept {
        E2D_ASSERT(
            target == GL_ARRAY_BUFFER ||
            target == GL_ELEMENT_ARRAY_BUFFER);
        GLuint id = 0;
        GL_CHECK_CODE(debug, glGenBuffers(1, &id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate buffer id");
            return gl_buffer_id(debug);
        }
        with_gl_bind_buffer(debug, target, id, []() noexcept {});
        return gl_buffer_id(debug, id, target, true);
    }

    gl_buffer_id gl_buffer_id::current(debug& debug, GLenum target) noexcept {
        GLint id = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(gl_target_to_get_target(target), &id));
        return gl_buffer_id(debug, math::numeric_cast<GLuint>(id), target, false);
    }

    gl_buffer_id::gl_buffer_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_buffer_id::~gl_buffer_id() noexcept {
        reset();
    }

    gl_buffer_id::gl_buffer_id(gl_buffer_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_buffer_id& gl_buffer_id::operator=(gl_buffer_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_buffer_id& gl_buffer_id::swap(gl_buffer_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(target_, other.target_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_buffer_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteBuffers(1, &id_));
        }
        id_ = 0;
        target_ = 0;
        owned_ = false;
    }

    bool gl_buffer_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_buffer_id::operator*() const noexcept {
        return id_;
    }

    GLenum gl_buffer_id::target() const noexcept {
        return target_;
    }

    //
    // gl_shader_id
    //

    gl_shader_id::gl_shader_id(debug& debug, GLuint id, GLenum type, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , type_(type)
    , owned_(owned){
        E2D_ASSERT(!id || glIsShader(id));
    }

    gl_shader_id gl_shader_id::create(debug& debug, GLenum type) noexcept {
        E2D_ASSERT(
            type == GL_VERTEX_SHADER ||
            type == GL_FRAGMENT_SHADER);
        GLuint id = 0;
        GL_CHECK_CODE(debug, gl_create_shader(type, &id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate shader id");
            return gl_shader_id(debug);
        }
        return gl_shader_id(debug, id, type, true);
    }

    gl_shader_id::gl_shader_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_shader_id::~gl_shader_id() noexcept {
        reset();
    }

    gl_shader_id::gl_shader_id(gl_shader_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_shader_id& gl_shader_id::operator=(gl_shader_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_shader_id& gl_shader_id::swap(gl_shader_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(type_, other.type_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_shader_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteShader(id_));
        }
        id_ = 0;
        type_ = 0;
        owned_ = false;
    }

    bool gl_shader_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_shader_id::operator*() const noexcept {
        return id_;
    }

    GLenum gl_shader_id::type() const noexcept {
        return type_;
    }

    //
    // gl_program_id
    //

    gl_program_id::gl_program_id(debug& debug, GLuint id, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , owned_(owned){
        E2D_ASSERT(!id || glIsProgram(id));
    }

    gl_program_id gl_program_id::create(debug& debug) noexcept {
        GLuint id = 0;
        GL_CHECK_CODE(debug, gl_create_program(&id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate program id");
            return gl_program_id(debug);
        }
        return gl_program_id(debug, id, true);
    }

    gl_program_id gl_program_id::current(debug& debug) noexcept {
        GLint id = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(GL_CURRENT_PROGRAM, &id));
        return gl_program_id(debug, math::numeric_cast<GLuint>(id), false);
    }

    gl_program_id::gl_program_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_program_id::~gl_program_id() noexcept {
        reset();
    }

    gl_program_id::gl_program_id(gl_program_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_program_id& gl_program_id::operator=(gl_program_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_program_id& gl_program_id::swap(gl_program_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_program_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteProgram(id_));
        }
        id_ = 0;
        owned_ = false;
    }

    bool gl_program_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_program_id::operator*() const noexcept {
        return id_;
    }

    //
    // gl_texture_id
    //

    gl_texture_id::gl_texture_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , target_(target)
    , owned_(owned){
        E2D_ASSERT(!id || glIsTexture(id));
    }

    gl_texture_id gl_texture_id::create(debug& debug, GLenum target) noexcept {
        E2D_ASSERT(
            target == GL_TEXTURE_2D ||
            target == GL_TEXTURE_CUBE_MAP);
        GLuint id = 0;
        GL_CHECK_CODE(debug, glGenTextures(1, &id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate texture id");
            return gl_texture_id(debug);
        }
        with_gl_bind_texture(debug, target, id, []() noexcept {});
        return gl_texture_id(debug, id, target, true);
    }

    gl_texture_id gl_texture_id::current(debug& debug, GLenum target) noexcept {
        GLint id = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(gl_target_to_get_target(target), &id));
        return gl_texture_id(debug, math::numeric_cast<GLuint>(id), target, false);
    }

    gl_texture_id::gl_texture_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_texture_id::~gl_texture_id() noexcept {
        reset();
    }

    gl_texture_id::gl_texture_id(gl_texture_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_texture_id& gl_texture_id::operator=(gl_texture_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_texture_id& gl_texture_id::swap(gl_texture_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(target_, other.target_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_texture_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteTextures(1, &id_));
        }
        id_ = 0;
        target_ = 0;
        owned_ = false;
    }

    bool gl_texture_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_texture_id::operator*() const noexcept {
        return id_;
    }

    GLenum gl_texture_id::target() const noexcept {
        return target_;
    }

    //
    // gl_framebuffer_id
    //

    gl_framebuffer_id::gl_framebuffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , target_(target)
    , owned_(owned){
        E2D_ASSERT(!id || glIsFramebuffer(id));
    }

    gl_framebuffer_id gl_framebuffer_id::create(debug& debug, GLenum target) noexcept {
        E2D_ASSERT(
            target == GL_FRAMEBUFFER);
        GLuint id = 0;
        GL_CHECK_CODE(debug, glGenFramebuffers(1, &id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate framebuffer id");
            return gl_framebuffer_id(debug);
        }
        with_gl_bind_framebuffer(debug, target, id, []() noexcept {});
        return gl_framebuffer_id(debug, id, target, true);
    }

    gl_framebuffer_id gl_framebuffer_id::current(debug& debug, GLenum target) noexcept {
        GLint id = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(gl_target_to_get_target(target), &id));
        return gl_framebuffer_id(debug, math::numeric_cast<GLuint>(id), target, false);
    }

    gl_framebuffer_id::gl_framebuffer_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_framebuffer_id::~gl_framebuffer_id() noexcept {
        reset();
    }

    gl_framebuffer_id::gl_framebuffer_id(gl_framebuffer_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_framebuffer_id& gl_framebuffer_id::operator=(gl_framebuffer_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_framebuffer_id& gl_framebuffer_id::swap(gl_framebuffer_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(target_, other.target_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_framebuffer_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteFramebuffers(1, &id_));
        }
        id_ = 0;
        target_ = 0;
        owned_ = false;
    }

    bool gl_framebuffer_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_framebuffer_id::operator*() const noexcept {
        return id_;
    }

    GLenum gl_framebuffer_id::target() const noexcept {
        return target_;
    }

    //
    // gl_renderbuffer_id
    //

    gl_renderbuffer_id::gl_renderbuffer_id(debug& debug, GLuint id, GLenum target, bool owned) noexcept
    : debug_(debug)
    , id_(id)
    , target_(target)
    , owned_(owned){
        E2D_ASSERT(!id || glIsRenderbuffer(id));
    }

    gl_renderbuffer_id gl_renderbuffer_id::create(debug& debug, GLenum target) noexcept {
        E2D_ASSERT(
            target == GL_RENDERBUFFER);
        GLuint id = 0;
        GL_CHECK_CODE(debug, glGenRenderbuffers(1, &id));
        if ( !id ) {
            debug.error("RENDER: Failed to generate renderbuffer id");
            return gl_renderbuffer_id(debug);
        }
        with_gl_bind_renderbuffer(debug, target, id, []() noexcept {});
        return gl_renderbuffer_id(debug, id, target, true);
    }

    gl_renderbuffer_id gl_renderbuffer_id::current(debug& debug, GLenum target) noexcept {
        GLint id = 0;
        GL_CHECK_CODE(debug, glGetIntegerv(gl_target_to_get_target(target), &id));
        return gl_renderbuffer_id(debug, math::numeric_cast<GLuint>(id), target, false);
    }

    gl_renderbuffer_id::gl_renderbuffer_id(debug& debug) noexcept
    : debug_(debug) {}

    gl_renderbuffer_id::~gl_renderbuffer_id() noexcept {
        reset();
    }

    gl_renderbuffer_id::gl_renderbuffer_id(gl_renderbuffer_id&& other) noexcept
    : debug_(other.debug_) {
        swap(other);
    }

    gl_renderbuffer_id& gl_renderbuffer_id::operator=(gl_renderbuffer_id&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.reset();
        }
        return *this;
    }

    gl_renderbuffer_id& gl_renderbuffer_id::swap(gl_renderbuffer_id& other) noexcept {
        if ( this != &other ) {
            using std::swap;
            swap(id_, other.id_);
            swap(target_, other.target_);
            swap(owned_, other.owned_);
        }
        return *this;
    }

    void gl_renderbuffer_id::reset() noexcept {
        if ( id_ && owned_ ) {
            GL_CHECK_CODE(debug_, glDeleteRenderbuffers(1, &id_));
        }
        id_ = 0;
        target_ = 0;
        owned_ = false;
    }

    bool gl_renderbuffer_id::empty() const noexcept {
        return id_ == 0;
    }

    GLuint gl_renderbuffer_id::operator*() const noexcept {
        return id_;
    }

    GLenum gl_renderbuffer_id::target() const noexcept {
        return target_;
    }

    //
    // operators
    //

    bool operator==(const gl_buffer_id& l, const gl_buffer_id& r) noexcept {
        return l.target() == r.target()
            && l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator==(const gl_shader_id& l, const gl_shader_id& r) noexcept {
        return l.type() == r.type()
            && l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator==(const gl_program_id& l, const gl_program_id& r) noexcept {
        return l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator==(const gl_texture_id& l, const gl_texture_id& r) noexcept {
        return l.target() == r.target()
            && l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator==(const gl_framebuffer_id& l, const gl_framebuffer_id& r) noexcept {
        return l.target() == r.target()
            && l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator==(const gl_renderbuffer_id& l, const gl_renderbuffer_id& r) noexcept {
        return l.target() == r.target()
            && l.empty() == r.empty()
            && (l.empty() || *l == *r);
    }

    bool operator!=(const gl_buffer_id& l, const gl_buffer_id& r) noexcept {
        return !(l == r);
    }

    bool operator!=(const gl_shader_id& l, const gl_shader_id& r) noexcept {
        return !(l == r);
    }

    bool operator!=(const gl_program_id& l, const gl_program_id& r) noexcept {
        return !(l == r);
    }

    bool operator!=(const gl_texture_id& l, const gl_texture_id& r) noexcept {
        return !(l == r);
    }

    bool operator!=(const gl_framebuffer_id& l, const gl_framebuffer_id& r) noexcept {
        return !(l == r);
    }

    bool operator!=(const gl_renderbuffer_id& l, const gl_renderbuffer_id& r) noexcept {
        return !(l == r);
    }
}

namespace e2d::opengl
{
    const char* uniform_type_to_cstr(uniform_type ut) noexcept {
        #define DEFINE_CASE(x) case uniform_type::x: return #x
        switch ( ut ) {
            DEFINE_CASE(signed_integer);
            DEFINE_CASE(floating_point);
            DEFINE_CASE(v2i);
            DEFINE_CASE(v3i);
            DEFINE_CASE(v4i);
            DEFINE_CASE(v2f);
            DEFINE_CASE(v3f);
            DEFINE_CASE(v4f);
            DEFINE_CASE(m2f);
            DEFINE_CASE(m3f);
            DEFINE_CASE(m4f);
            DEFINE_CASE(sampler_2d);
            DEFINE_CASE(sampler_cube);
            DEFINE_CASE(unknown);
            default:
                E2D_ASSERT_MSG(false, "unexpected uniform type");
                return "";
        }
        #undef DEFINE_CASE
    }

    const char* attribute_type_to_cstr(attribute_type at) noexcept {
        #define DEFINE_CASE(x) case attribute_type::x: return #x
        switch ( at ) {
            DEFINE_CASE(floating_point);
            DEFINE_CASE(v2f);
            DEFINE_CASE(v3f);
            DEFINE_CASE(v4f);
            DEFINE_CASE(m2f);
            DEFINE_CASE(m3f);
            DEFINE_CASE(m4f);
            DEFINE_CASE(unknown);
            default:
                E2D_ASSERT_MSG(false, "unexpected attribute type");
                return "";
        }
        #undef DEFINE_CASE
    }
}

namespace e2d::opengl
{
    GLenum convert_image_data_format_to_external_format(image_data_format f) noexcept {
        #define DEFINE_CASE(x,y) case image_data_format::x: return y
        switch ( f ) {
            DEFINE_CASE(a8, GL_ALPHA);
            DEFINE_CASE(l8, GL_LUMINANCE);
            DEFINE_CASE(la8, GL_LUMINANCE_ALPHA);
            DEFINE_CASE(rgb8, GL_RGB);
            DEFINE_CASE(rgba8, GL_RGBA);
            default:
                E2D_ASSERT_MSG(false, "unexpected image data format");
                return GL_RGBA;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_image_data_format_to_external_data_type(image_data_format f) noexcept {
        #define DEFINE_CASE(x,y) case image_data_format::x: return y
        switch ( f ) {
            DEFINE_CASE(a8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(l8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(la8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(rgb8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(rgba8, GL_UNSIGNED_BYTE);
            default:
                E2D_ASSERT_MSG(false, "unexpected image data format");
                return GL_UNSIGNED_BYTE;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_pixel_type_to_external_format(pixel_declaration::pixel_type f) noexcept {
        #define DEFINE_CASE(x,y) case pixel_declaration::pixel_type::x: return y
        switch ( f ) {
            DEFINE_CASE(depth16, GL_DEPTH_COMPONENT);
            DEFINE_CASE(depth24, GL_DEPTH_COMPONENT);
            DEFINE_CASE(depth24_stencil8, GL_DEPTH_STENCIL);
            DEFINE_CASE(a8, GL_ALPHA);
            DEFINE_CASE(l8, GL_LUMINANCE);
            DEFINE_CASE(la8, GL_LUMINANCE_ALPHA);
            DEFINE_CASE(rgb8, GL_RGB);
            DEFINE_CASE(rgba8, GL_RGBA);
            default:
                E2D_ASSERT_MSG(false, "unexpected pixel type");
                return GL_RGBA;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_pixel_type_to_external_data_type(pixel_declaration::pixel_type f) noexcept {
        #define DEFINE_CASE(x,y) case pixel_declaration::pixel_type::x: return y
        switch ( f ) {
            DEFINE_CASE(depth16, GL_UNSIGNED_SHORT);
            DEFINE_CASE(depth24, GL_UNSIGNED_INT);
            DEFINE_CASE(depth24_stencil8, GL_UNSIGNED_INT_24_8);
            DEFINE_CASE(a8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(l8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(la8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(rgb8, GL_UNSIGNED_BYTE);
            DEFINE_CASE(rgba8, GL_UNSIGNED_BYTE);
            default:
                E2D_ASSERT_MSG(false, "unexpected pixel type");
                return GL_UNSIGNED_BYTE;
        }
        #undef DEFINE_CASE
    }

    GLint convert_pixel_type_to_internal_format(pixel_declaration::pixel_type f) noexcept {
        #define DEFINE_CASE(x,y) case pixel_declaration::pixel_type::x: return y
        switch ( f ) {
            DEFINE_CASE(depth16, GL_DEPTH_COMPONENT16);
            DEFINE_CASE(depth24, GL_DEPTH_COMPONENT24);
            DEFINE_CASE(depth24_stencil8, GL_DEPTH24_STENCIL8);

            DEFINE_CASE(a8, GL_ALPHA);
            DEFINE_CASE(l8, GL_LUMINANCE);
            DEFINE_CASE(la8, GL_LUMINANCE_ALPHA);
            DEFINE_CASE(rgb8, GL_RGB);
            DEFINE_CASE(rgba8, GL_RGBA);

            DEFINE_CASE(rgba_dxt1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
            DEFINE_CASE(rgba_dxt3, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
            DEFINE_CASE(rgba_dxt5, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);

            DEFINE_CASE(rgb_etc1, GL_ETC1_RGB8_OES);
            DEFINE_CASE(rgb_etc2, GL_COMPRESSED_RGB8_ETC2);
            DEFINE_CASE(rgba_etc2, GL_COMPRESSED_RGBA8_ETC2_EAC);
            DEFINE_CASE(rgb_a1_etc2, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2);

            DEFINE_CASE(rgba_astc4x4, GL_COMPRESSED_RGBA_ASTC_4x4_KHR);
            DEFINE_CASE(rgba_astc5x5, GL_COMPRESSED_RGBA_ASTC_5x5_KHR);
            DEFINE_CASE(rgba_astc6x6, GL_COMPRESSED_RGBA_ASTC_6x6_KHR);
            DEFINE_CASE(rgba_astc8x8, GL_COMPRESSED_RGBA_ASTC_8x8_KHR);
            DEFINE_CASE(rgba_astc10x10, GL_COMPRESSED_RGBA_ASTC_10x10_KHR);
            DEFINE_CASE(rgba_astc12x12, GL_COMPRESSED_RGBA_ASTC_12x12_KHR);

            DEFINE_CASE(rgb_pvrtc2, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG);
            DEFINE_CASE(rgb_pvrtc4, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG);
            DEFINE_CASE(rgba_pvrtc2, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);
            DEFINE_CASE(rgba_pvrtc4, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);

            DEFINE_CASE(rgba_pvrtc2_v2, GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG);
            DEFINE_CASE(rgba_pvrtc4_v2, GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG);
            default:
                E2D_ASSERT_MSG(false, "unexpected pixel type");
                return GL_RGBA;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_pixel_type_to_internal_format_e(pixel_declaration::pixel_type f) noexcept {
        return math::numeric_cast<GLenum>(convert_pixel_type_to_internal_format(f));
    }

    pixel_declaration convert_image_data_format_to_pixel_declaration(image_data_format f) noexcept {
        #define DEFINE_CASE(x,y) case image_data_format::x: return pixel_declaration::pixel_type::y
        switch ( f ) {
            DEFINE_CASE(a8, a8);
            DEFINE_CASE(l8, l8);
            DEFINE_CASE(la8, la8);
            DEFINE_CASE(rgb8, rgb8);
            DEFINE_CASE(rgba8, rgba8);

            DEFINE_CASE(rgba_dxt1, rgba_dxt1);
            DEFINE_CASE(rgba_dxt3, rgba_dxt3);
            DEFINE_CASE(rgba_dxt5, rgba_dxt5);

            DEFINE_CASE(rgb_etc1, rgb_etc1);
            DEFINE_CASE(rgb_etc2, rgb_etc2);
            DEFINE_CASE(rgba_etc2, rgba_etc2);
            DEFINE_CASE(rgb_a1_etc2, rgb_a1_etc2);

            DEFINE_CASE(rgba_astc4x4, rgba_astc4x4);
            DEFINE_CASE(rgba_astc5x5, rgba_astc5x5);
            DEFINE_CASE(rgba_astc6x6, rgba_astc6x6);
            DEFINE_CASE(rgba_astc8x8, rgba_astc8x8);
            DEFINE_CASE(rgba_astc10x10, rgba_astc10x10);
            DEFINE_CASE(rgba_astc12x12, rgba_astc12x12);

            DEFINE_CASE(rgb_pvrtc2, rgb_pvrtc2);
            DEFINE_CASE(rgb_pvrtc4, rgb_pvrtc4);
            DEFINE_CASE(rgba_pvrtc2, rgba_pvrtc2);
            DEFINE_CASE(rgba_pvrtc4, rgba_pvrtc4);

            DEFINE_CASE(rgba_pvrtc2_v2, rgba_pvrtc2_v2);
            DEFINE_CASE(rgba_pvrtc4_v2, rgba_pvrtc4_v2);
            default:
                E2D_ASSERT_MSG(false, "unexpected image data format");
                return pixel_declaration(pixel_declaration::pixel_type::rgba8);
        }
        #undef DEFINE_CASE
    }

    GLenum convert_index_type(index_declaration::index_type it) noexcept {
        #define DEFINE_CASE(x,y) case index_declaration::index_type::x: return y;
        switch ( it ) {
            DEFINE_CASE(unsigned_short, GL_UNSIGNED_SHORT);
            DEFINE_CASE(unsigned_int, GL_UNSIGNED_INT);
            default:
                E2D_ASSERT_MSG(false, "unexpected index type");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_attribute_type(vertex_declaration::attribute_type at) noexcept {
        #define DEFINE_CASE(x,y) case vertex_declaration::attribute_type::x: return y;
        switch ( at ) {
            DEFINE_CASE(signed_byte, GL_BYTE);
            DEFINE_CASE(unsigned_byte, GL_UNSIGNED_BYTE);
            DEFINE_CASE(signed_short, GL_SHORT);
            DEFINE_CASE(unsigned_short, GL_UNSIGNED_SHORT);
            DEFINE_CASE(floating_point, GL_FLOAT);
            default:
                E2D_ASSERT_MSG(false, "unexpected attribute type");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLint convert_uniform_type(uniform_type ut) noexcept {
        #define DEFINE_CASE(x,y) case uniform_type::x: return y;
        switch ( ut ) {
            DEFINE_CASE(signed_integer, GL_INT);
            DEFINE_CASE(floating_point, GL_FLOAT);

            DEFINE_CASE(v2i, GL_INT_VEC2);
            DEFINE_CASE(v3i, GL_INT_VEC3);
            DEFINE_CASE(v4i, GL_INT_VEC4);

            DEFINE_CASE(v2f, GL_FLOAT_VEC2);
            DEFINE_CASE(v3f, GL_FLOAT_VEC3);
            DEFINE_CASE(v4f, GL_FLOAT_VEC4);

            DEFINE_CASE(m2f, GL_FLOAT_MAT2);
            DEFINE_CASE(m3f, GL_FLOAT_MAT3);
            DEFINE_CASE(m4f, GL_FLOAT_MAT4);

            DEFINE_CASE(sampler_2d, GL_SAMPLER_2D);
            DEFINE_CASE(sampler_cube, GL_SAMPLER_CUBE);
            default:
                E2D_ASSERT_MSG(false, "unexpected uniform type");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLint convert_attribute_type(attribute_type at) noexcept {
        #define DEFINE_CASE(x,y) case attribute_type::x: return y;
        switch ( at ) {
            DEFINE_CASE(floating_point, GL_FLOAT);

            DEFINE_CASE(v2f, GL_FLOAT_VEC2);
            DEFINE_CASE(v3f, GL_FLOAT_VEC3);
            DEFINE_CASE(v4f, GL_FLOAT_VEC4);

            DEFINE_CASE(m2f, GL_FLOAT_MAT2);
            DEFINE_CASE(m3f, GL_FLOAT_MAT3);
            DEFINE_CASE(m4f, GL_FLOAT_MAT4);
            default:
                E2D_ASSERT_MSG(false, "unexpected attribute type");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLint convert_sampler_wrap(render::sampler_wrap w) noexcept {
        #define DEFINE_CASE(x,y) case render::sampler_wrap::x: return y;
        switch ( w ) {
            DEFINE_CASE(clamp, GL_CLAMP_TO_EDGE);
            DEFINE_CASE(repeat, GL_REPEAT);
            DEFINE_CASE(mirror, GL_MIRRORED_REPEAT);
            default:
                E2D_ASSERT_MSG(false, "unexpected sampler wrap");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLint convert_sampler_filter(render::sampler_min_filter f) noexcept {
        #define DEFINE_CASE(x,y) case render::sampler_min_filter::x: return y;
        switch ( f ) {
            DEFINE_CASE(nearest, GL_NEAREST);
            DEFINE_CASE(linear, GL_LINEAR);
            default:
                E2D_ASSERT_MSG(false, "unexpected sampler min filter");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLint convert_sampler_filter(render::sampler_mag_filter f) noexcept {
        #define DEFINE_CASE(x,y) case render::sampler_mag_filter::x: return y;
        switch ( f ) {
            DEFINE_CASE(nearest, GL_NEAREST);
            DEFINE_CASE(linear, GL_LINEAR);
            default:
                E2D_ASSERT_MSG(false, "unexpected sampler mag filter");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_buffer_usage(index_buffer::usage u) noexcept {
        #define DEFINE_CASE(x,y) case index_buffer::usage::x: return y;
        switch ( u ) {
            DEFINE_CASE(static_draw, GL_STATIC_DRAW);
            DEFINE_CASE(stream_draw, GL_STREAM_DRAW);
            DEFINE_CASE(dynamic_draw, GL_DYNAMIC_DRAW);
            default:
                E2D_ASSERT_MSG(false, "unexpected index buffer usage");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_buffer_usage(vertex_buffer::usage u) noexcept {
        #define DEFINE_CASE(x,y) case vertex_buffer::usage::x: return y;
        switch ( u ) {
            DEFINE_CASE(static_draw, GL_STATIC_DRAW);
            DEFINE_CASE(stream_draw, GL_STREAM_DRAW);
            DEFINE_CASE(dynamic_draw, GL_DYNAMIC_DRAW);
            default:
                E2D_ASSERT_MSG(false, "unexpected vertex buffer usage");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_topology(render::topology t) noexcept {
        #define DEFINE_CASE(x,y) case render::topology::x: return y;
        switch ( t ) {
            DEFINE_CASE(triangles, GL_TRIANGLES);
            DEFINE_CASE(triangles_fan, GL_TRIANGLE_FAN);
            DEFINE_CASE(triangles_strip, GL_TRIANGLE_STRIP);
            default:
                E2D_ASSERT_MSG(false, "unexpected render topology");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_stencil_op(render::stencil_op sa) noexcept {
        #define DEFINE_CASE(x,y) case render::stencil_op::x: return y;
        switch ( sa ) {
            DEFINE_CASE(keep, GL_KEEP);
            DEFINE_CASE(zero, GL_ZERO);
            DEFINE_CASE(replace, GL_REPLACE);
            DEFINE_CASE(incr, GL_INCR);
            DEFINE_CASE(incr_wrap, GL_INCR_WRAP);
            DEFINE_CASE(decr, GL_DECR);
            DEFINE_CASE(decr_wrap, GL_DECR_WRAP);
            DEFINE_CASE(invert, GL_INVERT);
            default:
                E2D_ASSERT_MSG(false, "unexpected render stencil op");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_compare_func(render::compare_func cf) noexcept {
        #define DEFINE_CASE(x,y) case render::compare_func::x: return y;
        switch ( cf ) {
            DEFINE_CASE(never, GL_NEVER);
            DEFINE_CASE(less, GL_LESS);
            DEFINE_CASE(lequal, GL_LEQUAL);
            DEFINE_CASE(greater, GL_GREATER);
            DEFINE_CASE(gequal, GL_GEQUAL);
            DEFINE_CASE(equal, GL_EQUAL);
            DEFINE_CASE(notequal, GL_NOTEQUAL);
            DEFINE_CASE(always, GL_ALWAYS);
            default:
                E2D_ASSERT_MSG(false, "unexpected render compare func");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_culling_mode(render::culling_mode cm) noexcept {
        #define DEFINE_CASE(x,y) case render::culling_mode::x: return y;
        switch ( cm ) {
            DEFINE_CASE(cw, GL_CW);
            DEFINE_CASE(ccw, GL_CCW);
            default:
                E2D_ASSERT_MSG(false, "unexpected render culling mode");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_culling_face(render::culling_face cf) noexcept {
        #define DEFINE_CASE(x,y) case render::culling_face::x: return y;
        switch ( cf ) {
            DEFINE_CASE(back, GL_BACK);
            DEFINE_CASE(front, GL_FRONT);
            DEFINE_CASE(back_and_front, GL_FRONT_AND_BACK);
            default:
                E2D_ASSERT_MSG(false, "unexpected render culling face");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_blending_factor(render::blending_factor bf) noexcept {
        #define DEFINE_CASE(x,y) case render::blending_factor::x: return y;
        switch ( bf ) {
            DEFINE_CASE(zero, GL_ZERO);
            DEFINE_CASE(one, GL_ONE);
            DEFINE_CASE(src_color, GL_SRC_COLOR);
            DEFINE_CASE(one_minus_src_color, GL_ONE_MINUS_SRC_COLOR);
            DEFINE_CASE(dst_color, GL_DST_COLOR);
            DEFINE_CASE(one_minus_dst_color, GL_ONE_MINUS_DST_COLOR);
            DEFINE_CASE(src_alpha, GL_SRC_ALPHA);
            DEFINE_CASE(one_minus_src_alpha, GL_ONE_MINUS_SRC_ALPHA);
            DEFINE_CASE(dst_alpha, GL_DST_ALPHA);
            DEFINE_CASE(one_minus_dst_alpha, GL_ONE_MINUS_DST_ALPHA);
            DEFINE_CASE(constant_color, GL_CONSTANT_COLOR);
            DEFINE_CASE(one_minus_constant_color, GL_ONE_MINUS_CONSTANT_COLOR);
            DEFINE_CASE(constant_alpha, GL_CONSTANT_ALPHA);
            DEFINE_CASE(one_minus_constant_alpha, GL_ONE_MINUS_CONSTANT_ALPHA);
            DEFINE_CASE(src_alpha_saturate, GL_SRC_ALPHA_SATURATE);
            default:
                E2D_ASSERT_MSG(false, "unexpected render blending factor");
                return 0;
        }
        #undef DEFINE_CASE
    }

    GLenum convert_blending_equation(render::blending_equation be) noexcept {
        #define DEFINE_CASE(x,y) case render::blending_equation::x: return y;
        switch ( be ) {
            DEFINE_CASE(add, GL_FUNC_ADD);
            DEFINE_CASE(subtract, GL_FUNC_SUBTRACT);
            DEFINE_CASE(reverse_subtract, GL_FUNC_REVERSE_SUBTRACT);
            default:
                E2D_ASSERT_MSG(false, "unexpected render blending equation");
                return 0;
        }
        #undef DEFINE_CASE
    }

    uniform_type glsl_type_to_uniform_type(GLenum t) noexcept {
        #define DEFINE_CASE(x,y) case x: return uniform_type::y
        switch ( t ) {
            DEFINE_CASE(GL_INT, signed_integer);
            DEFINE_CASE(GL_FLOAT, floating_point);

            DEFINE_CASE(GL_INT_VEC2, v2i);
            DEFINE_CASE(GL_INT_VEC3, v3i);
            DEFINE_CASE(GL_INT_VEC4, v4i);

            DEFINE_CASE(GL_FLOAT_VEC2, v2f);
            DEFINE_CASE(GL_FLOAT_VEC3, v3f);
            DEFINE_CASE(GL_FLOAT_VEC4, v4f);

            DEFINE_CASE(GL_FLOAT_MAT2, m2f);
            DEFINE_CASE(GL_FLOAT_MAT3, m3f);
            DEFINE_CASE(GL_FLOAT_MAT4, m4f);

            DEFINE_CASE(GL_SAMPLER_2D, sampler_2d);
            DEFINE_CASE(GL_SAMPLER_CUBE, sampler_cube);
            default:
                return uniform_type::unknown;
        }
        #undef DEFINE_CASE
    }

    attribute_type glsl_type_to_attribute_type(GLenum t) noexcept {
        #define DEFINE_CASE(x,y) case x: return attribute_type::y
        switch ( t ) {
            DEFINE_CASE(GL_FLOAT, floating_point);

            DEFINE_CASE(GL_FLOAT_VEC2, v2f);
            DEFINE_CASE(GL_FLOAT_VEC3, v3f);
            DEFINE_CASE(GL_FLOAT_VEC4, v4f);

            DEFINE_CASE(GL_FLOAT_MAT2, m2f);
            DEFINE_CASE(GL_FLOAT_MAT3, m3f);
            DEFINE_CASE(GL_FLOAT_MAT4, m4f);
            default:
                return attribute_type::unknown;
        }
        #undef DEFINE_CASE
    }

    const char* glsl_type_to_cstr(GLenum t) noexcept {
        #define DEFINE_CASE(x) case x: return #x
        switch ( t ) {
            DEFINE_CASE(GL_INT);
            DEFINE_CASE(GL_FLOAT);

            DEFINE_CASE(GL_INT_VEC2);
            DEFINE_CASE(GL_INT_VEC3);
            DEFINE_CASE(GL_INT_VEC4);

            DEFINE_CASE(GL_FLOAT_VEC2);
            DEFINE_CASE(GL_FLOAT_VEC3);
            DEFINE_CASE(GL_FLOAT_VEC4);

            DEFINE_CASE(GL_FLOAT_MAT2);
            DEFINE_CASE(GL_FLOAT_MAT3);
            DEFINE_CASE(GL_FLOAT_MAT4);

            DEFINE_CASE(GL_SAMPLER_2D);
            DEFINE_CASE(GL_SAMPLER_CUBE);
            default:
                return "GL_UNKNOWN";
        }
        #undef DEFINE_CASE
    }

    const char* gl_error_code_to_cstr(GLenum e) noexcept {
        #define DEFINE_CASE(x) case x: return #x;
        switch ( e ) {
            DEFINE_CASE(GL_INVALID_ENUM);
            DEFINE_CASE(GL_INVALID_VALUE);
            DEFINE_CASE(GL_INVALID_OPERATION);
            DEFINE_CASE(GL_INVALID_FRAMEBUFFER_OPERATION);
            DEFINE_CASE(GL_OUT_OF_MEMORY);
            default:
                return "GL_UNKNOWN";
        }
        #undef DEFINE_CASE
    }

    const char* gl_framebuffer_status_to_cstr(GLenum s) noexcept {
        #define DEFINE_CASE(x) case x: return #x
        switch ( s ) {
            DEFINE_CASE(GL_FRAMEBUFFER_COMPLETE);
            DEFINE_CASE(GL_FRAMEBUFFER_UNSUPPORTED);
            DEFINE_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
            DEFINE_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            default:
                return "GL_FRAMEBUFFER_UNKNOWN";
        }
        #undef DEFINE_CASE
    }

    GLenum gl_target_to_get_target(GLenum t) noexcept {
        #define DEFINE_CASE(x,y) case x: return y
        switch ( t ) {
            DEFINE_CASE(GL_ARRAY_BUFFER, GL_ARRAY_BUFFER_BINDING);
            DEFINE_CASE(GL_ELEMENT_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER_BINDING);
            DEFINE_CASE(GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D);
            DEFINE_CASE(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP);
            DEFINE_CASE(GL_FRAMEBUFFER, GL_FRAMEBUFFER_BINDING);
            DEFINE_CASE(GL_RENDERBUFFER, GL_RENDERBUFFER_BINDING);
            default:
                E2D_ASSERT_MSG(false, "unexpected gl target type");
                return 0;
        }
        #undef DEFINE_CASE
    }
}

namespace e2d::opengl
{
    void gl_trace_info(debug& debug) noexcept {
        const char* vendor = nullptr;
        GL_CHECK_CODE(debug, gl_get_string(GL_VENDOR, &vendor));
        const char* renderer = nullptr;
        GL_CHECK_CODE(debug, gl_get_string(GL_RENDERER, &renderer));
        const char* version = nullptr;
        GL_CHECK_CODE(debug, gl_get_string(GL_VERSION, &version));
        const char* language = nullptr;
        GL_CHECK_CODE(debug, gl_get_string(GL_SHADING_LANGUAGE_VERSION, &language));
        const char* extensions = nullptr;
        GL_CHECK_CODE(debug, gl_get_string(GL_EXTENSIONS, &extensions));
        const auto cstr_or_undefined = [](const char* cstr) noexcept {
            return (cstr && *cstr)
                ? cstr
                : "(undefined)";
        };
        debug.trace("RENDER: opengl info:\n"
            "--> GL_VENDOR: %0\n"
            "--> GL_RENDERER: %1\n"
            "--> GL_VERSION: %2\n"
            "--> GL_SHADING_LANGUAGE_VERSION: %3\n"
            "--> GL_EXTENSIONS: %4",
            cstr_or_undefined(vendor),
            cstr_or_undefined(renderer),
            cstr_or_undefined(version),
            cstr_or_undefined(language),
            cstr_or_undefined(extensions));
    }

    void gl_trace_limits(debug& debug) noexcept {
        GLint max_texture_size = 0;
        GLint max_renderbuffer_size = 0;
        GLint max_cube_map_texture_size = 0;
        GLint max_vertex_attributes = 0;
        GLint max_texture_image_units = 0;
        GLint max_vertex_texture_image_units = 0;
        GLint max_combined_texture_image_units = 0;

        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &max_cube_map_texture_size));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attributes));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_vertex_texture_image_units));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_texture_image_units));
        debug.trace("RENDER: opengl limits:\n"
            "--> GL_MAX_TEXTURE_SIZE: %0\n"
            "--> GL_MAX_RENDERBUFFER_SIZE: %1\n"
            "--> GL_MAX_CUBE_MAP_TEXTURE_SIZE: %2\n"
            "--> GL_MAX_VERTEX_ATTRIBS: %3\n"
            "--> GL_MAX_TEXTURE_IMAGE_UNITS: %4\n"
            "--> GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: %5\n"
            "--> GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %6",
            max_texture_size,
            max_renderbuffer_size,
            max_cube_map_texture_size,
            max_vertex_attributes,
            max_texture_image_units,
            max_vertex_texture_image_units,
            max_combined_texture_image_units);
    }

    void gl_fill_device_caps(debug& debug, render::device_caps& caps) noexcept {
        GLint max_texture_size = 0;
        GLint max_renderbuffer_size = 0;
        GLint max_cube_map_texture_size = 0;

        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_renderbuffer_size));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &max_cube_map_texture_size));

        GLint max_texture_image_units = 0;
        GLint max_combined_texture_image_units = 0;

        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_image_units));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_texture_image_units));

        GLint max_vertex_attributes = 0;
        GLint max_vertex_texture_image_units = 0;

        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attributes));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_vertex_texture_image_units));

        GLint max_varying_vectors = 0;
        GLint max_vertex_uniform_vectors = 0;
        GLint max_fragment_uniform_vectors = 0;

    #if E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGLES
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VARYING_VECTORS, &max_varying_vectors));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &max_vertex_uniform_vectors));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &max_fragment_uniform_vectors));
    #elif E2D_RENDER_MODE == E2D_RENDER_MODE_OPENGL
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &max_varying_vectors));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &max_vertex_uniform_vectors));
        GL_CHECK_CODE(debug, glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max_fragment_uniform_vectors));
        max_varying_vectors *= 4;
        max_vertex_uniform_vectors *= 4;
        max_fragment_uniform_vectors *= 4;
    #else
    #   error unknown render mode
    #endif

        caps.max_texture_size = math::numeric_cast<u32>(max_texture_size);
        caps.max_renderbuffer_size = math::numeric_cast<u32>(max_renderbuffer_size);
        caps.max_cube_map_texture_size = math::numeric_cast<u32>(max_cube_map_texture_size);

        caps.max_texture_image_units = math::numeric_cast<u32>(max_texture_image_units);
        caps.max_combined_texture_image_units = math::numeric_cast<u32>(max_combined_texture_image_units);

        caps.max_vertex_attributes = math::numeric_cast<u32>(max_vertex_attributes);
        caps.max_vertex_texture_image_units = math::numeric_cast<u32>(max_vertex_texture_image_units);

        caps.max_varying_vectors = math::numeric_cast<u32>(max_varying_vectors);
        caps.max_vertex_uniform_vectors = math::numeric_cast<u32>(max_vertex_uniform_vectors);
        caps.max_fragment_uniform_vectors = math::numeric_cast<u32>(max_fragment_uniform_vectors);

        const gl_version version = gl_get_version(debug);

        caps.profile =
            version >= gl_version::gles_300 ? render::api_profile::opengles3 :
            version >= gl_version::gles_200 ? render::api_profile::opengles2 :
            version >= gl_version::gl_410 ? render::api_profile::opengl4_compat :
            render::api_profile::opengl2_compat;

        caps.npot_texture_supported =
            version >= gl_version::gl_210 || // gl_200
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_texture_npot",
                "GL_ARB_texture_non_power_of_two");

        caps.depth_texture_supported =
            version >= gl_version::gl_210 ||
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_depth_texture",
                "GL_ARB_depth_texture"); // gl_140

        caps.render_target_supported =
            version >= gl_version::gl_300 || // gl_300
            version >= gl_version::gles_200 ||
            gl_has_any_extension(debug,
                "GL_OES_framebuffer_object",
                "GL_EXT_framebuffer_object",
                "GL_ARB_framebuffer_object");

        caps.element_index_uint =
            version >= gl_version::gl_210 || // gl_100
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_element_index_uint");

        caps.depth16_supported =
            version >= gl_version::gl_210 || // gl_140
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_depth_texture",
                "GL_ARB_depth_texture");

        caps.depth24_supported =
            version >= gl_version::gl_210 || // gl_140
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_depth24",
                "GL_ARB_depth_texture");

        caps.depth24_stencil8_supported =
            version >= gl_version::gl_300 || // gl_300
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_OES_packed_depth_stencil",
                "GL_EXT_packed_depth_stencil");

        caps.dxt_compression_supported =
            gl_has_any_extension(debug,
                "GL_EXT_texture_compression_s3tc");

        caps.etc1_compression_supported =
            gl_has_any_extension(debug,
                "GL_OES_compressed_ETC1_RGB8_texture");

        caps.etc2_compression_supported =
            version >= gl_version::gles_300 ||
            gl_has_any_extension(debug,
                "GL_ARB_ES3_compatibility");

        caps.astc_compression_supported =
            gl_has_any_extension(debug,
                "GL_OES_texture_compression_astc",
                "GL_KHR_texture_compression_astc_ldr",
                "GL_KHR_texture_compression_astc_hdr");

        caps.pvrtc_compression_supported =
            gl_has_any_extension(debug,
                "GL_IMG_texture_compression_pvrtc");

        caps.pvrtc2_compression_supported =
            gl_has_any_extension(debug,
                "GL_IMG_texture_compression_pvrtc2");
    }
    
    bool gl_has_extension(debug& debug, str_view name) noexcept {
        return gl_has_any_extension(debug, name);
    }

    gl_shader_id gl_compile_shader(
        debug& debug,
        str_view header,
        str_view source,
        GLenum type) noexcept
    {
        gl_shader_id id = gl_shader_id::create(debug, type);
        if ( id.empty() ) {
            return id;
        }

        const GLchar* sources[] = {
            header.empty() ? "" : header.data(),
            source.empty() ? "" : source.data()};
        const GLint source_lengths[] = {
            math::numeric_cast<GLint>(header.size()),
            math::numeric_cast<GLint>(source.size())};
        static_assert(std::size(sources) == std::size(source_lengths));

        GL_CHECK_CODE(debug, glShaderSource(
            *id,
            math::numeric_cast<GLsizei>(std::size(sources)),
            sources,
            source_lengths));
        GL_CHECK_CODE(debug, glCompileShader(*id));

        return process_shader_compilation_result(debug, *id)
            ? std::move(id)
            : gl_shader_id(debug);
    }

    gl_program_id gl_link_program(debug& debug, gl_shader_id vs, gl_shader_id fs) noexcept {
        E2D_ASSERT(!vs.empty() && !fs.empty());
        gl_program_id id = gl_program_id::create(debug);
        if ( id.empty() ) {
            return id;
        }

        GL_CHECK_CODE(debug, glAttachShader(*id, *vs));
        GL_CHECK_CODE(debug, glAttachShader(*id, *fs));
        GL_CHECK_CODE(debug, glLinkProgram(*id));

        return process_program_linking_result(debug, *id)
            ? std::move(id)
            : gl_program_id(debug);
    }

    bool gl_check_framebuffer(
        debug& debug,
        const gl_framebuffer_id& fb,
        GLenum* out_status) noexcept
    {
        E2D_ASSERT(!fb.empty());
        GLenum status = GL_FRAMEBUFFER_COMPLETE;
        with_gl_bind_framebuffer(debug, fb, [&debug, &fb, &status]() noexcept {
            GL_CHECK_CODE(debug, gl_check_framebuffer_status(
                fb.target(), &status));
        });
        if ( out_status ) {
            *out_status = status;
        }
        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    void gl_attach_texture(
        debug& debug,
        const gl_framebuffer_id& fb,
        const gl_texture_id& tex,
        GLenum attachment) noexcept
    {
        E2D_ASSERT(!fb.empty() && !tex.empty());
        with_gl_bind_framebuffer(debug, fb, [&debug, &fb, &tex, &attachment]() noexcept {
            GL_CHECK_CODE(debug, glFramebufferTexture2D(
                fb.target(),
                attachment,
                tex.target(),
                *tex,
                0));
        });
    }

    void gl_attach_renderbuffer(
        debug& debug,
        const gl_framebuffer_id& fb,
        const gl_renderbuffer_id& rb,
        GLenum attachment) noexcept
    {
        E2D_ASSERT(!fb.empty() && !rb.empty());
        with_gl_bind_framebuffer(debug, fb, [&debug, &fb, &rb, &attachment]() noexcept {
            GL_CHECK_CODE(debug, glFramebufferRenderbuffer(
                fb.target(),
                attachment,
                rb.target(),
                *rb));
        });
    }

    gl_renderbuffer_id gl_compile_renderbuffer(
        debug& debug,
        const v2u& size,
        GLenum format)
    {
        gl_renderbuffer_id id = gl_renderbuffer_id::create(
            debug, GL_RENDERBUFFER);
        if ( id.empty() ) {
            return id;
        }
        with_gl_bind_renderbuffer(debug, id, [&debug, &id, &size, &format]() noexcept {
            GL_CHECK_CODE(debug, glRenderbufferStorage(
                id.target(),
                format,
                math::numeric_cast<GLsizei>(size.x),
                math::numeric_cast<GLsizei>(size.y)));
        });
        return id;
    }
}

namespace e2d::opengl
{
    void grab_program_uniforms(
        debug& debug,
        GLuint program,
        vector<uniform_info>& uniforms)
    {
        E2D_ASSERT(program && glIsProgram(program));

        GLint uniform_count = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(
            program, GL_ACTIVE_UNIFORMS, &uniform_count));

        GLint uniform_max_len = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(
            program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_max_len));

        GLchar* name_buffer = static_cast<GLchar*>(E2D_ALLOCA(
            sizeof(GLchar) * math::numeric_cast<std::size_t>(uniform_max_len)));

        uniforms.reserve(uniforms.size() +
            math::numeric_cast<std::size_t>(uniform_count));

        for ( GLuint i = 0, e = math::numeric_cast<GLuint>(uniform_count); i < e; ++i ) {
            GLint size = 0;
            GLenum type = 0;
            GL_CHECK_CODE(debug, glGetActiveUniform(
                program, i, uniform_max_len,
                nullptr, &size, &type, name_buffer));
            GLint location = 0;
            GL_CHECK_CODE(debug, gl_get_uniform_location(
                program, name_buffer, &location));
            uniforms.emplace_back(
                name_buffer,
                size,
                location,
                glsl_type_to_uniform_type(type));
        }
    }

    void grab_program_attributes(
        debug& debug,
        GLuint program,
        vector<attribute_info>& attributes)
    {
        E2D_ASSERT(program && glIsProgram(program));

        GLint attribute_count = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(
            program, GL_ACTIVE_ATTRIBUTES, &attribute_count));

        GLint attribute_max_len = 0;
        GL_CHECK_CODE(debug, glGetProgramiv(
            program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attribute_max_len));

        GLchar* name_buffer = static_cast<GLchar*>(E2D_ALLOCA(
            sizeof(GLchar) * math::numeric_cast<std::size_t>(attribute_max_len)));

        attributes.reserve(attributes.size() +
            math::numeric_cast<std::size_t>(attribute_count));

        for ( GLuint i = 0, e = math::numeric_cast<GLuint>(attribute_count); i < e; ++i ) {
            GLint size = 0;
            GLenum type = 0;
            GL_CHECK_CODE(debug, glGetActiveAttrib(
                program, i, attribute_max_len,
                nullptr, &size, &type, name_buffer));
            GLint location = 0;
            GL_CHECK_CODE(debug, gl_get_attribute_location(
                program, name_buffer, &location));
            attributes.emplace_back(
                name_buffer,
                size,
                location,
                glsl_type_to_attribute_type(type));
        }
    }
}

#endif
#endif

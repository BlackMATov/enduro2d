/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "render_impl/render.hpp"

namespace
{
    using namespace e2d;

    struct pixel_type_description {
        const char* cstr;
        u32 bits_per_pixel;
        bool color;
        bool depth;
        bool stencil;
        pixel_declaration::pixel_type type;
        bool compressed;
        v2u block_size;
    };

    const pixel_type_description pixel_type_descriptions[] = {
        {"unknown",           0, false, false, false, pixel_declaration::pixel_type::unknown,          false, v2u(1)},

        {"depth16",          16, false, true,  false, pixel_declaration::pixel_type::depth16,          false, v2u(1)},
        {"depth16_stencil8",  0, false, true,  true,  pixel_declaration::pixel_type::depth16_stencil8, false, v2u(1)},
        {"depth24",          24, false, true,  false, pixel_declaration::pixel_type::depth24,          false, v2u(1)},
        {"depth24_stencil8", 32, false, true,  true,  pixel_declaration::pixel_type::depth24_stencil8, false, v2u(1)},
        {"depth32",          32, false, true,  false, pixel_declaration::pixel_type::depth32,          false, v2u(1)},
        {"depth32_stencil8",  0, false, true,  true,  pixel_declaration::pixel_type::depth32_stencil8, false, v2u(1)},

        {"g8",                8, true,  false, false, pixel_declaration::pixel_type::g8,               false, v2u(1)},
        {"ga8",              16, true,  false, false, pixel_declaration::pixel_type::ga8,              false, v2u(1)},
        {"rgb8",             24, true,  false, false, pixel_declaration::pixel_type::rgb8,             false, v2u(1)},
        {"rgba8",            32, true,  false, false, pixel_declaration::pixel_type::rgba8,            false, v2u(1)},

        {"rgb_dxt1",          4, true,  false, false, pixel_declaration::pixel_type::rgb_dxt1,         true,  v2u(4,4)},
        {"rgba_dxt1",         4, true,  false, false, pixel_declaration::pixel_type::rgba_dxt1,        true,  v2u(4,4)},
        {"rgba_dxt3",         8, true,  false, false, pixel_declaration::pixel_type::rgba_dxt3,        true,  v2u(4,4)},
        {"rgba_dxt5",         8, true,  false, false, pixel_declaration::pixel_type::rgba_dxt5,        true,  v2u(4,4)},

        {"rgb_pvrtc2",        2, true,  false, false, pixel_declaration::pixel_type::rgb_pvrtc2,       true,  v2u(8,4)},
        {"rgb_pvrtc4",        4, true,  false, false, pixel_declaration::pixel_type::rgb_pvrtc4,       true,  v2u(4,4)},
        {"rgba_pvrtc2",       2, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc2,      true,  v2u(8,4)},
        {"rgba_pvrtc4",       4, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc4,      true,  v2u(4,4)},

        {"rgba_pvrtc2",       2, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc2,      true,  v2u(8,4)},
        {"rgba_pvrtc4",       4, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc4,      true,  v2u(4,4)},

        {"rgba_pvrtc2_v2",    2, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc2_v2,   true,  v2u(8,4)},
        {"rgba_pvrtc4_v2",    4, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc4_v2,   true,  v2u(4,4)}
    };

    const pixel_type_description& get_pixel_type_description(pixel_declaration::pixel_type type) noexcept {
        const std::size_t index = math::numeric_cast<std::size_t>(utils::enum_to_underlying(type));
        E2D_ASSERT(index < std::size(pixel_type_descriptions));
        const pixel_type_description& desc = pixel_type_descriptions[index];
        E2D_ASSERT(desc.type == type);
        return desc;
    }

    const char* index_element_cstr(index_declaration::index_type it) noexcept {
        #define DEFINE_CASE(x) case index_declaration::index_type::x: return #x;
        switch ( it ) {
            DEFINE_CASE(unsigned_short);
            DEFINE_CASE(unsigned_int);
            default:
                E2D_ASSERT_MSG(false, "unexpected index type");
                return "";
        }
        #undef DEFINE_CASE
    }

    std::size_t index_element_size(index_declaration::index_type it) noexcept {
        #define DEFINE_CASE(x,y) case index_declaration::index_type::x: return y;
        switch ( it ) {
            DEFINE_CASE(unsigned_short, sizeof(u16));
            DEFINE_CASE(unsigned_int, sizeof(u32));
            default:
                E2D_ASSERT_MSG(false, "unexpected index type");
                return 0;
        }
        #undef DEFINE_CASE
    }

    std::size_t attribute_element_size(vertex_declaration::attribute_type at) noexcept {
        #define DEFINE_CASE(x,y) case vertex_declaration::attribute_type::x: return y;
        switch ( at ) {
            DEFINE_CASE(signed_byte, sizeof(u8));
            DEFINE_CASE(unsigned_byte, sizeof(u8));
            DEFINE_CASE(signed_short, sizeof(u16));
            DEFINE_CASE(unsigned_short, sizeof(u16));
            DEFINE_CASE(floating_point, sizeof(u32));
            default:
                E2D_ASSERT_MSG(false, "unexpected attribute type");
                return 0;
        }
        #undef DEFINE_CASE
    }
    
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

    class command_value_visitor final : private noncopyable {
    public:
        command_value_visitor(render& render) noexcept
        : render_(render) {}

        void operator()(const render::zero_command& command) const {
            E2D_UNUSED(command);
        }

        void operator()(const render::bind_vertex_buffers_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::material_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::scissor_command& command) const {
            render_.execute(command);
        }
        
        void operator()(const render::blending_state_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::culling_state_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::stencil_state_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::depth_state_command& command) const {
            render_.execute(command);
        }
        
        void operator()(const render::blend_constant_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::draw_command& command) const {
            render_.execute(command);
        }

        void operator()(const render::draw_indexed_command& command) const {
            render_.execute(command);
        }
    private:
        render& render_;
    };
}

namespace e2d
{
    //
    // pixel_declaration
    //

    const char* pixel_declaration::pixel_type_to_cstr(pixel_type pt) noexcept {
        return get_pixel_type_description(pt).cstr;
    }

    pixel_declaration::pixel_declaration(pixel_type type) noexcept
    : type_(type) {}

    pixel_declaration::pixel_type pixel_declaration::type() const noexcept {
        return type_;
    }

    bool pixel_declaration::is_color() const noexcept {
        return get_pixel_type_description(type_).color;
    }

    bool pixel_declaration::is_depth() const noexcept {
        return get_pixel_type_description(type_).depth;
    }

    bool pixel_declaration::is_stencil() const noexcept {
        return get_pixel_type_description(type_).stencil;
    }

    bool pixel_declaration::is_compressed() const noexcept {
        return get_pixel_type_description(type_).compressed;
    }

    std::size_t pixel_declaration::bits_per_pixel() const noexcept {
        return get_pixel_type_description(type_).bits_per_pixel;
    }

    v2u pixel_declaration::compressed_block_size() const noexcept {
        return get_pixel_type_description(type_).block_size;
    }

    bool operator==(const pixel_declaration& l, const pixel_declaration& r) noexcept {
        return l.type() == r.type();
    }

    bool operator!=(const pixel_declaration& l, const pixel_declaration& r) noexcept {
        return !(l == r);
    }

    //
    // index_declaration
    //

    const char* index_declaration::index_type_to_cstr(index_type it) noexcept {
        return index_element_cstr(it);
    }

    index_declaration::index_declaration(index_type type) noexcept
    : type_(type) {}

    index_declaration::index_type index_declaration::type() const noexcept {
        return type_;
    }

    std::size_t index_declaration::bytes_per_index() const noexcept {
        return index_element_size(type_);
    }

    bool operator==(const index_declaration& l, const index_declaration& r) noexcept {
        return l.type() == r.type();
    }

    bool operator!=(const index_declaration& l, const index_declaration& r) noexcept {
        return !(l == r);
    }

    //
    // vertex_declaration::attribute_info
    //

    vertex_declaration::attribute_info::attribute_info(
        std::size_t nstride,
        str_hash nname,
        u8 nrows,
        u8 ncolumns,
        attribute_type ntype,
        bool nnormalized) noexcept
    : stride(nstride)
    , name(std::move(nname))
    , rows(nrows)
    , columns(ncolumns)
    , type(ntype)
    , normalized(nnormalized) {}

    std::size_t vertex_declaration::attribute_info::row_size() const noexcept {
        return attribute_element_size(type) * columns;
    }
    
    //
    // vertex_declaration::hash
    //

    std::size_t vertex_declaration::hash::operator()(const vertex_declaration& x) const noexcept {
        return hash_of(x);
    }

    //
    // vertex_declaration
    //

    vertex_declaration& vertex_declaration::normalized() noexcept {
        E2D_ASSERT(attribute_count_ > 0);
        attributes_[attribute_count_ - 1].normalized = true;
        return *this;
    }

    vertex_declaration& vertex_declaration::skip_bytes(std::size_t bytes) noexcept {
        bytes_per_vertex_ += bytes;
        return *this;
    }

    vertex_declaration& vertex_declaration::add_attribute(
        str_hash name,
        u8 rows,
        u8 columns,
        attribute_type type,
        bool normalized) noexcept
    {
        E2D_ASSERT(attribute_count_ < attributes_.size());
        const std::size_t stride = bytes_per_vertex_;
        attributes_[attribute_count_] = attribute_info(
            stride,
            name,
            rows,
            columns,
            type,
            normalized);
        bytes_per_vertex_ += attribute_element_size(type) * rows * columns;
        ++attribute_count_;
        return *this;
    }

    const vertex_declaration::attribute_info& vertex_declaration::attribute(std::size_t index) const noexcept {
        E2D_ASSERT(index < attribute_count_);
        return attributes_[index];
    }

    std::size_t vertex_declaration::attribute_count() const noexcept {
        return attribute_count_;
    }

    std::size_t vertex_declaration::bytes_per_vertex() const noexcept {
        return bytes_per_vertex_;
    }

    bool operator==(const vertex_declaration& l, const vertex_declaration& r) noexcept {
        if ( l.bytes_per_vertex() != r.bytes_per_vertex() ) {
            return false;
        }
        if ( l.attribute_count() != r.attribute_count() ) {
            return false;
        }
        for ( std::size_t i = 0, e = l.attribute_count(); i < e; ++i ) {
            if ( l.attribute(i) != r.attribute(i) ) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const vertex_declaration& l, const vertex_declaration& r) noexcept {
        return !(l == r);
    }

    bool operator==(
        const vertex_declaration::attribute_info& l,
        const vertex_declaration::attribute_info& r) noexcept
    {
        return l.stride == r.stride
            && l.name == r.name
            && l.rows == r.rows
            && l.columns == r.columns
            && l.type == r.type
            && l.normalized == r.normalized;
    }

    bool operator!=(
        const vertex_declaration::attribute_info& l,
        const vertex_declaration::attribute_info& r) noexcept
    {
        return !(l == r);
    }

    //
    // depth_state
    //
    
    render::depth_state& render::depth_state::test(bool enable) noexcept {
        test_ = enable;
        return *this;
    }

    render::depth_state& render::depth_state::write(bool enable) noexcept {
        write_ = enable;
        return *this;
    }

    render::depth_state& render::depth_state::func(compare_func func) noexcept {
        func_ = func;
        return *this;
    }
    
    bool render::depth_state::test() const noexcept {
        return test_;
    }

    bool render::depth_state::write() const noexcept {
        return write_;
    }

    render::compare_func render::depth_state::func() const noexcept {
        return func_;
    }
    
    //
    // depth_dynamic_state
    //

    render::depth_dynamic_state& render::depth_dynamic_state::test(bool enable) noexcept {
        test_ = enable;
        return *this;
    }

    render::depth_dynamic_state& render::depth_dynamic_state::write(bool enable) noexcept {
        write_ = enable;
        return *this;
    }

    bool render::depth_dynamic_state::test() const noexcept {
        return test_;
    }

    bool render::depth_dynamic_state::write() const noexcept {
        return write_;
    }

    //
    // stencil_state
    //
    
    render::stencil_state& render::stencil_state::test(bool enabled) noexcept {
        test_ = enabled;
        return *this;
    }

    render::stencil_state& render::stencil_state::write(u8 mask) noexcept {
        write_mask_ = mask;
        return *this;
    }

    render::stencil_state& render::stencil_state::func(compare_func func, u8 ref, u8 mask) noexcept {
        func_ = func;
        ref_ = ref;
        read_ = mask;
        return *this;
    }

    render::stencil_state& render::stencil_state::op(stencil_op pass, stencil_op sfail, stencil_op zfail) noexcept {
        pass_ = pass;
        sfail_ = sfail;
        zfail_ = zfail;
        return *this;
    }

    bool render::stencil_state::test() const noexcept {
        return test_;
    }

    u8 render::stencil_state::write() const noexcept {
        return write_mask_;
    }

    render::compare_func render::stencil_state::func() const noexcept {
        return func_;
    }

    u8 render::stencil_state::ref() const noexcept {
        return ref_;
    }

    u8 render::stencil_state::mask() const noexcept {
        return read_;
    }

    render::stencil_op render::stencil_state::pass() const noexcept {
        return pass_;
    }

    render::stencil_op render::stencil_state::sfail() const noexcept {
        return sfail_;
    }

    render::stencil_op render::stencil_state::zfail() const noexcept {
        return zfail_;
    }

    //
    // culling_state
    //

    render::culling_state& render::culling_state::face(culling_face value) noexcept {
        face_ = value;
        return *this;
    }
    
    render::culling_state& render::culling_state::enable(bool value) noexcept {
        enabled_ = value;
        return *this;
    }

    render::culling_face render::culling_state::face() const noexcept {
        return face_;
    }
    
    bool render::culling_state::enabled() const noexcept {
        return enabled_;
    }

    //
    // blending_state
    //
    
    render::blending_state& render::blending_state::enable(bool value) noexcept {
        enabled_ = value;
        return *this;
    }

    render::blending_state& render::blending_state::color_mask(blending_color_mask mask) noexcept {
        color_mask_ = mask;
        return *this;
    }

    render::blending_state& render::blending_state::factor(blending_factor src, blending_factor dst) noexcept {
        rgb_factor(src, dst);
        alpha_factor(src, dst);
        return *this;
    }

    render::blending_state& render::blending_state::src_factor(blending_factor src) noexcept {
        src_rgb_factor(src);
        src_alpha_factor(src);
        return *this;
    }

    render::blending_state& render::blending_state::dst_factor(blending_factor dst) noexcept {
        dst_rgb_factor(dst);
        dst_alpha_factor(dst);
        return *this;
    }

    render::blending_state& render::blending_state::rgb_factor(blending_factor src, blending_factor dst) noexcept {
        src_rgb_factor(src);
        dst_rgb_factor(dst);
        return *this;
    }

    render::blending_state& render::blending_state::src_rgb_factor(blending_factor src) noexcept {
        src_rgb_factor_ = src;
        return *this;
    }

    render::blending_state& render::blending_state::dst_rgb_factor(blending_factor dst) noexcept {
        dst_rgb_factor_ = dst;
        return *this;
    }

    render::blending_state& render::blending_state::alpha_factor(blending_factor src, blending_factor dst) noexcept {
        src_alpha_factor(src);
        dst_alpha_factor(dst);
        return *this;
    }

    render::blending_state& render::blending_state::src_alpha_factor(blending_factor src) noexcept {
        src_alpha_factor_ = src;
        return *this;
    }

    render::blending_state& render::blending_state::dst_alpha_factor(blending_factor dst) noexcept {
        dst_alpha_factor_ = dst;
        return *this;
    }

    render::blending_state& render::blending_state::equation(blending_equation equation) noexcept {
        rgb_equation(equation);
        alpha_equation(equation);
        return *this;
    }

    render::blending_state& render::blending_state::rgb_equation(blending_equation equation) noexcept {
        rgb_equation_ = equation;
        return *this;
    }

    render::blending_state& render::blending_state::alpha_equation(blending_equation equation) noexcept {
        alpha_equation_ = equation;
        return *this;
    }
    
    bool render::blending_state::enabled() const noexcept {
        return enabled_;
    }

    render::blending_color_mask render::blending_state::color_mask() const noexcept {
        return color_mask_;
    }

    render::blending_factor render::blending_state::src_rgb_factor() const noexcept {
        return src_rgb_factor_;
    }

    render::blending_factor render::blending_state::dst_rgb_factor() const noexcept {
        return dst_rgb_factor_;
    }

    render::blending_factor render::blending_state::src_alpha_factor() const noexcept {
        return src_alpha_factor_;
    }

    render::blending_factor render::blending_state::dst_alpha_factor() const noexcept {
        return dst_alpha_factor_;
    }

    render::blending_equation render::blending_state::rgb_equation() const noexcept {
        return rgb_equation_;
    }

    render::blending_equation render::blending_state::alpha_equation() const noexcept {
        return alpha_equation_;
    }

    //
    // state_block
    //

    render::state_block& render::state_block::depth(const depth_state& state) noexcept {
        depth_ = state;
        return *this;
    }

    render::state_block& render::state_block::stencil(const stencil_state& state) noexcept {
        stencil_ = state;
        return *this;
    }

    render::state_block& render::state_block::culling(const culling_state& state) noexcept {
        culling_ = state;
        return *this;
    }

    render::state_block& render::state_block::blending(const blending_state& state) noexcept {
        blending_ = state;
        return *this;
    }

    render::depth_state& render::state_block::depth() noexcept {
        return depth_;
    }

    render::stencil_state& render::state_block::stencil() noexcept {
        return stencil_;
    }

    render::culling_state& render::state_block::culling() noexcept {
        return culling_;
    }

    render::blending_state& render::state_block::blending() noexcept {
        return blending_;
    }

    const render::depth_state& render::state_block::depth() const noexcept {
        return depth_;
    }

    const render::stencil_state& render::state_block::stencil() const noexcept {
        return stencil_;
    }

    const render::blending_state& render::state_block::blending() const noexcept {
        return blending_;
    }
    
    const render::culling_state& render::state_block::culling() const noexcept {
        return culling_;
    }

    //
    // render::property_block::sampler
    //

    render::sampler_state& render::sampler_state::texture(const texture_ptr& texture) noexcept {
        texture_ = texture;
        return *this;
    }

    render::sampler_state& render::sampler_state::wrap(sampler_wrap st) noexcept {
        s_wrap(st);
        t_wrap(st);
        return *this;
    }

    render::sampler_state& render::sampler_state::s_wrap(sampler_wrap s) noexcept {
        s_wrap_ = s;
        return *this;
    }

    render::sampler_state& render::sampler_state::t_wrap(sampler_wrap t) noexcept {
        t_wrap_ = t;
        return *this;
    }

    render::sampler_state& render::sampler_state::filter(sampler_min_filter min, sampler_mag_filter mag) noexcept {
        min_filter(min);
        mag_filter(mag);
        return *this;
    }

    render::sampler_state& render::sampler_state::min_filter(sampler_min_filter min) noexcept {
        min_filter_ = min;
        return *this;
    }

    render::sampler_state& render::sampler_state::mag_filter(sampler_mag_filter mag) noexcept {
        mag_filter_ = mag;
        return *this;
    }

    const texture_ptr& render::sampler_state::texture() const noexcept {
        return texture_;
    }

    render::sampler_wrap render::sampler_state::s_wrap() const noexcept {
        return s_wrap_;
    }

    render::sampler_wrap render::sampler_state::t_wrap() const noexcept {
        return t_wrap_;
    }

    render::sampler_min_filter render::sampler_state::min_filter() const noexcept {
        return min_filter_;
    }

    render::sampler_mag_filter render::sampler_state::mag_filter() const noexcept {
        return mag_filter_;
    }
    
    //
    // render::sampler_block
    //

    render::sampler_block& render::sampler_block::bind(str_hash name, const sampler_state& state) noexcept {
        for ( std::size_t i = 0; i < count_; ++i ) {
            if ( names_[i] == name ) {
                samplers_[i] = state;
                return *this;
            }
        }
        E2D_ASSERT(count_ < samplers_.size());
        names_[count_] = name;
        samplers_[count_] = state;
        ++count_;
        return *this;
    }

    std::size_t render::sampler_block::count() const noexcept {
        return count_;
    }

    str_hash render::sampler_block::name(std::size_t index) const noexcept {
        E2D_ASSERT(index < count_);
        return names_[index];
    }

    const render::sampler_state& render::sampler_block::sampler(std::size_t index) const noexcept {
        E2D_ASSERT(index < count_);
        return samplers_[index];
    }

    //
    // render::renderpass_desc
    //

    render::renderpass_desc::renderpass_desc() {
        color_.clear_value = color::clear();
        depth_.clear_value = 1.0f;
        stencil_.clear_value = 0;
        depth_range_ = v2f(0.0f, 1.0f);
    }

    render::renderpass_desc::renderpass_desc(const render_target_ptr& rt) noexcept {
        target_ = rt;
        color_.clear_value = color::clear();
        depth_.clear_value = 1.0f;
        stencil_.clear_value = 0;
        depth_range_ = v2f(0.0f, 1.0f);
    }

    render::renderpass_desc& render::renderpass_desc::target(const render_target_ptr& value) noexcept {
        target_ = value;
        return *this;
    }

    const render_target_ptr& render::renderpass_desc::target() const noexcept {
        return target_;
    }

    render::renderpass_desc& render::renderpass_desc::viewport(const b2u& value) noexcept {
        viewport_ = value;
        return *this;
    }

    const b2u& render::renderpass_desc::viewport() const noexcept {
        return viewport_;
    }
            
    render::renderpass_desc& render::renderpass_desc::depth_range(const v2f& value) noexcept {
        depth_range_ = value;
        return *this;
    }

    const v2f& render::renderpass_desc::depth_range() const noexcept {
        return depth_range_;
    }

    render::renderpass_desc& render::renderpass_desc::states(const state_block& states) noexcept {
        states_ = states;
        return *this;
    }

    const render::state_block& render::renderpass_desc::states() const noexcept {
        return states_;
    }
            
    render::renderpass_desc& render::renderpass_desc::color_clear(const color& value) noexcept {
        color_.clear_value = value;
        color_.load_op = attachment_load_op::clear;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_load() noexcept {
        color_.load_op = attachment_load_op::load;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_invalidate() noexcept {
        color_.load_op = attachment_load_op::invalidate;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_store() noexcept {
        color_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_discard() noexcept {
        color_.store_op = attachment_store_op::discard;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_load_op(attachment_load_op value) noexcept {
        color_.load_op = value;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_store_op(attachment_store_op value) noexcept {
        color_.store_op = value;
        return *this;
    }

    const color& render::renderpass_desc::color_clear_value() const noexcept {
        E2D_ASSERT(color_.load_op == attachment_load_op::clear);
        return color_.clear_value;
    }

    render::attachment_load_op render::renderpass_desc::color_load_op() const noexcept {
        return color_.load_op;
    }

    render::attachment_store_op render::renderpass_desc::color_store_op() const noexcept {
        return color_.store_op;
    }

    render::renderpass_desc& render::renderpass_desc::depth_clear(float value) noexcept {
        depth_.clear_value = value;
        depth_.load_op = attachment_load_op::clear;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_load() noexcept {
        depth_.load_op = attachment_load_op::load;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_invalidate() noexcept {
        depth_.load_op = attachment_load_op::invalidate;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_store() noexcept {
        depth_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_discard() noexcept {
        depth_.store_op = attachment_store_op::discard;
        return *this;
    }
    
    render::renderpass_desc& render::renderpass_desc::depth_load_op(attachment_load_op value) noexcept {
        depth_.load_op = value;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_store_op(attachment_store_op value) noexcept {
        depth_.store_op = value;
        return *this;
    }

    float render::renderpass_desc::depth_clear_value() const noexcept {
        E2D_ASSERT(depth_.load_op == attachment_load_op::clear);
        return depth_.clear_value;
    }

    render::attachment_load_op render::renderpass_desc::depth_load_op() const noexcept {
        return depth_.load_op;
    }

    render::attachment_store_op render::renderpass_desc::depth_store_op() const noexcept {
        return depth_.store_op;
    }
            
    render::renderpass_desc& render::renderpass_desc::stencil_clear(u8 value) noexcept {
        stencil_.clear_value = value;
        stencil_.load_op = attachment_load_op::clear;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_load() noexcept {
        stencil_.load_op = attachment_load_op::load;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_invalidate() noexcept {
        stencil_.load_op = attachment_load_op::invalidate;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_store() noexcept {
        stencil_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_discard() noexcept {
        stencil_.store_op = attachment_store_op::discard;
        return *this;
    }
    
    render::renderpass_desc& render::renderpass_desc::stencil_load_op(attachment_load_op value) noexcept {
        stencil_.load_op = value;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_store_op(attachment_store_op value) noexcept {
        stencil_.store_op = value;
        return *this;
    }

    u8 render::renderpass_desc::stencil_clear_value() const noexcept {
        E2D_ASSERT(stencil_.load_op == attachment_load_op::clear);
        return stencil_.clear_value;
    }

    render::attachment_load_op render::renderpass_desc::stencil_load_op() const noexcept {
        return stencil_.load_op;
    }

    render::attachment_store_op render::renderpass_desc::stencil_store_op() const noexcept {
        return stencil_.store_op;
    }

    //
    // render::material
    //
    
    render::material& render::material::blending(const blending_state& value) noexcept {
        blending_ = value;
        return *this;
    }

    render::material& render::material::culling(const culling_state& value) noexcept {
        culling_ = value;
        return *this;
    }
    
    render::material& render::material::depth(const depth_dynamic_state& value) noexcept {
        depth_ = value;
        return *this;
    }

    render::material& render::material::shader(const shader_ptr& value) noexcept {
        shader_ = value;
        return *this;
    }

    render::material& render::material::constants(const const_buffer_ptr& value) noexcept {
        constants_ = value;
        return *this;
    }
    
    render::material& render::material::sampler(str_hash name, const sampler_state& sampler) noexcept {
        sampler_block_.bind(name, sampler);
        return *this;
    }
    
    render::material& render::material::samplers(const sampler_block& value) noexcept {
        sampler_block_ = value;
        return *this;
    }

    const std::optional<render::blending_state>& render::material::blending() const noexcept {
        return blending_;
    }

    const std::optional<render::culling_state>& render::material::culling() const noexcept {
        return culling_;
    }
    
    const render::depth_dynamic_state_opt& render::material::depth() const noexcept {
        return depth_;
    }

    const shader_ptr& render::material::shader() const noexcept {
        return shader_;
    }

    const const_buffer_ptr& render::material::constants() const noexcept {
        return constants_;
    }

    const render::sampler_block& render::material::samplers() const noexcept {
        return sampler_block_;
    }

    //
    // render::bind_vertex_buffers_command
    //

    render::bind_vertex_buffers_command& render::bind_vertex_buffers_command::add(
        const vertex_buffer_ptr& buffer,
        const vertex_attribs_ptr& attribs,
        std::size_t offset) noexcept
    {
        return bind(count_, buffer, attribs, offset);
    }

    render::bind_vertex_buffers_command& render::bind_vertex_buffers_command::bind(
        std::size_t index,
        const vertex_buffer_ptr& buffer,
        const vertex_attribs_ptr& attribs,
        std::size_t offset) noexcept
    {
        E2D_ASSERT(index < buffers_.size());
        count_ = math::max(index+1, count_);

        buffers_[index] = buffer;
        attribs_[index] = attribs;
        offsets_[index] = offset;
        return *this;
    }

    std::size_t render::bind_vertex_buffers_command::binding_count() const noexcept {
        return count_;
    }

    const vertex_buffer_ptr& render::bind_vertex_buffers_command::vertices(std::size_t index) const noexcept {
        E2D_ASSERT(index < count_);
        return buffers_[index];
    }

    const vertex_attribs_ptr& render::bind_vertex_buffers_command::attributes(std::size_t index) const noexcept {
        E2D_ASSERT(index < count_);
        return attribs_[index];
    }

    std::size_t render::bind_vertex_buffers_command::vertex_offset(std::size_t index) const noexcept {
        E2D_ASSERT(index < count_);
        return offsets_[index];
    }

    //
    // render::material_command
    //
    
    render::material_command::material_command(const material_cptr& value)
    : material_(value) {}

    const render::material_cptr& render::material_command::material() const noexcept {
        return material_;
    }

    //
    // render::scissor_command
    //

    render::scissor_command::scissor_command(const b2u& scissor_rect) noexcept
    : scissor_rect_(scissor_rect)
    , scissoring_(true) {}

    render::scissor_command& render::scissor_command::scissor_rect(const b2u& value) noexcept {
        scissor_rect_ = value;
        return *this;
    }

    render::scissor_command& render::scissor_command::scissoring(bool value) noexcept {
        scissoring_ = value;
        return *this;
    }

    const b2u& render::scissor_command::scissor_rect() const noexcept {
        return scissor_rect_;
    }

    bool render::scissor_command::scissoring() const noexcept {
        return scissoring_;
    }
        
    //
    // render::draw_command
    //
    
    render::draw_command& render::draw_command::constants(const const_buffer_ptr& value) noexcept {
        cbuffer_ = value;
        return *this;
    }

    render::draw_command& render::draw_command::topo(topology value) noexcept {
        topology_ = value;
        return *this;
    }

    render::draw_command& render::draw_command::vertex_range(u32 first, u32 count) noexcept {
        first_vertex_ = first;
        vertex_count_ = count;
        return *this;
    }

    render::draw_command& render::draw_command::first_vertex(u32 value) noexcept {
        first_vertex_ = value;
        return *this;
    }

    render::draw_command& render::draw_command::vertex_count(u32 value) noexcept {
        vertex_count_ = value;
        return *this;
    }

    u32 render::draw_command::first_vertex() const noexcept {
        return first_vertex_;
    }

    u32 render::draw_command::vertex_count() const noexcept {
        return vertex_count_;
    }

    render::topology render::draw_command::topo() const noexcept {
        return topology_;
    }
    
    const const_buffer_ptr& render::draw_command::constants() const noexcept {
        return cbuffer_;
    }

    //
    // render::draw_indexed_command
    //
    
    render::draw_indexed_command& render::draw_indexed_command::constants(const const_buffer_ptr& value) noexcept {
        cbuffer_ = value;
        return *this;
    }

    render::draw_indexed_command& render::draw_indexed_command::indices(const index_buffer_ptr& value) noexcept {
        index_buffer_ = value;
        return *this;
    }

    render::draw_indexed_command& render::draw_indexed_command::topo(topology value) noexcept {
        topology_ = value;
        return *this;
    }

    /*render::draw_indexed_command& render::draw_indexed_command::index_range(u32 count, size_t offset) noexcept {
        index_offset_ = offset;
        index_count_ = count;
        return *this;
    }*/

    render::draw_indexed_command& render::draw_indexed_command::index_offset(size_t offsetInBytes) noexcept {
        index_offset_ = offsetInBytes;
        return *this;
    }

    render::draw_indexed_command& render::draw_indexed_command::index_count(u32 value) noexcept {
        index_count_ = value;
        return *this;
    }
            
    size_t render::draw_indexed_command::index_offset() const noexcept {
        return index_offset_;
    }

    u32 render::draw_indexed_command::index_count() const noexcept {
        return index_count_;
    }

    render::topology render::draw_indexed_command::topo() const noexcept {
        return topology_;
    }

    const index_buffer_ptr& render::draw_indexed_command::indices() const noexcept {
        return index_buffer_;
    }
    
    const const_buffer_ptr& render::draw_indexed_command::constants() const noexcept {
        return cbuffer_;
    }

    //
    // render
    //

    render& render::execute(const command_value& command) {
        E2D_ASSERT(is_in_main_thread());
        E2D_ASSERT(!command.valueless_by_exception());
        stdex::visit(command_value_visitor(*this), command);
        return *this;
    }
}

namespace e2d
{
    bool operator==(const render::state_block& l, const render::state_block& r) noexcept {
        return l.depth() == r.depth()
            && l.stencil() == r.stencil()
            && l.culling() == r.culling()
            && l.blending() == r.blending();
    }

    bool operator!=(const render::state_block& l, const render::state_block& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::depth_state& l, const render::depth_state& r) noexcept {
        return l.test() == r.test()
            && l.write() == r.write()
            && (!l.test() || l.func() == r.func());
    }

    bool operator!=(const render::depth_state& l, const render::depth_state& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::stencil_state& l, const render::stencil_state& r) noexcept {
        if ( !l.test() ) {
            return !r.test();
        }
        return l.write() == r.write()
            && l.ref() == r.ref()
            && l.mask() == r.mask()
            && l.pass() == r.pass()
            && l.sfail() == r.sfail()
            && l.zfail() == r.zfail()
            && l.func() == r.func();
    }

    bool operator!=(const render::stencil_state& l, const render::stencil_state& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::culling_state& l, const render::culling_state& r) noexcept {
        if ( !l.enabled() ) {
            return !r.enabled();
        }
        return l.enabled() == r.enabled()
            && l.face() == r.face();
    }

    bool operator!=(const render::culling_state& l, const render::culling_state& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::blending_state& l, const render::blending_state& r) noexcept {
        if ( !l.enabled() ) {
            return !r.enabled();
        }
        return l.enabled() == r.enabled()
            && l.color_mask() == r.color_mask()
            && l.src_rgb_factor() == r.src_rgb_factor()
            && l.dst_rgb_factor() == r.dst_rgb_factor()
            && l.rgb_equation() == r.rgb_equation()
            && l.src_alpha_factor() == r.src_alpha_factor()
            && l.dst_alpha_factor() == r.dst_alpha_factor()
            && l.alpha_equation() == r.alpha_equation();
    }

    bool operator!=(const render::blending_state& l, const render::blending_state& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::sampler_state& l, const render::sampler_state& r) noexcept {
        return l.texture() == r.texture()
            && l.s_wrap() == r.s_wrap()
            && l.t_wrap() == r.t_wrap()
            && l.min_filter() == r.min_filter()
            && l.mag_filter() == r.mag_filter();
    }

    bool operator!=(const render::sampler_state& l, const render::sampler_state& r) noexcept {
        return !(l == r);
    }

    bool operator==(const render::sampler_block& l, const render::sampler_block& r) noexcept {
        if ( l.count() != r.count() ) {
            return false;
        }
        for ( size_t i = 0; i < l.count(); ++i ) {
            if ( l.name(i) != r.name(i) || l.sampler(i) != r.sampler(i) ) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const render::sampler_block& l, const render::sampler_block& r) noexcept {
        return !(l == r);
    }
    
    bool operator==(const render::material& l, const render::material& r) noexcept {
        return l.blending() == r.blending()
            && l.culling() == r.culling()
            && l.shader() == r.shader()
            && l.constants() == r.constants()
            && l.samplers() == r.samplers();
    }

    bool operator!=(const render::material& l, const render::material& r) noexcept {
        return !(l == r);
    }
}

namespace
{
    class render_schema_parsing_exception final : public exception {
        const char* what() const noexcept final {
            return "render scheme parsing exception";
        }
    };

    const char* render_schema_definitions_source = R"json({
        "render_pass_description" : {
            "type" : "object",
            "required" : [],
            "additionalProperties" : false,
            "properties" : {
                "viewport" : { "$ref": "#/common_definitions/b2" },
                "depth_range" : {
                    "type" : "object",
                    "additionalProperties" : false,
                    "properties" : {
                        "near" : { "type" : "number" },
                        "far" : { "type" : "number" }
                    }
                },
                "state_block" : { "$ref": "#/render_definitions/state_block" },

                "color_clear_value" : { "$ref" : "#/common_definitions/color" },
                "color_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                "color_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" },

                "depth_clear_value" : { "type" : "number" },
                "depth_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                "depth_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" },

                "stencil_clear_value" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                "stencil_load_op" : { "$ref" : "#/render_definitions/attachment_load_op" },
                "stencil_store_op" : { "$ref" : "#/render_definitions/attachment_store_op" }
            }
        },
        "state_block" : {
            "type" : "object",
            "additionalProperties" : false,
            "properties" : {
                "depth_state" : { "$ref": "#/render_definitions/depth_state" },
                "stencil_state" : { "$ref": "#/render_definitions/stencil_state" },
                "culling_state" : { "$ref": "#/render_definitions/culling_state" },
                "blending_state" : { "$ref": "#/render_definitions/blending_state" }
            }
        },
        "depth_state" : {
            "type" : "object",
            "additionalProperties" : false,
            "properties" : {
                "test" : { "type" : "boolean" },
                "write" : { "type" : "boolean" },
                "func" : { "$ref" : "#/render_definitions/compare_func" }
            }
        },
        "depth_dynamic_state" : {
            "type" : "object",
            "additionalProperties" : false,
            "properties" : {
                "test" : { "type" : "boolean" },
                "write" : { "type" : "boolean" }
            }
        },
        "stencil_state" : {
            "type" : "object",
            "required" : [ "test" ],
            "additionalProperties" : false,
            "properties" : {
                "test" : { "type" : "boolean" },
                "write" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                "func" : { "$ref" : "#/render_definitions/compare_func" },
                "ref" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                "mask" : { "type" : "integer", "minimum" : 0, "maximum": 255 },
                "pass" : { "$ref" : "#/render_definitions/stencil_op" },
                "sfail" : { "$ref" : "#/render_definitions/stencil_op" },
                "zfail" : { "$ref" : "#/render_definitions/stencil_op" }
            }
        },
        "culling_state" : {
            "type" : "object",
            "additionalProperties" : false,
            "properties" : {
                "enable" : { "type" : "boolean" },
                "face" : { "$ref" : "#/render_definitions/culling_face" }
            }
        },
        "blending_state" : {
            "type" : "object",
            "required" : [ "enable" ],
            "additionalProperties" : false,
            "properties" : {
                "enable" : { 
                    "type" : "boolean"
                },
                "color_mask" : {
                    "$ref" : "#/render_definitions/color_mask"
                },
                "src_factor" : {
                    "anyOf" : [{
                        "type" : "object",
                        "additionalProperties" : false,
                        "properties" : {
                            "rgb" : { "$ref" : "#/render_definitions/blending_factor" },
                            "alpha" : { "$ref" : "#/render_definitions/blending_factor" }
                        }
                    }, {
                        "$ref" : "#/render_definitions/blending_factor"
                    }]
                },
                "dst_factor" : {
                    "anyOf" : [{
                        "type" : "object",
                        "additionalProperties" : false,
                        "properties" : {
                            "rgb" : { "$ref" : "#/render_definitions/blending_factor" },
                            "alpha" : { "$ref" : "#/render_definitions/blending_factor" }
                        }
                    }, {
                        "$ref" : "#/render_definitions/blending_factor"
                    }]
                },
                "equation" : {
                    "anyOf" : [{
                        "type" : "object",
                        "additionalProperties" : false,
                        "properties" : {
                            "rgb" : { "$ref" : "#/render_definitions/blending_equation" },
                            "alpha" : { "$ref" : "#/render_definitions/blending_equation" }
                        }
                    }, {
                        "$ref" : "#/render_definitions/blending_equation"
                    }]
                }
            }
        },
        "property" : {
            "type" : "object",
            "required" : [ "name", "type" ],
            "additionalProperties" : false,
            "properties" : {
                "name" : { "$ref" : "#/common_definitions/name" },
                "type" : { "$ref" : "#/render_definitions/property_type" },
                "value" : { "$ref" : "#/render_definitions/property_value" }
            }
        },
        "property_type" : {
            "type" : "string",
            "enum" : [
                "f32",
                "v2f", "v3f", "v4f",
                "m2f", "m3f", "m4f"
            ]
        },
        "property_value" : {
            "anyOf": [
                { "type" : "number" },
                { "$ref" : "#/common_definitions/v2" },
                { "$ref" : "#/common_definitions/v3" },
                { "$ref" : "#/common_definitions/v4" },
                { "$ref" : "#/common_definitions/m2" },
                { "$ref" : "#/common_definitions/m3" },
                { "$ref" : "#/common_definitions/m4" }
            ]
        },
        "stencil_op" : {
            "type" : "string",
            "enum" : [
                "keep",
                "zero",
                "replace",
                "incr",
                "incr_wrap",
                "decr",
                "decr_wrap",
                "invert"
            ]
        },
        "compare_func" : {
            "type" : "string",
            "enum" : [
                "never",
                "less",
                "lequal",
                "greater",
                "gequal",
                "equal",
                "notequal",
                "always"
            ]
        },
        "culling_face" : {
            "type" : "string",
            "enum" : [
                "back",
                "front",
                "back_and_front"
            ]
        },
        "blending_factor" : {
            "type" : "string",
            "enum" : [
                "zero",
                "one",
                "src_color",
                "one_minus_src_color",
                "dst_color",
                "one_minus_dst_color",
                "src_alpha",
                "one_minus_src_alpha",
                "dst_alpha",
                "one_minus_dst_alpha",
                "constant_color",
                "one_minus_constant_color",
                "constant_alpha",
                "one_minus_constant_alpha",
                "src_alpha_saturate"
            ]
        },
        "blending_equation" : {
            "type" : "string",
            "enum" : [
                "add",
                "subtract",
                "reverse_subtract"
            ]
        },
        "blending_color_mask" : {
            "type" : "string",
            "enum" : [
                "none",
                "r",
                "g",
                "b",
                "a",
                "rg",
                "rb",
                "ra",
                "gb",
                "ga",
                "ba",
                "rgb",
                "rga",
                "rba",
                "gba",
                "rgba"
            ]
        },
        "sampler_wrap" : {
            "type" : "string",
            "enum" : [
                "clamp",
                "repeat",
                "mirror"
            ]
        },
        "sampler_filter" : {
            "type" : "string",
            "enum" : [
                "nearest",
                "linear"
            ]
        },
        "attachment_load_op" : {
            "type" : "string",
            "enum" : [
                "load",
                "clear",
                "invalidate"
            ]
        },
        "attachment_store_op" : {
            "type" : "string",
            "enum" : [
                "store",
                "discard"
            ]
        }
    })json";
    
    const rapidjson::Value& render_schema_definitions() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::Document> defs_doc;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !defs_doc ) {
            rapidjson::Document doc;
            if ( doc.Parse(render_schema_definitions_source).HasParseError() ) {
                throw render_schema_parsing_exception();
            }
            defs_doc = std::make_unique<rapidjson::Document>(std::move(doc));
        }

        return *defs_doc;
    }
}

namespace e2d
{   
    render::batchr::batch_::batch_(const material &mtr)
    : mtr(mtr) {}
    
    size_t render::batchr::buffer_::available(size_t align) const noexcept {
        size_t off = math::align_ceil(offset, align);
        return off < content.size()
             ? math::align_ceil(content.size() - off, align)
             : 0;
    }
    
    buffer_view render::batchr::buffer_::actual_data() const noexcept {
        static_assert(vertex_buffer_size_ % gpu_buffer_alignment == 0, "");
        static_assert(index_buffer_size_ % gpu_buffer_alignment == 0, "");
        return buffer_view(content.data(), math::align_ceil(offset, gpu_buffer_alignment));
    }

    render::batchr::batchr(debug& d, render& r)
    : debug_(d)
    , render_(r) {}

    render::batchr::~batchr() noexcept = default;

    render::batchr::batch_& render::batchr::append_batch_(
        const material& mtr,
        topology topo,
        vertex_attribs_ptr attribs,
        size_t vert_stride,
        size_t vertex_count,
        size_t min_ib_size,
        const std::optional<b2u>& scissor)
    {
        const size_t min_vb_size = vertex_count * vert_stride;
        constexpr size_t max_vertex_count = std::numeric_limits<batch_index_t>::max();
        
        if ( vertex_count > max_vertex_count ||
             min_vb_size > vertex_buffer_size_ ||
             min_ib_size > index_buffer_size_ )
        {
            throw std::runtime_error("too big for batching");
        }

        // try to reuse last batch
        if ( batches_.size() ) {
            auto& last = batches_.back();
            auto& vb = vertex_buffers_[last.vb_index];
            auto& ib = index_buffers_[last.ib_index];
            size_t vert_offset = (vb.offset + vert_stride-1) / vert_stride;

            if ( last.mtr == mtr &&
                 last.attribs == attribs &&
                 last.topo == topo &&
                 vert_offset + vertex_count < max_vertex_count &&
                 vb.available(vert_stride) >= min_vb_size &&
                 ib.available(index_stride_) >= min_ib_size &&
                 last.scissor_test == scissor.has_value() &&
                 (!last.scissor_test || last.scissor_rect == scissor.value()) )
            {
                ++curr_stats_.num_appended_batches;
                return last;
            }
        }

        // create new batch
        ++curr_stats_.num_created_batches;
        batch_& result = batches_.emplace_back(mtr);

        if ( vertex_buffers_.empty() ||
             vertex_buffers_.back().available(vert_stride) < min_vb_size ||
             ((vertex_buffers_.back().offset + vert_stride-1) / vert_stride) + vertex_count >= max_vertex_count )
        {
            auto& vb = vertex_buffers_.emplace_back();
            vb.content.resize(vertex_buffer_size_);
        }

        if ( index_buffers_.empty() ||
             index_buffers_.back().available(index_stride_) < min_ib_size )
        {
            auto& ib = index_buffers_.emplace_back();
            ib.content.resize(index_buffer_size_);
        }
        
        if ( scissor.has_value() ) {
            result.scissor_test = true;
            result.scissor_rect = scissor.value();
        }

        result.attribs = attribs;
        result.topo = topo;
        result.vb_index = u32(vertex_buffers_.size()-1);
        result.ib_index = u32(index_buffers_.size()-1);
        result.idx_offset = index_buffers_.back().offset;

        return result;
    }

    void render::batchr::flush() {
        if ( !dirty_ ) {
            return;
        }
        dirty_ = false;

        vertex_attribs_ptr curr_attribs;
        shader_ptr curr_shader;
        u32 curr_vb_index = ~0u;

        std::vector<vertex_buffer_ptr> vert_buffers(vertex_buffers_.size());
        for ( std::size_t i = 0; i  < vertex_buffers_.size(); ++i ) {
            vert_buffers[i] = render_.create_vertex_buffer(
                vertex_buffers_[i].actual_data(),
                vertex_buffer::usage::static_draw);
        }

        std::vector<index_buffer_ptr> index_buffers(index_buffers_.size());
        for ( std::size_t i = 0; i < index_buffers_.size(); ++i ) {
            index_buffers[i] = render_.create_index_buffer(
                index_buffers_[i].actual_data(),
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::static_draw);
        }

        for ( auto& batch : batches_ ) {
            if ( curr_vb_index != batch.vb_index ||
                 curr_attribs != batch.attribs ||
                 curr_shader != batch.mtr.shader() )
            {
                curr_vb_index = batch.vb_index;
                curr_attribs = batch.attribs;
                curr_shader = batch.mtr.shader();

                render_.execute(render::bind_vertex_buffers_command()
                    .bind(0, vert_buffers[curr_vb_index], curr_attribs));
            }
            
            render_.set_material(batch.mtr);

            render_.execute(render::scissor_command()
                .scissor_rect(batch.scissor_rect)
                .scissoring(batch.scissor_test));

            render_.execute(render::draw_indexed_command()
                .index_count(batch.idx_count)
                .index_offset(batch.idx_offset)
                .indices(index_buffers[batch.ib_index])
                .topo(batch.topo));
        }

        // TODO: optimize
        vertex_buffers_.clear();
        index_buffers_.clear();
        batches_.clear();

        ++curr_stats_.flush_counter;
    }
    
    void render::batchr::on_present_() {
        flush();
        last_stats_ = curr_stats_;
        curr_stats_ = {};
    }
}

namespace e2d::json_utils
{
    void add_render_schema_definitions(rapidjson::Document& schema) {
        schema.AddMember(
            "render_definitions",
            rapidjson::Value(
                render_schema_definitions(),
                schema.GetAllocator()).Move(),
            schema.GetAllocator());
    }
}

namespace e2d::json_utils
{
    bool try_parse_value(const rapidjson::Value& root, render::topology& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::topology::x; return true; }
        DEFINE_IF(triangles);
        DEFINE_IF(triangles_strip);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::stencil_op& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::stencil_op::x; return true; }
        DEFINE_IF(keep);
        DEFINE_IF(zero);
        DEFINE_IF(replace);
        DEFINE_IF(incr);
        DEFINE_IF(incr_wrap);
        DEFINE_IF(decr);
        DEFINE_IF(decr_wrap);
        DEFINE_IF(invert);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::compare_func& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::compare_func::x; return true; }
        DEFINE_IF(never);
        DEFINE_IF(less);
        DEFINE_IF(lequal);
        DEFINE_IF(greater);
        DEFINE_IF(gequal);
        DEFINE_IF(equal);
        DEFINE_IF(notequal);
        DEFINE_IF(always);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::culling_face& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::culling_face::x; return true; }
        DEFINE_IF(back);
        DEFINE_IF(front);
        DEFINE_IF(back_and_front);
    #undef DEFINE_IF
        return false;
    }
    
    bool try_parse_value(const rapidjson::Value& root, render::blending_factor& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::blending_factor::x; return true; }
        DEFINE_IF(zero);
        DEFINE_IF(one);
        DEFINE_IF(src_color);
        DEFINE_IF(one_minus_src_color);
        DEFINE_IF(dst_color);
        DEFINE_IF(one_minus_dst_color);
        DEFINE_IF(src_alpha);
        DEFINE_IF(one_minus_src_alpha);
        DEFINE_IF(dst_alpha);
        DEFINE_IF(one_minus_dst_alpha);
        DEFINE_IF(constant_color);
        DEFINE_IF(one_minus_constant_color);
        DEFINE_IF(constant_alpha);
        DEFINE_IF(one_minus_constant_alpha);
        DEFINE_IF(src_alpha_saturate);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::blending_equation& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::blending_equation::x; return true; }
        DEFINE_IF(add);
        DEFINE_IF(subtract);
        DEFINE_IF(reverse_subtract);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::blending_color_mask& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::blending_color_mask::x; return true; }
        DEFINE_IF(none);
        DEFINE_IF(r);
        DEFINE_IF(g);
        DEFINE_IF(b);
        DEFINE_IF(a);
        DEFINE_IF(rg);
        DEFINE_IF(rb);
        DEFINE_IF(ra);
        DEFINE_IF(gb);
        DEFINE_IF(ga);
        DEFINE_IF(ba);
        DEFINE_IF(rgb);
        DEFINE_IF(rga);
        DEFINE_IF(rba);
        DEFINE_IF(gba);
        DEFINE_IF(rgba);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::sampler_wrap& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::sampler_wrap::x; return true; }
        DEFINE_IF(clamp);
        DEFINE_IF(repeat);
        DEFINE_IF(mirror);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::sampler_min_filter& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::sampler_min_filter::x; return true; }
        DEFINE_IF(nearest);
        DEFINE_IF(linear);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::sampler_mag_filter& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x,y) if ( str == #x ) { v = render::sampler_mag_filter::y; return true; }
        DEFINE_IF(nearest, nearest);
        DEFINE_IF(linear, linear);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::attachment_load_op& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::attachment_load_op::x; return true; }
        DEFINE_IF(load);
        DEFINE_IF(clear);
        DEFINE_IF(invalidate);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::attachment_store_op& v) noexcept {
        E2D_ASSERT(root.IsString());
        str_view str = root.GetString();
    #define DEFINE_IF(x) if ( str == #x ) { v = render::attachment_store_op::x; return true; }
        DEFINE_IF(store);
        DEFINE_IF(discard);
    #undef DEFINE_IF
        return false;
    }

    bool try_parse_value(const rapidjson::Value& root, render::depth_state& depth) noexcept {
        E2D_ASSERT(root.IsObject());

        if ( root.HasMember("test") ) {
            E2D_ASSERT(root["test"].IsBool());
            depth.test(root["test"].GetBool());
        }

        if ( root.HasMember("write") ) {
            E2D_ASSERT(root["write"].IsBool());
            depth.write(root["write"].GetBool());
        }

        if ( root.HasMember("func") ) {
            render::compare_func func = depth.func();
            if ( !try_parse_value(root["func"], func) ) {
                E2D_ASSERT_MSG(false, "unexpected depth state func");
                return false;
            }

            depth.func(func);
        }
        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::depth_dynamic_state& depth) noexcept {
        E2D_ASSERT(root.IsObject());
        
        if ( root.HasMember("test") ) {
            E2D_ASSERT(root["test"].IsBool());
            depth.test(root["test"].GetBool());
        }

        if ( root.HasMember("write") ) {
            E2D_ASSERT(root["write"].IsBool());
            depth.write(root["write"].GetBool());
        }

        return true;
    }
    
    bool try_parse_value(const rapidjson::Value& root, render::stencil_state& stencil) noexcept {
        E2D_ASSERT(root.IsObject());
        
        if ( root.HasMember("test") ) {
            E2D_ASSERT(root["test"].IsBool());
            stencil.test(root["test"].GetBool());
        }

        if ( root.HasMember("write") ) {
            E2D_ASSERT(root["write"].IsUint() && root["write"].GetUint() <= 255);
            stencil.write(math::numeric_cast<u8>(
                root["write"].GetUint()));
        }

        if ( root.HasMember("func") ) {
            render::compare_func func = stencil.func();
            if ( !try_parse_value(root["func"], func) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state func");
                return false;
            }

            stencil.func(
                func,
                stencil.ref(),
                stencil.mask());
        }

        if ( root.HasMember("ref") ) {
            E2D_ASSERT(root["ref"].IsUint() && root["ref"].GetUint() <= 255);

            stencil.func(
                stencil.func(),
                math::numeric_cast<u8>(root["ref"].GetUint()),
                stencil.mask());
        }

        if ( root.HasMember("mask") ) {
            E2D_ASSERT(root["mask"].IsUint() && root["mask"].GetUint() <= 255);

            stencil.func(
                stencil.func(),
                stencil.ref(),
                math::numeric_cast<u8>(root["mask"].GetUint()));
        }

        if ( root.HasMember("pass") ) {
            render::stencil_op op = stencil.pass();
            if ( !try_parse_value(root["pass"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state pass");
                return false;
            }

            stencil.op(
                op,
                stencil.sfail(),
                stencil.zfail());
        }

        if ( root.HasMember("sfail") ) {
            render::stencil_op op = stencil.sfail();
            if ( !try_parse_value(root["sfail"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state sfail");
                return false;
            }

            stencil.op(
                stencil.pass(),
                op,
                stencil.zfail());
        }

        if ( root.HasMember("zfail") ) {
            render::stencil_op op = stencil.zfail();
            if ( !try_parse_value(root["zfail"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil state zfail");
                return false;
            }

            stencil.op(
                stencil.pass(),
                stencil.sfail(),
                op);
        }

        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::culling_state& culling) noexcept {
        E2D_ASSERT(root.IsObject());

        if ( root.HasMember("enable") ) {
            E2D_ASSERT(root["enable"].IsBool());
            culling.enable(root["enable"].GetBool());
        }

        if ( root.HasMember("face") ) {
            render::culling_face face = culling.face();
            if ( !try_parse_value(root["face"], face) ) {
                E2D_ASSERT_MSG(false, "unexpected culling state face");
                return false;
            }

            culling.face(face);
        }
        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::blending_state& blending) noexcept {
        E2D_ASSERT(root.IsObject());

        if ( root.HasMember("enable") ) {
            E2D_ASSERT(root["enable"].IsBool());
            blending.enable(root["enable"].GetBool());
        }

        if ( root.HasMember("color_mask") ) {
            render::blending_color_mask mask = blending.color_mask();
            if ( !try_parse_value(root["color_mask"], mask) ) {
                E2D_ASSERT_MSG(false, "unexpected blending state color mask");
                return false;
            }

            blending.color_mask(mask);
        }

        if ( root.HasMember("src_factor") ) {
            if ( root["src_factor"].IsString() ) {
                render::blending_factor factor = blending.src_rgb_factor();
                if ( !try_parse_value(root["src_factor"], factor) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                    return false;
                }
                blending.src_factor(factor);
            } else if ( root["src_factor"].IsObject() ) {
                const auto& root_src_factor = root["src_factor"];

                if ( root_src_factor.HasMember("rgb") ) {
                    render::blending_factor factor = blending.src_rgb_factor();
                    if ( !try_parse_value(root_src_factor["rgb"], factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                        return false;
                    }
                    blending.src_rgb_factor(factor);
                }

                if ( root_src_factor.HasMember("alpha") ) {
                    render::blending_factor factor = blending.src_alpha_factor();
                    if ( !try_parse_value(root_src_factor["alpha"], factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state src factor");
                        return false;
                    }
                    blending.src_alpha_factor(factor);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state src factor");
            }
        }

        if ( root.HasMember("dst_factor") ) {
            if ( root["dst_factor"].IsString() ) {
                render::blending_factor factor = blending.dst_rgb_factor();
                if ( !try_parse_value(root["dst_factor"], factor) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                    return false;
                }
                blending.dst_factor(factor);
            } else if ( root["dst_factor"].IsObject() ) {
                const auto& root_dst_factor = root["dst_factor"];

                if ( root_dst_factor.HasMember("rgb") ) {
                    render::blending_factor factor = blending.dst_rgb_factor();
                    if ( !try_parse_value(root_dst_factor["rgb"], factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                        return false;
                    }
                    blending.dst_rgb_factor(factor);
                }

                if ( root_dst_factor.HasMember("alpha") ) {
                    render::blending_factor factor = blending.dst_alpha_factor();
                    if ( !try_parse_value(root_dst_factor["alpha"], factor) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
                        return false;
                    }
                    blending.dst_alpha_factor(factor);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state dst factor");
            }
        }

        if ( root.HasMember("equation") ) {
            if ( root["equation"].IsString() ) {
                render::blending_equation equation = blending.rgb_equation();
                if ( !try_parse_value(root["equation"], equation) ) {
                    E2D_ASSERT_MSG(false, "unexpected blending state equation");
                    return false;
                }
                blending.equation(equation);
            } else if ( root["equation"].IsObject() ) {
                const auto& root_equation = root["equation"];

                if ( root_equation.HasMember("rgb") ) {
                    render::blending_equation equation = blending.rgb_equation();
                    if ( !try_parse_value(root_equation["rgb"], equation) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state equation");
                        return false;
                    }
                    blending.rgb_equation(equation);
                }

                if ( root_equation.HasMember("alpha") ) {
                    render::blending_equation equation = blending.alpha_equation();
                    if ( !try_parse_value(root_equation["alpha"], equation) ) {
                        E2D_ASSERT_MSG(false, "unexpected blending state equation");
                        return false;
                    }
                    blending.alpha_equation(equation);
                }
            } else {
                E2D_ASSERT_MSG(false, "unexpected blending state equation");
            }
        }
        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::state_block& block) noexcept {
        E2D_ASSERT(root.IsObject());

        if ( root.HasMember("depth_state") ) {
            if ( !try_parse_value(root["depth_state"], block.depth()) ) {
                return false;
            }
        }

        if ( root.HasMember("stencil_state") ) {
            if ( !try_parse_value(root["stencil_state"], block.stencil()) ) {
                return false;
            }
        }

        if ( root.HasMember("culling_state") ) {
            if ( !try_parse_value(root["culling_state"], block.culling()) ) {
                return false;
            }
        }

        if ( root.HasMember("blending_state") ) {
            if ( !try_parse_value(root["blending_state"], block.blending()) ) {
                return false;
            }
        }

        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::property_map& props) noexcept {
        E2D_ASSERT(root.IsArray());

        for ( rapidjson::SizeType i = 0; i < root.Size(); ++i ) {
            E2D_ASSERT(root[i].IsObject());
            const auto& property_json = root[i];

            E2D_ASSERT(property_json.HasMember("name") && property_json["name"].IsString());
            E2D_ASSERT(property_json.HasMember("type") && property_json["type"].IsString());

        #define DEFINE_CASE(x)\
            if ( 0 == std::strcmp(property_json["type"].GetString(), #x) ) {\
                x value;\
                if ( property_json.HasMember("value") ) {\
                    if ( !try_parse_value(property_json["value"], value) ) {\
                        E2D_ASSERT_MSG(false, "unexpected property value");\
                        return false;\
                    }\
                }\
                props.property(property_json["name"].GetString(), value);\
                continue;\
            }

            DEFINE_CASE(f32);

            DEFINE_CASE(v2f);
            DEFINE_CASE(v3f);
            DEFINE_CASE(v4f);

            DEFINE_CASE(m2f);
            DEFINE_CASE(m3f);
            DEFINE_CASE(m4f);
        #undef DEFINE_CASE

            E2D_ASSERT_MSG(false, "unexpected property type");
            return false;
        }
        return true;
    }

    bool try_parse_value(const rapidjson::Value& root, render::renderpass_desc& pass) noexcept {
        E2D_ASSERT(root.IsObject());

        if ( root.HasMember("viewport") ) {
            b2u viewport;
            if ( !try_parse_value(root["viewport"], viewport) ) {
                E2D_ASSERT_MSG(false, "unexpected viewport value");
                return false;
            }
            pass.viewport(viewport);
        }

        if ( root.HasMember("depth_range") ) {
            E2D_ASSERT(root["range"].IsObject());
            const auto& root_range = root["range"];

            if ( root_range.HasMember("near") ) {
                E2D_ASSERT(root_range["near"].IsNumber());
                pass.depth_range(v2f(
                    root_range["near"].GetFloat(),
                    pass.depth_range().y));
            }

            if ( root_range.HasMember("far") ) {
                E2D_ASSERT(root_range["far"].IsNumber());
                pass.depth_range(v2f(
                    pass.depth_range().x,
                    root_range["far"].GetFloat()));
            }
        }

        if ( root.HasMember("state_block") ) {
            render::state_block states;
            if ( !try_parse_value(root["state_block"], states) ) {
                return false;
            }
            pass.states(states);
        }

        if ( root.HasMember("color_load_op") ) {
            render::attachment_load_op op = pass.color_load_op();
            if ( !try_parse_value(root["color_load_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected color load operation");
                return false;
            }
            pass.color_load_op(op);
        }

        if ( pass.color_load_op() == render::attachment_load_op::clear
            &&root.HasMember("color_clear_value") )
        {
            color col;
            if ( !try_parse_value(root["color_clear_value"], col) ) {
                E2D_ASSERT_MSG(false, "unexpected color clear value");
                return false;
            }
            pass.color_clear(col);
        }

        if ( root.HasMember("color_store_op") ) {
            render::attachment_store_op op = pass.color_store_op();
            if ( !try_parse_value(root["color_store_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected color store operation");
                return false;
            }
            pass.color_store_op(op);
        }

        if ( root.HasMember("depth_load_op") ) {
            render::attachment_load_op op = pass.depth_load_op();
            if ( !try_parse_value(root["depth_load_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected depth load operation");
                return false;
            }
            pass.depth_load_op(op);
        }

        if ( pass.depth_load_op() == render::attachment_load_op::clear
            && root.HasMember("depth_clear_value") )
        {
            const auto& root_d = root["depth_clear_value"];
            E2D_ASSERT(root_d.IsNumber());
            pass.depth_clear(root_d.GetFloat());
        }

        if ( root.HasMember("depth_store_op") ) {
            render::attachment_store_op op = pass.depth_store_op();
            if ( !try_parse_value(root["depth_store_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected depth store operation");
                return false;
            }
            pass.depth_store_op(op);
        }

        if ( root.HasMember("stencil_load_op") ) {
            render::attachment_load_op op = pass.stencil_load_op();
            if ( !try_parse_value(root["stencil_load_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil load operation");
                return false;
            }
            pass.stencil_load_op(op);
        }

        if ( pass.stencil_load_op() == render::attachment_load_op::clear
            && root.HasMember("stencil_clear_value") )
        {
            const auto& root_s = root["stencil_clear_value"];
            E2D_ASSERT(root_s.IsInt());
            pass.stencil_clear(math::numeric_cast<u8>(root_s.GetInt()));
        }

        if ( root.HasMember("stencil_store_op") ) {
            render::attachment_store_op op = pass.stencil_store_op();
            if ( !try_parse_value(root["stencil_store_op"], op) ) {
                E2D_ASSERT_MSG(false, "unexpected stencil store operation");
                return false;
            }
            pass.stencil_store_op(op);
        }

        return true;
    }
}

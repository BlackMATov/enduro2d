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
        u32 bytes_per_block;
        bool color;
        bool depth;
        bool stencil;
        pixel_declaration::pixel_type type;
        bool compressed;
        v2u block_size;
    };

    const pixel_type_description pixel_type_descriptions[] = {
        {"depth16",           2, false, true,  false, pixel_declaration::pixel_type::depth16,          false, v2u(1)},
        {"depth16_stencil8",  0, false, true,  true,  pixel_declaration::pixel_type::depth16_stencil8, false, v2u(1)},
        {"depth24",           3, false, true,  false, pixel_declaration::pixel_type::depth24,          false, v2u(1)},
        {"depth24_stencil8",  4, false, true,  true,  pixel_declaration::pixel_type::depth24_stencil8, false, v2u(1)},
        {"depth32",           4, false, true,  false, pixel_declaration::pixel_type::depth32,          false, v2u(1)},
        {"depth32_stencil8",  0, false, true,  true,  pixel_declaration::pixel_type::depth32_stencil8, false, v2u(1)},
        
        {"a8",                1, true,  false, false, pixel_declaration::pixel_type::a8,               false, v2u(1,1)},
        {"l8",                1, true,  false, false, pixel_declaration::pixel_type::l8,               false, v2u(1,1)},
        {"la8",               2, true,  false, false, pixel_declaration::pixel_type::la8,              false, v2u(1,1)},
        {"rgb8",              3, true,  false, false, pixel_declaration::pixel_type::rgb8,             false, v2u(1,1)},
        {"rgba8",             4, true,  false, false, pixel_declaration::pixel_type::rgba8,            false, v2u(1,1)},
        
        {"rgba_dxt1",         8, true,  false, false, pixel_declaration::pixel_type::rgba_dxt1,        true,  v2u(4,4)},
        {"rgba_dxt3",        16, true,  false, false, pixel_declaration::pixel_type::rgba_dxt3,        true,  v2u(4,4)},
        {"rgba_dxt5",        16, true,  false, false, pixel_declaration::pixel_type::rgba_dxt5,        true,  v2u(4,4)},

        {"rgb_etc1",          8, true,  false, false, pixel_declaration::pixel_type::rgb_etc1,         true,  v2u(4,4)},
        {"rgb_etc2",          8, true,  false, false, pixel_declaration::pixel_type::rgb_etc2,         true,  v2u(4,4)},
        {"rgba_etc2",        16, true,  false, false, pixel_declaration::pixel_type::rgba_etc2,        true,  v2u(4,4)},
        {"rgb_a1_etc2",       8, true,  false, false, pixel_declaration::pixel_type::rgb_a1_etc2,      true,  v2u(4,4)},

        {"rgba_astc4x4",     16, true,  false, false, pixel_declaration::pixel_type::rgba_astc4x4,     true,  v2u(4,4)},
        {"rgba_astc5x5",     16, true,  false, false, pixel_declaration::pixel_type::rgba_astc5x5,     true,  v2u(5,5)},
        {"rgba_astc6x6",     16, true,  false, false, pixel_declaration::pixel_type::rgba_astc6x6,     true,  v2u(6,6)},
        {"rgba_astc8x8",     16, true,  false, false, pixel_declaration::pixel_type::rgba_astc8x8,     true,  v2u(8,8)},
        {"rgba_astc10x10",   16, true,  false, false, pixel_declaration::pixel_type::rgba_astc10x10,   true,  v2u(10,10)},
        {"rgba_astc12x12",   16, true,  false, false, pixel_declaration::pixel_type::rgba_astc12x12,   true,  v2u(12,12)},

        {"rgb_pvrtc2",        8, true,  false, false, pixel_declaration::pixel_type::rgb_pvrtc2,       true,  v2u(8,4)},
        {"rgb_pvrtc4",        8, true,  false, false, pixel_declaration::pixel_type::rgb_pvrtc4,       true,  v2u(4,4)},
        {"rgba_pvrtc2",       8, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc2,      true,  v2u(8,4)},
        {"rgba_pvrtc4",       8, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc4,      true,  v2u(4,4)},

        {"rgba_pvrtc2_v2",    8, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc2_v2,   true,  v2u(8,4)},
        {"rgba_pvrtc4_v2",    8, true,  false, false, pixel_declaration::pixel_type::rgba_pvrtc4_v2,   true,  v2u(4,4)}
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
    
    v2u pixel_declaration::block_size() const noexcept {
        return get_pixel_type_description(type_).block_size;
    }

    std::size_t pixel_declaration::bytes_per_block() const noexcept {
        return get_pixel_type_description(type_).bytes_per_block;
    }

    std::size_t pixel_declaration::data_size_for_dimension(v2u dim) const noexcept {
        const v2u bs = block_size();
        const std::size_t bpb = bytes_per_block();
        return bs.x > 0 && bs.y > 0 && bpb > 0
            ? bpb * ((dim.x + bs.x - 1u) / bs.x) * ((dim.y + bs.y - 1u) / bs.y)
            : 0u;
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
    
    render::culling_state& render::culling_state::mode(culling_mode mode) noexcept {
        mode_ = mode;
        return *this;
    }
    
    render::culling_state& render::culling_state::enable(bool value) noexcept {
        enabled_ = value;
        return *this;
    }

    render::culling_face render::culling_state::face() const noexcept {
        return face_;
    }
    
    render::culling_mode render::culling_state::mode() const noexcept {
        return mode_;
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
    
    render::sampler_state& render::sampler_state::wrap(sampler_wrap s, sampler_wrap t) noexcept {
        s_wrap(s);
        t_wrap(t);
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

    /*render::renderpass_desc& render::renderpass_desc::color_invalidate() noexcept {
        color_.load_op = attachment_load_op::invalidate;
        return *this;
    }*/

    render::renderpass_desc& render::renderpass_desc::color_store() noexcept {
        color_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::color_discard() noexcept {
        color_.store_op = attachment_store_op::discard;
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

    /*render::renderpass_desc& render::renderpass_desc::depth_invalidate() noexcept {
        depth_.load_op = attachment_load_op::invalidate;
        return *this;
    }*/

    render::renderpass_desc& render::renderpass_desc::depth_store() noexcept {
        depth_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::depth_discard() noexcept {
        depth_.store_op = attachment_store_op::discard;
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

    /*render::renderpass_desc& render::renderpass_desc::stencil_invalidate() noexcept {
        stencil_.load_op = attachment_load_op::invalidate;
        return *this;
    }*/

    render::renderpass_desc& render::renderpass_desc::stencil_store() noexcept {
        stencil_.store_op = attachment_store_op::store;
        return *this;
    }

    render::renderpass_desc& render::renderpass_desc::stencil_discard() noexcept {
        stencil_.store_op = attachment_store_op::discard;
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
        E2D_ASSERT(scissoring_);
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
        std::visit(command_value_visitor(*this), command);
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
            && l.face() == r.face()
            && l.mode() == r.mode();
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

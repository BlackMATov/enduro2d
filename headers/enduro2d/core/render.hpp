/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_core.hpp"

namespace e2d
{
    class render;
    class shader;
    class texture;
    class index_buffer;
    class vertex_buffer;
    class render_target;
    class pixel_declaration;
    class index_declaration;
    class vertex_declaration;

    using shader_ptr = std::shared_ptr<shader>;
    using texture_ptr = std::shared_ptr<texture>;
    using index_buffer_ptr = std::shared_ptr<index_buffer>;
    using vertex_buffer_ptr = std::shared_ptr<vertex_buffer>;
    using render_target_ptr = std::shared_ptr<render_target>;

    //
    // bad_render_operation
    //

    class bad_render_operation final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad render operation";
        }
    };

    //
    // pixel_declaration
    //

    class pixel_declaration final {
    public:
        enum class pixel_type : u8 {
            depth16,
            depth24,
            depth24_stencil8,

            a8,
            l8,
            la8,
            rgb8,
            rgba8,

            rgba_dxt1,
            rgba_dxt3,
            rgba_dxt5,

            rgb_etc1,
            rgb_etc2,
            rgba_etc2,
            rgb_a1_etc2,

            rgba_astc4x4,
            rgba_astc5x5,
            rgba_astc6x6,
            rgba_astc8x8,
            rgba_astc10x10,
            rgba_astc12x12,

            rgb_pvrtc2,
            rgb_pvrtc4,
            rgba_pvrtc2,
            rgba_pvrtc4,

            rgba_pvrtc2_v2,
            rgba_pvrtc4_v2
        };
        static const char* pixel_type_to_cstr(pixel_type pt) noexcept;
    public:
        pixel_declaration() = default;
        ~pixel_declaration() noexcept = default;

        pixel_declaration(const pixel_declaration&) noexcept = default;
        pixel_declaration& operator=(const pixel_declaration&) noexcept = default;

        pixel_declaration(pixel_type type) noexcept;

        pixel_type type() const noexcept;
        bool is_color() const noexcept;
        bool is_depth() const noexcept;
        bool is_stencil() const noexcept;
        bool is_compressed() const noexcept;
        v2u block_size() const noexcept;
        std::size_t bytes_per_block() const noexcept;
        std::size_t data_size_for_dimension(v2u dim) const noexcept;
    private:
        pixel_type type_ = pixel_type::rgba8;
    };

    bool operator==(
        const pixel_declaration& l,
        const pixel_declaration& r) noexcept;
    bool operator!=(
        const pixel_declaration& l,
        const pixel_declaration& r) noexcept;

    //
    // index_declaration
    //

    class index_declaration final {
    public:
        enum class index_type : u8 {
            unsigned_short,
            unsigned_int
        };
        static const char* index_type_to_cstr(index_type it) noexcept;
    public:
        index_declaration() = default;
        ~index_declaration() noexcept = default;

        index_declaration(const index_declaration&) noexcept = default;
        index_declaration& operator=(const index_declaration&) noexcept = default;

        index_declaration(index_type type) noexcept;

        index_type type() const noexcept;
        std::size_t bytes_per_index() const noexcept;
    private:
        index_type type_ = index_type::unsigned_short;
    };

    bool operator==(
        const index_declaration& l,
        const index_declaration& r) noexcept;
    bool operator!=(
        const index_declaration& l,
        const index_declaration& r) noexcept;

    //
    // vertex_declaration
    //

    class vertex_declaration final {
    public:
        enum class attribute_type : u8 {
            signed_byte,
            unsigned_byte,
            signed_short,
            unsigned_short,
            floating_point
        };

        class attribute_info final {
        public:
            std::size_t stride = 0;
            str_hash name;
            u8 rows = 0;
            u8 columns = 0;
            attribute_type type = attribute_type::floating_point;
            bool normalized = false;
        public:
            attribute_info() = default;
            ~attribute_info() noexcept = default;

            attribute_info(const attribute_info&) noexcept = default;
            attribute_info& operator=(const attribute_info&) noexcept = default;

            attribute_info(
                std::size_t stride,
                str_hash name,
                u8 rows,
                u8 columns,
                attribute_type type,
                bool normalized) noexcept;

            std::size_t row_size() const noexcept;
        };
    public:
        vertex_declaration() = default;
        ~vertex_declaration() noexcept = default;

        vertex_declaration(const vertex_declaration&) noexcept = default;
        vertex_declaration& operator=(const vertex_declaration&) noexcept = default;

        template < typename T >
        vertex_declaration& add_attribute(str_hash name) noexcept;
        vertex_declaration& normalized() noexcept;

        vertex_declaration& skip_bytes(
            std::size_t bytes) noexcept;

        vertex_declaration& add_attribute(
            str_hash name,
            u8 rows,
            u8 columns,
            attribute_type type,
            bool normalized) noexcept;

        const attribute_info& attribute(std::size_t index) const noexcept;
        std::size_t attribute_count() const noexcept;
        std::size_t bytes_per_vertex() const noexcept;
    private:
        constexpr static std::size_t max_attribute_count = 8;
        std::array<attribute_info, max_attribute_count> attributes_;
        std::size_t attribute_count_ = 0;
        std::size_t bytes_per_vertex_ = 0;
    };

    bool operator==(
        const vertex_declaration& l,
        const vertex_declaration& r) noexcept;
    bool operator!=(
        const vertex_declaration& l,
        const vertex_declaration& r) noexcept;
    bool operator==(
        const vertex_declaration::attribute_info& l,
        const vertex_declaration::attribute_info& r) noexcept;
    bool operator!=(
        const vertex_declaration::attribute_info& l,
        const vertex_declaration::attribute_info& r) noexcept;

    //
    // shader
    //

    class shader final : noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        explicit shader(internal_state_uptr);
        ~shader() noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // texture
    //

    class texture final : noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        explicit texture(internal_state_uptr);
        ~texture() noexcept;
    public:
        const v2u& size() const noexcept;
        const pixel_declaration& decl() const noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // index buffer
    //

    class index_buffer final : private noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        enum class usage : u8 {
            static_draw,
            stream_draw,
            dynamic_draw
        };
    public:
        explicit index_buffer(internal_state_uptr);
        ~index_buffer() noexcept;
    public:
        std::size_t buffer_size() const noexcept;
        std::size_t index_count() const noexcept;
        const index_declaration& decl() const noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // vertex buffer
    //

    class vertex_buffer final : private noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        enum class usage : u8 {
            static_draw,
            stream_draw,
            dynamic_draw
        };
    public:
        explicit vertex_buffer(internal_state_uptr);
        ~vertex_buffer() noexcept;
    public:
        std::size_t buffer_size() const noexcept;
        std::size_t vertex_count() const noexcept;
        const vertex_declaration& decl() const noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // render target
    //

    class render_target final : noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        enum class external_texture : u8 {
            color = (1 << 0),
            depth = (1 << 1),
            color_and_depth = color | depth
        };
    public:
        explicit render_target(internal_state_uptr);
        ~render_target() noexcept;
    public:
        const v2u& size() const noexcept;
        const texture_ptr& color() const noexcept;
        const texture_ptr& depth() const noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // render
    //

    class render final : public module<render> {
    public:
        enum class topology : u8 {
            triangles,
            triangles_fan,
            triangles_strip
        };

        enum class stencil_op : u8 {
            keep,
            zero,
            replace,
            incr,
            incr_wrap,
            decr,
            decr_wrap,
            invert
        };

        enum class compare_func : u8 {
            never,
            less,
            lequal,
            greater,
            gequal,
            equal,
            notequal,
            always
        };

        enum class culling_mode : u8 {
            cw,
            ccw
        };

        enum class culling_face : u8 {
            back = (1 << 0),
            front = (1 << 1),
            back_and_front = back | front
        };

        enum class blending_factor : u8 {
            zero,
            one,
            src_color,
            one_minus_src_color,
            dst_color,
            one_minus_dst_color,
            src_alpha,
            one_minus_src_alpha,
            dst_alpha,
            one_minus_dst_alpha,
            constant_color,
            one_minus_constant_color,
            constant_alpha,
            one_minus_constant_alpha,
            src_alpha_saturate
        };

        enum class blending_equation : u8 {
            add,
            subtract,
            reverse_subtract
        };

        enum class blending_color_mask : u8 {
            none = 0,

            r = (1 << 0),
            g = (1 << 1),
            b = (1 << 2),
            a = (1 << 3),

            rg = r | g,
            rb = r | b,
            ra = r | a,
            gb = g | b,
            ga = g | a,
            ba = b | a,

            rgb = r | g | b,
            rga = r | g | a,
            rba = r | b | a,
            gba = g | b | a,

            rgba = r | g | b | a
        };

        enum class sampler_wrap : u8 {
            clamp,
            repeat,
            mirror
        };

        enum class sampler_min_filter : u8 {
            nearest,
            linear
        };

        enum class sampler_mag_filter : u8 {
            nearest,
            linear
        };

        class depth_state final {
        public:
            depth_state& range(f32 near, f32 far) noexcept;
            depth_state& write(bool enable) noexcept;
            depth_state& func(compare_func func) noexcept;

            f32 range_near() const noexcept;
            f32 range_far() const noexcept;
            bool write() const noexcept;
            compare_func func() const noexcept;
        private:
            f32 range_near_ = 0.0f;
            f32 range_far_ = 1.0f;
            bool write_ = true;
            compare_func func_ = compare_func::less;
        };

        class stencil_state final {
        public:
            stencil_state& write(u8 mask) noexcept;
            stencil_state& func(compare_func func, u8 ref, u8 mask) noexcept;
            stencil_state& op(stencil_op pass, stencil_op sfail, stencil_op zfail) noexcept;

            u8 write() const noexcept;
            compare_func func() const noexcept;
            u8 ref() const noexcept;
            u8 mask() const noexcept;
            stencil_op pass() const noexcept;
            stencil_op sfail() const noexcept;
            stencil_op zfail() const noexcept;
        private:
            u8 write_ = 1;
            u8 ref_ = 0;
            u8 read_ = 1;
            stencil_op pass_ = stencil_op::keep;
            stencil_op sfail_ = stencil_op::keep;
            stencil_op zfail_ = stencil_op::keep;
            compare_func func_ = compare_func::always;
        };

        class culling_state final {
        public:
            culling_state& mode(culling_mode mode) noexcept;
            culling_state& face(culling_face face) noexcept;

            culling_mode mode() const noexcept;
            culling_face face() const noexcept;
        private:
            culling_mode mode_ = culling_mode::ccw;
            culling_face face_ = culling_face::back;
        };

        class blending_state final {
        public:
            blending_state& constant_color(const color& c) noexcept;
            blending_state& color_mask(blending_color_mask mask) noexcept;

            blending_state& factor(blending_factor src, blending_factor dst) noexcept;
            blending_state& src_factor(blending_factor src) noexcept;
            blending_state& dst_factor(blending_factor dst) noexcept;

            blending_state& rgb_factor(blending_factor src, blending_factor dst) noexcept;
            blending_state& src_rgb_factor(blending_factor src) noexcept;
            blending_state& dst_rgb_factor(blending_factor dst) noexcept;

            blending_state& alpha_factor(blending_factor src, blending_factor dst) noexcept;
            blending_state& src_alpha_factor(blending_factor src) noexcept;
            blending_state& dst_alpha_factor(blending_factor dst) noexcept;

            blending_state& equation(blending_equation equation) noexcept;
            blending_state& rgb_equation(blending_equation equation) noexcept;
            blending_state& alpha_equation(blending_equation equation) noexcept;

            const color& constant_color() const noexcept;
            blending_color_mask color_mask() const noexcept;

            blending_factor src_rgb_factor() const noexcept;
            blending_factor dst_rgb_factor() const noexcept;

            blending_factor src_alpha_factor() const noexcept;
            blending_factor dst_alpha_factor() const noexcept;

            blending_equation rgb_equation() const noexcept;
            blending_equation alpha_equation() const noexcept;
        private:
            color constant_color_ = color::clear();
            blending_color_mask color_mask_ = blending_color_mask::rgba;
            blending_factor src_rgb_factor_ = blending_factor::one;
            blending_factor dst_rgb_factor_ = blending_factor::zero;
            blending_equation rgb_equation_ = blending_equation::add;
            blending_factor src_alpha_factor_ = blending_factor::one;
            blending_factor dst_alpha_factor_ = blending_factor::zero;
            blending_equation alpha_equation_ = blending_equation::add;
        };

        class capabilities_state final {
        public:
            capabilities_state& culling(bool enable) noexcept;
            capabilities_state& blending(bool enable) noexcept;
            capabilities_state& depth_test(bool enable) noexcept;
            capabilities_state& stencil_test(bool enable) noexcept;

            bool culling() const noexcept;
            bool blending() const noexcept;
            bool depth_test() const noexcept;
            bool stencil_test() const noexcept;
        private:
            bool culling_ = false;
            bool blending_ = false;
            bool depth_test_ = false;
            bool stencil_test_ = false;
        };

        class state_block final {
        public:
            state_block& depth(const depth_state& state_block) noexcept;
            state_block& stencil(const stencil_state& state_block) noexcept;
            state_block& culling(const culling_state& state_block) noexcept;
            state_block& blending(const blending_state& state_block) noexcept;
            state_block& capabilities(const capabilities_state& state_block) noexcept;

            depth_state& depth() noexcept;
            stencil_state& stencil() noexcept;
            culling_state& culling() noexcept;
            blending_state& blending() noexcept;
            capabilities_state& capabilities() noexcept;

            const depth_state& depth() const noexcept;
            const stencil_state& stencil() const noexcept;
            const culling_state& culling() const noexcept;
            const blending_state& blending() const noexcept;
            const capabilities_state& capabilities() const noexcept;
        private:
            depth_state depth_;
            stencil_state stencil_;
            culling_state culling_;
            blending_state blending_;
            capabilities_state capabilities_;
        };

        class sampler_state {
        public:
            sampler_state& texture(const texture_ptr& texture) noexcept;

            sampler_state& wrap(sampler_wrap s, sampler_wrap t) noexcept;
            sampler_state& s_wrap(sampler_wrap s) noexcept;
            sampler_state& t_wrap(sampler_wrap t) noexcept;

            sampler_state& filter(sampler_min_filter min, sampler_mag_filter mag) noexcept;
            sampler_state& min_filter(sampler_min_filter min) noexcept;
            sampler_state& mag_filter(sampler_mag_filter mag) noexcept;

            const texture_ptr& texture() const noexcept;

            sampler_wrap s_wrap() const noexcept;
            sampler_wrap t_wrap() const noexcept;

            sampler_min_filter min_filter() const noexcept;
            sampler_mag_filter mag_filter() const noexcept;
        private:
            texture_ptr texture_;
            sampler_wrap s_wrap_ = sampler_wrap::repeat;
            sampler_wrap t_wrap_ = sampler_wrap::repeat;
            sampler_min_filter min_filter_ = sampler_min_filter::linear;
            sampler_mag_filter mag_filter_ = sampler_mag_filter::linear;
        };

        using property_value = std::variant<
            i32, f32,
            v2i, v3i, v4i,
            v2f, v3f, v4f,
            m2f, m3f, m4f>;

        template < typename T >
        class property_map final {
        public:
            property_map() = default;

            property_map(property_map&& other) = default;
            property_map& operator=(property_map&& other) = default;

            property_map(const property_map& other) = default;
            property_map& operator=(const property_map& other) = default;

            T* find(str_hash key) noexcept;
            const T* find(str_hash key) const noexcept;

            void assign(str_hash key, T&& value);
            void assign(str_hash key, const T& value);

            void clear() noexcept;
            std::size_t size() const noexcept;

            template < typename F >
            void foreach(F&& f) const;
            void merge(const property_map& other);
            bool equals(const property_map& other) const noexcept;
        private:
            flat_map<str_hash, T> values_;
        };

        class property_block final {
        public:
            property_block() = default;
            ~property_block() noexcept = default;

            property_block(property_block&&) = default;
            property_block& operator=(property_block&&) = default;

            property_block(const property_block&) = default;
            property_block& operator=(const property_block&) = default;

            property_block& clear() noexcept;
            property_block& merge(const property_block& pb);
            bool equals(const property_block& other) const noexcept;

            property_block& sampler(str_hash name, const sampler_state& s);
            sampler_state* sampler(str_hash name) noexcept;
            const sampler_state* sampler(str_hash name) const noexcept;

            template < typename T >
            property_block& property(str_hash name, T&& v);
            template < typename T >
            const T* property(str_hash name) const noexcept;

            property_block& property(str_hash name, const property_value& v);
            property_value* property(str_hash name) noexcept;
            const property_value* property(str_hash name) const noexcept;

            template < typename F >
            void foreach_by_samplers(F&& f) const;

            template < typename F >
            void foreach_by_properties(F&& f) const;

            std::size_t sampler_count() const noexcept;
            std::size_t property_count() const noexcept;
        private:
            property_map<sampler_state> samplers_;
            property_map<property_value> properties_;
        };

        class pass_state final {
        public:
            pass_state& shader(const shader_ptr& shader) noexcept;
            pass_state& states(const state_block& states) noexcept;
            pass_state& properties(const property_block& properties) noexcept;

            shader_ptr& shader() noexcept;
            state_block& states() noexcept;
            property_block& properties() noexcept;

            const shader_ptr& shader() const noexcept;
            const state_block& states() const noexcept;
            const property_block& properties() const noexcept;
        private:
            property_block properties_;
            shader_ptr shader_;
            state_block states_;
        };

        class material final {
        public:
            material& clear() noexcept;
            bool equals(const material& other) const noexcept;

            material& add_pass(const pass_state& pass) noexcept;
            std::size_t pass_count() const noexcept;

            material& properties(const property_block& properties) noexcept;

            pass_state& pass(std::size_t index) noexcept;
            const pass_state& pass(std::size_t index) const noexcept;

            property_block& properties() noexcept;
            const property_block& properties() const noexcept;
        private:
            constexpr static std::size_t max_pass_count = 8;
            std::array<pass_state, max_pass_count> passes_;
            std::size_t pass_count_ = 0;
            property_block properties_;
        };

        class geometry final {
        public:
            geometry& clear() noexcept;
            bool equals(const geometry& other) const noexcept;

            geometry& add_vertices(const vertex_buffer_ptr& vb) noexcept;
            std::size_t vertices_count() const noexcept;

            geometry& topo(topology tp) noexcept;
            geometry& indices(const index_buffer_ptr& ib) noexcept;
            geometry& vertices(std::size_t index, const vertex_buffer_ptr& vb) noexcept;

            topology& topo() noexcept;
            index_buffer_ptr& indices() noexcept;
            vertex_buffer_ptr& vertices(std::size_t index) noexcept;

            const topology& topo() const noexcept;
            const index_buffer_ptr& indices() const noexcept;
            const vertex_buffer_ptr& vertices(std::size_t index) const noexcept;
        private:
            constexpr static std::size_t max_vertices_count = 8;
            index_buffer_ptr indices_;
            std::array<vertex_buffer_ptr, max_vertices_count> vertices_;
            std::size_t vertices_count_ = 0;
            topology topology_ = topology::triangles;
        };

        class zero_command final {
        public:
            zero_command() = default;
        };

        class draw_command final {
        public:
            draw_command() = delete;
            draw_command(const material& mat, const geometry& geo) noexcept;
            draw_command(const material& mat, const geometry& geo, const property_block& props) noexcept;

            draw_command& index_range(std::size_t first, std::size_t count) noexcept;

            draw_command& first_index(std::size_t value) noexcept;
            draw_command& index_count(std::size_t value) noexcept;
            draw_command& material_ref(const material& value) noexcept;
            draw_command& geometry_ref(const geometry& value) noexcept;
            draw_command& properties_ref(const property_block& value);

            std::size_t first_index() const noexcept;
            std::size_t index_count() const noexcept;
            const material& material_ref() const noexcept;
            const geometry& geometry_ref() const noexcept;
            const property_block& properties_ref() const noexcept;
        private:
            std::size_t first_index_ = 0;
            std::size_t index_count_ = std::size_t(-1);
            const material* material_ = nullptr;
            const geometry* geometry_ = nullptr;
            const property_block* properties_ = nullptr;
        };

        class clear_command final {
        public:
            enum class buffer : u8 {
                color = (1 << 0),
                depth = (1 << 1),
                stencil = (1 << 2),
                color_depth = color | depth,
                color_stencil = color | stencil,
                depth_stencil = depth | stencil,
                color_depth_stencil = color | depth | stencil
            };
        public:
            clear_command() = default;
            clear_command(buffer clear_buffer) noexcept;

            clear_command& color_value(const color& value) noexcept;
            clear_command& depth_value(f32 value) noexcept;
            clear_command& stencil_value(u8 value) noexcept;

            color& color_value() noexcept;
            f32& depth_value() noexcept;
            u8& stencil_value() noexcept;

            const color& color_value() const noexcept;
            f32 depth_value() const noexcept;
            u8 stencil_value() const noexcept;

            buffer& clear_buffer() noexcept;
            buffer clear_buffer() const noexcept;
        private:
            color color_value_ = color::clear();
            f32 depth_value_ = 1.f;
            u8 stencil_value_ = 0;
            buffer clear_buffer_ = buffer::color_depth_stencil;
        };

        class target_command final {
        public:
            target_command() = default;
            target_command(const render_target_ptr& rt) noexcept;
            target_command& target(const render_target_ptr& value) noexcept;
            render_target_ptr& target() noexcept;
            const render_target_ptr& target() const noexcept;
        private:
            render_target_ptr target_;
        };

        class viewport_command final {
        public:
            viewport_command() = delete;
            viewport_command(const b2u& viewport_rect) noexcept;
            viewport_command(const b2u& viewport_rect, const b2u& scissor_rect) noexcept;

            viewport_command& viewport_rect(const b2u& value) noexcept;
            viewport_command& scissor_rect(const b2u& value) noexcept;
            viewport_command& scissoring(bool value) noexcept;

            b2u& viewport_rect() noexcept;
            b2u& scissor_rect() noexcept;
            bool& scissoring() noexcept;

            const b2u& viewport_rect() const noexcept;
            const b2u& scissor_rect() const noexcept;
            bool scissoring() const noexcept;
        private:
            b2u viewport_rect_ = b2u::zero();
            b2u scissor_rect_ = b2u::zero();
            bool scissoring_ = false;
        };

        using command_value = std::variant<
            zero_command,
            draw_command,
            clear_command,
            target_command,
            viewport_command>;

        template < std::size_t N >
        class command_block final {
        public:
            command_block() = default;

            command_block& add_command(command_value&& value);
            command_block& add_command(const command_value& value);

            const command_value& command(std::size_t index) const noexcept;
            std::size_t command_count() const noexcept;
        private:
            std::array<command_value, N> commands_;
            std::size_t command_count_ = 0;
        };

        enum class api_profile {
            unknown,
            gles_2_0,
            gles_3_0,
            gl_2_1_compat,
            gl_3_2_compat
        };

        struct device_caps {
            api_profile profile = api_profile::unknown;

            u32 max_texture_size = 0;
            u32 max_renderbuffer_size = 0;
            u32 max_cube_map_texture_size = 0;

            u32 max_texture_image_units = 0;
            u32 max_combined_texture_image_units = 0;

            u32 max_vertex_attributes = 0;
            u32 max_vertex_texture_image_units = 0;

            u32 max_varying_vectors = 0;
            u32 max_vertex_uniform_vectors = 0;
            u32 max_fragment_uniform_vectors = 0;

            bool npot_texture_supported = false;
            bool depth_texture_supported = false;
            bool render_target_supported = false;

            bool element_index_uint = false;

            bool depth16_supported = false;
            bool depth24_supported = false;
            bool depth24_stencil8_supported = false;

            bool dxt_compression_supported = false;
            bool etc1_compression_supported = false;
            bool etc2_compression_supported = false;
            bool astc_compression_supported = false;
            bool pvrtc_compression_supported = false;
            bool pvrtc2_compression_supported = false;
        };
    public:
        render(debug& d, window& w);
        ~render() noexcept final;

        shader_ptr create_shader(
            str_view vertex_source,
            str_view fragment_source);

        shader_ptr create_shader(
            const input_stream_uptr& vertex_stream,
            const input_stream_uptr& fragment_stream);

        texture_ptr create_texture(
            const image& image);

        texture_ptr create_texture(
            const input_stream_uptr& image_stream);

        texture_ptr create_texture(
            const v2u& size,
            const pixel_declaration& decl);

        index_buffer_ptr create_index_buffer(
            buffer_view indices,
            const index_declaration& decl,
            index_buffer::usage usage);

        vertex_buffer_ptr create_vertex_buffer(
            buffer_view vertices,
            const vertex_declaration& decl,
            vertex_buffer::usage usage);

        render_target_ptr create_render_target(
            const v2u& size,
            const pixel_declaration& color_decl,
            const pixel_declaration& depth_decl,
            render_target::external_texture external_texture);

        template < std::size_t N >
        render& execute(const command_block<N>& commands);
        render& execute(const command_value& command);

        render& execute(const draw_command& command);
        render& execute(const clear_command& command);
        render& execute(const target_command& command);
        render& execute(const viewport_command& command);

        render& update_buffer(
            const index_buffer_ptr& ibuffer,
            buffer_view indices,
            std::size_t offset);

        render& update_buffer(
            const vertex_buffer_ptr& vbuffer,
            buffer_view vertices,
            std::size_t offset);

        render& update_texture(
            const texture_ptr& tex,
            const image& img,
            v2u offset);

        render& update_texture(
            const texture_ptr& tex,
            buffer_view pixels,
            const b2u& region);

        const device_caps& device_capabilities() const noexcept;
        bool is_pixel_supported(const pixel_declaration& decl) const noexcept;
        bool is_index_supported(const index_declaration& decl) const noexcept;
        bool is_vertex_supported(const vertex_declaration& decl) const noexcept;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };
}

namespace e2d
{
    //
    // render::state_block
    //

    bool operator==(const render::state_block& l, const render::state_block& r) noexcept;
    bool operator!=(const render::state_block& l, const render::state_block& r) noexcept;

    bool operator==(const render::depth_state& l, const render::depth_state& r) noexcept;
    bool operator!=(const render::depth_state& l, const render::depth_state& r) noexcept;

    bool operator==(const render::stencil_state& l, const render::stencil_state& r) noexcept;
    bool operator!=(const render::stencil_state& l, const render::stencil_state& r) noexcept;

    bool operator==(const render::culling_state& l, const render::culling_state& r) noexcept;
    bool operator!=(const render::culling_state& l, const render::culling_state& r) noexcept;

    bool operator==(const render::blending_state& l, const render::blending_state& r) noexcept;
    bool operator!=(const render::blending_state& l, const render::blending_state& r) noexcept;

    bool operator==(const render::capabilities_state& l, const render::capabilities_state& r) noexcept;
    bool operator!=(const render::capabilities_state& l, const render::capabilities_state& r) noexcept;

    //
    // render::property_block
    //

    bool operator==(const render::property_block& l, const render::property_block& r) noexcept;
    bool operator!=(const render::property_block& l, const render::property_block& r) noexcept;

    bool operator==(const render::sampler_state& l, const render::sampler_state& r) noexcept;
    bool operator!=(const render::sampler_state& l, const render::sampler_state& r) noexcept;

    //
    // render::material
    //

    bool operator==(const render::material& l, const render::material& r) noexcept;
    bool operator!=(const render::material& l, const render::material& r) noexcept;

    bool operator==(const render::pass_state& l, const render::pass_state& r) noexcept;
    bool operator!=(const render::pass_state& l, const render::pass_state& r) noexcept;

    //
    // render::geometry
    //

    bool operator==(const render::geometry& l, const render::geometry& r) noexcept;
    bool operator!=(const render::geometry& l, const render::geometry& r) noexcept;
}

#include "render.inl"

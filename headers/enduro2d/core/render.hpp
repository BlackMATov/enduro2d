/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_core.hpp"
#include <optional> // TODO

namespace e2d
{
    class render;
    class shader;
    class texture;
    class index_buffer;
    class vertex_buffer;
    class const_buffer;
    class render_target;
    class pixel_declaration;
    class index_declaration;
    class vertex_declaration;
    class vertex_attribs;

    using shader_ptr = std::shared_ptr<shader>;
    using texture_ptr = std::shared_ptr<texture>;
    using index_buffer_ptr = std::shared_ptr<index_buffer>;
    using vertex_buffer_ptr = std::shared_ptr<vertex_buffer>;
    using vertex_attribs_ptr = std::shared_ptr<const vertex_attribs>;
    using const_buffer_ptr = std::shared_ptr<const_buffer>;
    using render_target_ptr = std::shared_ptr<render_target>;
    
    namespace render_cfg {
        constexpr static std::size_t max_attribute_count = 8;
        constexpr static std::size_t max_vertex_buffer_count = 4;
        constexpr static std::size_t max_samplers_in_block = 4;
    }

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
            unknown,

            depth16,
            depth16_stencil8,
            depth24,
            depth24_stencil8,
            depth32,
            depth32_stencil8,

            g8,
            ga8,
            rgb8,
            rgba8,

            rgb_dxt1,
            rgba_dxt1,
            rgba_dxt3,
            rgba_dxt5,

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
        std::size_t bits_per_pixel() const noexcept;
        v2u compressed_block_size() const noexcept;
    private:
        pixel_type type_ = pixel_type::unknown;
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

        struct hash {
            std::size_t operator()(const vertex_declaration& x) const noexcept;
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
        std::array<attribute_info, render_cfg::max_attribute_count> attributes_;
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
    private:
        internal_state_uptr state_;
    };

    //
    // vertex attribs
    //

    class vertex_attribs final : private noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        explicit vertex_attribs(internal_state_uptr);
        ~vertex_attribs() noexcept;
    public:
        const vertex_declaration& decl() const noexcept;
    private:
        internal_state_uptr state_;
    };

    //
    // const buffer
    //

    class const_buffer final : private noncopyable {
    public:
        class internal_state;
        using internal_state_uptr = std::unique_ptr<internal_state>;
        const internal_state& state() const noexcept;
    public:
        enum class scope : u8 {
            render_pass,
            material,
            draw_command,
            last_
        };
    public:
        explicit const_buffer(internal_state_uptr);
        ~const_buffer() noexcept;
    public:
        [[nodiscard]] std::size_t buffer_size() const noexcept;
        [[nodiscard]] scope binding_scope() const noexcept;
        [[nodiscard]] bool is_compatible_with(const shader_ptr& shader) const noexcept;
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
        
        enum class attachment_load_op : u8 {
            load,
            clear,
            invalidate,
        };

        enum class attachment_store_op : u8 {
            store,
            discard,
        };

        class depth_state final {
        public:
            depth_state& test(bool enable) noexcept;
            depth_state& write(bool enable) noexcept;
            depth_state& func(compare_func func) noexcept;

            bool test() const noexcept;
            bool write() const noexcept;
            compare_func func() const noexcept;
        private:
            bool test_ = false;
            bool write_ = true;
            compare_func func_ = compare_func::less;
        };
        
        class depth_dynamic_state final {
        public:
            depth_dynamic_state& test(bool enable) noexcept;
            depth_dynamic_state& write(bool enable) noexcept;

            bool test() const noexcept;
            bool write() const noexcept;
        private:
            bool test_ = false;
            bool write_ = true;
        };

        class stencil_state final {
        public:
            stencil_state& test(bool enabled) noexcept;
            stencil_state& write(u8 mask) noexcept;
            stencil_state& func(compare_func func, u8 ref, u8 mask) noexcept;
            stencil_state& op(stencil_op pass, stencil_op sfail, stencil_op zfail) noexcept;

            bool test() const noexcept;
            u8 write() const noexcept;
            compare_func func() const noexcept;
            u8 ref() const noexcept;
            u8 mask() const noexcept;
            stencil_op pass() const noexcept;
            stencil_op sfail() const noexcept;
            stencil_op zfail() const noexcept;
        private:
            bool test_ = false;
            u8 write_mask_ = 0xFF;
            u8 ref_ = 0;
            u8 read_ = 0xFF;
            stencil_op pass_ = stencil_op::keep;
            stencil_op sfail_ = stencil_op::keep;
            stencil_op zfail_ = stencil_op::keep;
            compare_func func_ = compare_func::always;
        };

        class culling_state final {
        public:
            culling_state& face(culling_face face) noexcept;
            culling_state& enable(bool value) noexcept;
            
            culling_face face() const noexcept;
            bool enabled() const noexcept;
        private:
            culling_face face_ = culling_face::back;
            bool enabled_ = false;
        };

        class blending_state final {
        public:
            blending_state& enable(bool value) noexcept;
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

            bool enabled() const noexcept;
            blending_color_mask color_mask() const noexcept;

            blending_factor src_rgb_factor() const noexcept;
            blending_factor dst_rgb_factor() const noexcept;

            blending_factor src_alpha_factor() const noexcept;
            blending_factor dst_alpha_factor() const noexcept;

            blending_equation rgb_equation() const noexcept;
            blending_equation alpha_equation() const noexcept;
        private:
            bool enabled_ = false;
            blending_color_mask color_mask_ = blending_color_mask::rgba;
            blending_factor src_rgb_factor_ = blending_factor::one;
            blending_factor dst_rgb_factor_ = blending_factor::zero;
            blending_equation rgb_equation_ = blending_equation::add;
            blending_factor src_alpha_factor_ = blending_factor::one;
            blending_factor dst_alpha_factor_ = blending_factor::zero;
            blending_equation alpha_equation_ = blending_equation::add;
        };

        class state_block final {
        public:
            state_block& depth(const depth_state& state_block) noexcept;
            state_block& stencil(const stencil_state& state_block) noexcept;
            state_block& culling(const culling_state& state_block) noexcept;
            state_block& blending(const blending_state& state_block) noexcept;

            depth_state& depth() noexcept;
            stencil_state& stencil() noexcept;
            culling_state& culling() noexcept;
            blending_state& blending() noexcept;

            const depth_state& depth() const noexcept;
            const stencil_state& stencil() const noexcept;
            const culling_state& culling() const noexcept;
            const blending_state& blending() const noexcept;
        private:
            depth_state depth_;
            stencil_state stencil_;
            culling_state culling_;
            blending_state blending_;
        };

        class sampler_state {
        public:
            sampler_state& texture(const texture_ptr& texture) noexcept;

            sampler_state& wrap(sampler_wrap st) noexcept;
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

        class sampler_block final {
        public:
            enum class scope : u8 {
                render_pass,
                material,
                last_
            };
        public:
            sampler_block() = default;

            sampler_block& bind(str_hash name, const sampler_state& state) noexcept;

            std::size_t count() const noexcept;
            str_hash name(std::size_t index) const noexcept;
            const sampler_state& sampler(std::size_t index) const noexcept;
        private:
            std::array<str_hash, render_cfg::max_samplers_in_block> names_;
            std::array<sampler_state, render_cfg::max_samplers_in_block> samplers_;
            std::size_t count_ = 0;
        };

        class property_map final {
        public:
            using property_value = stdex::variant<
                f32,
                v2f, v3f, v4f,
                m2f, m3f, m4f>;
        public:
            property_map() = default;

            property_map(property_map&& other) = default;
            property_map& operator=(property_map&& other) = default;

            property_map(const property_map& other) = default;
            property_map& operator=(const property_map& other) = default;

            property_value* find(str_hash key) noexcept;
            const property_value* find(str_hash key) const noexcept;
            template< typename T >
            const T* find(str_hash key) const noexcept;

            property_map& property(str_hash key, property_value&& value);
            property_map& property(str_hash key, const property_value& value);

            void clear() noexcept;
            std::size_t size() const noexcept;

            template < typename F >
            void foreach(F&& f) const;
            property_map& merge(const property_map& other);
            bool equals(const property_map& other) const noexcept;
        private:
            flat_map<str_hash, property_value> values_; // TODO: use static memory
        };
        
        class renderpass_desc final {
        public:
            renderpass_desc();
            renderpass_desc(const render_target_ptr& rt) noexcept;

            renderpass_desc& target(const render_target_ptr& value) noexcept;
            [[nodiscard]] const render_target_ptr& target() const noexcept;

            renderpass_desc& viewport(const b2u& value) noexcept;
            [[nodiscard]] const b2u& viewport() const noexcept;
            
            renderpass_desc& depth_range(const v2f& value) noexcept;
            [[nodiscard]] const v2f& depth_range() const noexcept;

            renderpass_desc& states(const state_block& states) noexcept;
            [[nodiscard]] const state_block& states() const noexcept;
            
            renderpass_desc& color_clear(const color& value) noexcept;
            renderpass_desc& color_load() noexcept;
            renderpass_desc& color_invalidate() noexcept;
            renderpass_desc& color_store() noexcept;
            renderpass_desc& color_discard() noexcept;
            renderpass_desc& color_load_op(attachment_load_op value) noexcept;
            renderpass_desc& color_store_op(attachment_store_op value) noexcept;
            [[nodiscard]] const color& color_clear_value() const noexcept;
            [[nodiscard]] attachment_load_op color_load_op() const noexcept;
            [[nodiscard]] attachment_store_op color_store_op() const noexcept;

            renderpass_desc& depth_clear(float value) noexcept;
            renderpass_desc& depth_load() noexcept;
            renderpass_desc& depth_invalidate() noexcept;
            renderpass_desc& depth_store() noexcept;
            renderpass_desc& depth_discard() noexcept;
            renderpass_desc& depth_load_op(attachment_load_op value) noexcept;
            renderpass_desc& depth_store_op(attachment_store_op value) noexcept;
            [[nodiscard]] float depth_clear_value() const noexcept;
            [[nodiscard]] attachment_load_op depth_load_op() const noexcept;
            [[nodiscard]] attachment_store_op depth_store_op() const noexcept;
            
            renderpass_desc& stencil_clear(u8 value) noexcept;
            renderpass_desc& stencil_load() noexcept;
            renderpass_desc& stencil_invalidate() noexcept;
            renderpass_desc& stencil_store() noexcept;
            renderpass_desc& stencil_discard() noexcept;
            renderpass_desc& stencil_load_op(attachment_load_op value) noexcept;
            renderpass_desc& stencil_store_op(attachment_store_op value) noexcept;
            [[nodiscard]] u8 stencil_clear_value() const noexcept;
            [[nodiscard]] attachment_load_op stencil_load_op() const noexcept;
            [[nodiscard]] attachment_store_op stencil_store_op() const noexcept;
        private:
            template < typename ClearValue >
            struct target_props_ {
                attachment_load_op load_op = attachment_load_op::load;
                attachment_store_op store_op = attachment_store_op::store;
                ClearValue clear_value;
            };
        private:
            render_target_ptr target_;
            target_props_<color> color_;
            target_props_<float> depth_;
            target_props_<u8> stencil_;
            b2u viewport_;
            v2f depth_range_;
            state_block states_; // default/global states for all render pass, some states can be overriden by material
        };

        using blending_state_opt = std::optional<blending_state>;
        using culling_state_opt = std::optional<culling_state>;
        using depth_dynamic_state_opt = std::optional<depth_dynamic_state>;

        class material final {
        public:
            material() = default;

            material& blending(const blending_state& value) noexcept;
            material& culling(const culling_state& value) noexcept;
            material& depth(const depth_dynamic_state& value) noexcept;
            material& shader(const shader_ptr& value) noexcept;
            material& constants(const const_buffer_ptr& value) noexcept;
            material& sampler(str_hash name, const sampler_state& sampler) noexcept;
            material& samplers(const sampler_block& value) noexcept;

            const blending_state_opt& blending() const noexcept;
            const culling_state_opt& culling() const noexcept;
            const depth_dynamic_state_opt& depth() const noexcept;
            const shader_ptr& shader() const noexcept;
            const const_buffer_ptr& constants() const noexcept;
            const sampler_block& samplers() const noexcept;
        private:
            blending_state_opt blending_;
            culling_state_opt culling_;
            depth_dynamic_state_opt depth_;
            shader_ptr shader_;
            const_buffer_ptr constants_;
            sampler_block sampler_block_;
        };

        using material_cptr = std::shared_ptr<const material>;

        class zero_command final {
        public:
            zero_command() = default;
        };

        class bind_vertex_buffers_command final {
        public:
            bind_vertex_buffers_command() = default;

            bind_vertex_buffers_command& add(
                const vertex_buffer_ptr& buffer,
                const vertex_attribs_ptr& attribs,
                std::size_t offset = 0) noexcept;

            bind_vertex_buffers_command& bind(
                std::size_t index,
                const vertex_buffer_ptr& buffer,
                const vertex_attribs_ptr& attribs,
                std::size_t offset = 0) noexcept;

            std::size_t binding_count() const noexcept;
            const vertex_buffer_ptr& vertices(std::size_t index) const noexcept;
            const vertex_attribs_ptr& attributes(std::size_t index) const noexcept;
            std::size_t vertex_offset(std::size_t index) const noexcept;

        private:
            std::array<vertex_buffer_ptr, render_cfg::max_vertex_buffer_count> buffers_;
            std::array<vertex_attribs_ptr, render_cfg::max_vertex_buffer_count> attribs_;
            std::array<std::size_t, render_cfg::max_vertex_buffer_count> offsets_; // in bytes
            std::size_t count_ = 0;
        };
        
        class material_command final {
        public:
            material_command() = delete;
            material_command(const material_cptr& value);

            const material_cptr& material() const noexcept;
        private:
            material_cptr material_;
        };
        
        class scissor_command final {
        public:
            scissor_command() = default;
            scissor_command(const b2u& scissor_rect) noexcept;

            scissor_command& scissor_rect(const b2u& value) noexcept;
            scissor_command& scissoring(bool value) noexcept;

            const b2u& scissor_rect() const noexcept;
            bool scissoring() const noexcept;
        private:
            b2u scissor_rect_;
            bool scissoring_ = false;
        };

        template < typename T >
        class change_state_command_ final {
        public:
            change_state_command_() = default;
            explicit change_state_command_(const T& state);

            const std::optional<T>& state() const noexcept;
        private:
            std::optional<T> state_;
        };

        using blending_state_command = change_state_command_<blending_state>;
        using culling_state_command = change_state_command_<culling_state>;
        using stencil_state_command = change_state_command_<stencil_state>;
        using depth_state_command = change_state_command_<depth_dynamic_state>;
        using blend_constant_command = change_state_command_<color>;
        
        class draw_command final {
        public:
            draw_command() = default;

            draw_command& constants(const const_buffer_ptr& value) noexcept;

            draw_command& topo(topology value) noexcept;
            draw_command& vertex_range(u32 first, u32 count) noexcept;
            draw_command& first_vertex(u32 value) noexcept;
            draw_command& vertex_count(u32 value) noexcept;

            u32 first_vertex() const noexcept;
            u32 vertex_count() const noexcept;
            topology topo() const noexcept;
            const const_buffer_ptr& constants() const noexcept;
        private:
            const_buffer_ptr cbuffer_;
            topology topology_ = topology::triangles;
            u32 first_vertex_ = 0;
            u32 vertex_count_ = 0;
        };
        
        class draw_indexed_command final {
        public:
            draw_indexed_command() = default;

            draw_indexed_command& constants(const const_buffer_ptr& value) noexcept;
            draw_indexed_command& indices(const index_buffer_ptr& value) noexcept;
            draw_indexed_command& topo(topology value) noexcept;

            //draw_indexed_command& index_range(u32 count, size_t offset) noexcept;
            draw_indexed_command& index_offset(size_t offsetInBytes) noexcept;
            draw_indexed_command& index_count(u32 value) noexcept;
            
            size_t index_offset() const noexcept;
            u32 index_count() const noexcept;
            topology topo() const noexcept;
            const index_buffer_ptr& indices() const noexcept;
            const const_buffer_ptr& constants() const noexcept;
        private:
            const_buffer_ptr cbuffer_;
            index_buffer_ptr index_buffer_;
            topology topology_ = topology::triangles;
            size_t index_offset_ = 0; // in bytes
            u32 index_count_ = 0;
        };

        using command_value = stdex::variant<
            zero_command,
            bind_vertex_buffers_command,
            material_command,
            scissor_command,
            blending_state_command,
            culling_state_command,
            stencil_state_command,
            depth_state_command,
            blend_constant_command,
            draw_command,
            draw_indexed_command>;

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
            opengles2,
            opengles3,
            opengl2_compat,
            opengl4_compat
        };

        struct device_caps {
            api_profile profile = api_profile::unknown;

            pixel_declaration depth_rt_format;
            pixel_declaration depth_stencil_rt_format;

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

            bool dxt_compression_supported = false;
            bool pvrtc_compression_supported = false;
            bool pvrtc2_compression_supported = false;
            
            bool uniform_buffer_supported = false;
        };

        struct statistics {
            u32 render_pass_count = 0;
            u32 attachment_load_count = 0;
            u32 attachment_store_count = 0;
            u32 draw_calls = 0;
        };
        
        class batchr;
    public:
        render(debug& d, window& w);
        ~render() noexcept final;

        shader_ptr create_shader(
            const shader_source& source);

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

        index_buffer_ptr create_index_buffer(
            size_t size,
            const index_declaration& decl,
            index_buffer::usage usage);

        vertex_buffer_ptr create_vertex_buffer(
            buffer_view vertices,
            vertex_buffer::usage usage);
        
        vertex_buffer_ptr create_vertex_buffer(
            size_t size,
            vertex_buffer::usage usage);

        vertex_attribs_ptr create_vertex_attribs(
            const vertex_declaration& decl);
        
        const_buffer_ptr create_const_buffer(
            const shader_ptr& shader,
            const_buffer::scope scope);
        const_buffer_ptr create_const_buffer(
            const cbuffer_template_ptr& templ,
            const_buffer::scope scope);

        render_target_ptr create_render_target(
            const v2u& size,
            const pixel_declaration& color_decl,
            const pixel_declaration& depth_decl,
            render_target::external_texture external_texture);

        render& begin_pass(
            const renderpass_desc& desc,
            const const_buffer_ptr& constants = nullptr,
            const sampler_block& samplers = {});
        render& end_pass();
        render& present();

        template < std::size_t N >
        render& execute(const command_block<N>& commands);
        render& execute(const command_value& command);

        render& execute(const bind_vertex_buffers_command& command);
        render& execute(const material_command& command);
        render& execute(const scissor_command& command);
        render& execute(const blending_state_command& command);
        render& execute(const culling_state_command& command);
        render& execute(const stencil_state_command& command);
        render& execute(const depth_state_command& command);
        render& execute(const blend_constant_command& command);
        render& execute(const draw_command& command);
        render& execute(const draw_indexed_command& command);

        render& set_material(const material& mtr);

        render& update_buffer(
            const index_buffer_ptr& ibuffer,
            buffer_view indices,
            std::size_t offset);

        render& update_buffer(
            const vertex_buffer_ptr& vbuffer,
            buffer_view vertices,
            std::size_t offset);

        render& update_buffer(
            const const_buffer_ptr& cbuffer,
            const property_map& properties);

        render& update_texture(
            const texture_ptr& tex,
            const image& img,
            v2u offset);

        render& update_texture(
            const texture_ptr& tex,
            buffer_view pixels,
            const b2u& region);

        batchr& batcher() noexcept;

        const device_caps& device_capabilities() const noexcept;
        const statistics& frame_statistic() const noexcept;

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

    //
    // render::sampler_state
    //

    bool operator==(const render::sampler_state& l, const render::sampler_state& r) noexcept;
    bool operator!=(const render::sampler_state& l, const render::sampler_state& r) noexcept;

    //
    // render::sampler_block
    //

    bool operator==(const render::sampler_block& l, const render::sampler_block& r) noexcept;
    bool operator!=(const render::sampler_block& l, const render::sampler_block& r) noexcept;
    
    //
    // render::material
    //

    bool operator==(const render::material& l, const render::material& r) noexcept;
    bool operator!=(const render::material& l, const render::material& r) noexcept;
}

namespace e2d::json_utils
{
    void add_render_schema_definitions(rapidjson::Document& schema);
}

namespace e2d::json_utils
{
    bool try_parse_value(const rapidjson::Value& root, render::topology& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::stencil_op& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::compare_func& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::culling_face& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::blending_factor& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::blending_equation& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::blending_color_mask& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::sampler_wrap& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::sampler_min_filter& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::sampler_mag_filter& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::attachment_load_op& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::attachment_store_op& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::depth_state& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::depth_dynamic_state& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::stencil_state& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::culling_state& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::blending_state& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::state_block& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::property_map& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, render::renderpass_desc& v) noexcept;
}

#include "render.inl"

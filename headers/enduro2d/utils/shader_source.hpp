/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"
#include "strings.hpp"
#include "cbuffer_template.hpp"

namespace e2d
{
    class shader_source final {
    public:
        enum class scope_type : u8 {
            render_pass,
            material,
            draw_command,
            last_,
            unknown
        };

        using value_type = cbuffer_template::value_type;
        using blocks_t = std::array<cbuffer_template_cptr, u32(scope_type::last_)>;

        enum class sampler_type : u8 {
            _2d,
            cube_map,
            unknown
        };

        struct sampler {
            str name;
            u8 unit = 0;
            sampler_type type = sampler_type::unknown;
            scope_type scope = scope_type::unknown;
        };

        struct attribute {
            str name;
            u8 index = 0;
            value_type type = value_type::unknown;
        };
        
        static constexpr char cb_pass_name[] = "cb_pass";
        static constexpr char cb_material_name[] = "cb_material";
        static constexpr char cb_command_name[] = "cb_command";
    public:
        shader_source() = default;

        shader_source(shader_source&& other) noexcept;
        shader_source& operator=(shader_source&& other) noexcept;

        shader_source(const shader_source& other);
        shader_source& operator=(const shader_source& other);

        shader_source& assign(shader_source&& other) noexcept;
        shader_source& assign(const shader_source& other);

        void swap(shader_source& other) noexcept;
        void clear() noexcept;
        bool empty() const noexcept;

        shader_source& vertex_shader(str source);
        shader_source& fragment_shader(str source);
        shader_source& add_sampler(str name, u32 unit, sampler_type type, scope_type scope);
        shader_source& add_attribute(str name, u32 index, value_type type);
        shader_source& set_block(const cbuffer_template_cptr& cb, scope_type scope);

        const str& vertex_shader() const noexcept;
        const str& fragment_shader() const noexcept;
        const vector<sampler>& samplers() const noexcept;
        const vector<attribute>& attributes() const noexcept;
        const cbuffer_template_cptr& block(scope_type scope) const noexcept;
    private:
        str vs_;
        str fs_;
        vector<sampler> samplers_;
        vector<attribute> attributes_;
        blocks_t blocks_;
    };
}

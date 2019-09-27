/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"
#include "strings.hpp"

namespace e2d
{
    class cbuffer_template;
    using cbuffer_template_ptr = std::shared_ptr<const cbuffer_template>;

    class cbuffer_template final {
    public:
        enum class value_type : u8 {
            f32,
            v2f,
            v3f,
            v4f,
            m2f,
            m3f,
            m4f,
            unknown
        };

        struct uniform {
            str name;
            str_hash name_hash;
            u16 offset = 0;
            //u16 array_size = 0; // not supported
            value_type type = value_type::unknown;
        public:
            uniform(str&& name, u16 off, value_type t)
            : name(name), name_hash(str_hash(this->name)), offset(off), type(t) {}
        };

    public:
        cbuffer_template() = default;

        cbuffer_template& add_uniform(str name, size_t offset, value_type type);

        const vector<uniform>& uniforms() const noexcept;
        size_t block_size() const noexcept;
    private:
        vector<uniform> uniforms_;
        size_t size_ = 0;
    };

}

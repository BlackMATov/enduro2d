/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/utils/cbuffer_template.hpp>

namespace
{
    using namespace e2d;

    size_t get_uniform_size(cbuffer_template::value_type type) {
        using value_type = cbuffer_template::value_type;
        switch ( type ) {
            case value_type::f32: return sizeof(float);
            case value_type::v2f: return sizeof(v2f);
            case value_type::v3f: return sizeof(v3f);
            case value_type::v4f: return sizeof(v4f);
            case value_type::m2f: return sizeof(v4f) * 2;
            case value_type::m3f: return sizeof(v4f) * 3;
            case value_type::m4f: return sizeof(v4f) * 4;
            default:
                E2D_ASSERT_MSG(false, "unexpected uniform value type");
                return 0;
        }
    }
}

namespace e2d
{
    cbuffer_template& cbuffer_template::add_uniform(
        str name,
        size_t offset,
        value_type type)
    {
        uniforms_.push_back({
            std::move(name),
            math::numeric_cast<u16>(offset),
            type});
        size_ = math::max(size_, offset + get_uniform_size(type));
        return *this;
    }
    
    /*cbuffer_template& cbuffer_template::set_uid(u32 value) noexcept {
        uid_ = value;
        return *this;
    }*/

    const vector<cbuffer_template::uniform>& cbuffer_template::uniforms() const noexcept {
        return uniforms_;
    }

    /*u32 cbuffer_template::uid() const noexcept {
        return uid_;
    }*/
    
    size_t cbuffer_template::block_size() const noexcept {
        return size_;
    }
}

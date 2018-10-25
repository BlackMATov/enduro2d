/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018 Matvey Cherevko
 ******************************************************************************/

#ifndef E2D_INCLUDE_GUARD_8247225BE32B4135BDAECEEE88535A86
#define E2D_INCLUDE_GUARD_8247225BE32B4135BDAECEEE88535A86
#pragma once

#include "_core.hpp"
#include "render.hpp"

namespace e2d
{
    //
    // vertex_declaration
    //

    template < typename T >
    vertex_declaration& vertex_declaration::add_attribute(str_hash name) noexcept {
        E2D_UNUSED(name);
        static_assert(sizeof(T) == 0, "not implemented for this type");
        return *this;
    }

    #define DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(t, rows, columns, type)\
        template <>\
        inline vertex_declaration& vertex_declaration::add_attribute<t>(str_hash name) noexcept {\
            return add_attribute(name, (rows), (columns), attribute_type::type, false);\
        }

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(i8, 1, 1, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<i8>, 1, 2, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<i8>, 1, 3, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<i8>, 1, 4, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<i8>, 2, 2, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<i8>, 3, 3, signed_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<i8>, 4, 4, signed_byte)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(u8, 1, 1, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<u8>, 1, 2, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<u8>, 1, 3, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<u8>, 1, 4, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<u8>, 2, 2, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<u8>, 3, 3, unsigned_byte)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<u8>, 4, 4, unsigned_byte)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(i16, 1, 1, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<i16>, 1, 2, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<i16>, 1, 3, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<i16>, 1, 4, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<i16>, 2, 2, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<i16>, 3, 3, signed_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<i16>, 4, 4, signed_short)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(u16, 1, 1, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<u16>, 1, 2, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<u16>, 1, 3, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<u16>, 1, 4, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<u16>, 2, 2, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<u16>, 3, 3, unsigned_short)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<u16>, 4, 4, unsigned_short)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(f32, 1, 1, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec2<f32>, 1, 2, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec3<f32>, 1, 3, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(vec4<f32>, 1, 4, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat2<f32>, 2, 2, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat3<f32>, 3, 3, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(mat4<f32>, 4, 4, floating_point)

    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(color, 1, 4, floating_point)
    DEFINE_ADD_ATTRIBUTE_SPECIALIZATION(color32, 1, 4, unsigned_byte)

    #undef DEFINE_ADD_ATTRIBUTE_SPECIALIZATION

    //
    // render::property_block
    //

    template < typename T >
    render::property_block& render::property_block::property(str_hash name, T&& v) {
        properties_[name] = std::forward<T>(v);
        return *this;
    }

    template < typename T >
    const T* render::property_block::property(str_hash name) const noexcept {
        const auto iter = properties_.find(name);
        return iter != properties_.end()
            ? stdex::get_if<T>(&iter->second)
            : nullptr;
    }

    template < typename F >
    void render::property_block::foreach_by_samplers(F&& f) const {
        for ( const auto& p : samplers_ ) {
            stdex::invoke(f, p.first, p.second);
        }
    }

    template < typename F >
    void render::property_block::foreach_by_properties(F&& f) const {
        for ( const auto& p : properties_ ) {
            stdex::invoke(f, p.first, p.second);
        }
    }

    //
    // render::command_block
    //

    template < std::size_t N >
    render::command_block<N>& render::command_block<N>::clear() noexcept {
        commands_.clear();
        return *this;
    }

    template < std::size_t N >
    render::command_block<N>& render::command_block<N>::add_command(command_value&& value) {
        E2D_ASSERT(command_count_ < commands_.size());
        commands_[command_count_] = std::move(value);
        ++command_count_;
        return *this;
    }

    template < std::size_t N >
    render::command_block<N>& render::command_block<N>::add_command(const command_value& value) {
        E2D_ASSERT(command_count_ < commands_.size());
        commands_[command_count_] = value;
        ++command_count_;
        return *this;
    }

    template < std::size_t N >
    const render::command_value& render::command_block<N>::command(std::size_t index) const noexcept {
        E2D_ASSERT(index < command_count_);
        return commands_[index];
    }

    template < std::size_t N >
    std::size_t render::command_block<N>::command_count() const noexcept {
        return command_count_;
    }

    //
    // render
    //

    template < std::size_t N >
    render& render::execute(const command_block<N>& commands) {
        E2D_ASSERT(
            std::this_thread::get_id() ==
            modules::main_thread<render>());

        for ( std::size_t i = 0; i < commands.command_count(); ++i ) {
            execute(commands.command(i));
        }
        return *this;
    }
}

#endif

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

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
    // render::property_map
    //

    template < typename T >
    T* render::property_map<T>::find(str_hash key) noexcept {
        const auto iter = values_.find(key);
        return iter != values_.end()
            ? &iter->second
            : nullptr;
    }

    template < typename T >
    const T* render::property_map<T>::find(str_hash key) const noexcept {
        const auto iter = values_.find(key);
        return iter != values_.end()
            ? &iter->second
            : nullptr;
    }

    template < typename T >
    void render::property_map<T>::assign(str_hash key, T&& value) {
        values_[key] = std::move(value);
    }

    template < typename T >
    void render::property_map<T>::assign(str_hash key, const T& value) {
        values_[key] = value;
    }

    template < typename T >
    void render::property_map<T>::clear() noexcept {
        values_.clear();
    }

    template < typename T >
    std::size_t render::property_map<T>::size() const noexcept {
        return values_.size();
    }

    template < typename T >
    template < typename F >
    void render::property_map<T>::foreach(F&& f) const {
        for ( const auto& [name, value] : values_ ) {
            f(name, value);
        }
    }

    template < typename T >
    void render::property_map<T>::merge(const property_map& other) {
        if ( this != &other ) {
            other.foreach([this](str_hash name, const T& value){
                assign(name, value);
            });
        }
    }

    template < typename T >
    bool render::property_map<T>::equals(const property_map& other) const noexcept {
        return this == &other
            ? true
            : values_ == other.values_;
    }

    //
    // render::property_block
    //

    template < typename T >
    render::property_block& render::property_block::property(str_hash name, T&& v) {
        properties_.assign(name, std::forward<T>(v));
        return *this;
    }

    template < typename T >
    const T* render::property_block::property(str_hash name) const noexcept {
        const property_value* prop = properties_.find(name);
        return prop
            ? std::get_if<T>(prop)
            : nullptr;
    }

    template < typename F >
    void render::property_block::foreach_by_samplers(F&& f) const {
        samplers_.foreach(std::forward<F>(f));
    }

    template < typename F >
    void render::property_block::foreach_by_properties(F&& f) const {
        properties_.foreach(std::forward<F>(f));
    }

    //
    // render::command_block
    //

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
        E2D_ASSERT(is_in_main_thread());
        for ( std::size_t i = 0, e = commands.command_count(); i < e; ++i ) {
            execute(commands.command(i));
        }
        return *this;
    }
}

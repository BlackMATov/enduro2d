/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"
#include "assets/sprite_asset.hpp"
#include "assets/sprite_9p_asset.hpp"

namespace e2d
{
    template < typename T >
    class ui_image_style_templ final {
    public:
        ui_image_style_templ() = default;
        ~ui_image_style_templ() noexcept = default;

        ui_image_style_templ& disabled(const T& value) noexcept;
        ui_image_style_templ& idle(const T& value) noexcept;
        ui_image_style_templ& mouse_over(const T& value) noexcept;
        ui_image_style_templ& touched(const T& value) noexcept;
        ui_image_style_templ& selected(const T& value) noexcept;
        ui_image_style_templ& dragging(const T& value) noexcept;

        const T& disabled() const noexcept;
        const T& idle() const noexcept;
        const T& mouse_over() const noexcept;
        const T& touched() const noexcept;
        const T& selected() const noexcept;
        const T& dragging() const noexcept;
    private:
        T disabled_;
        T idle_;
        T mouse_over_;
        T touched_;
        T selected_;
        T dragging_;
    };

    using ui_sprite_style = ui_image_style_templ<sprite_asset::ptr>;
    using ui_sprite_9p_style = ui_image_style_templ<sprite_9p_asset::ptr>;
}

namespace e2d
{
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::disabled(const T& value) noexcept {
        disabled_ = value;
        return *this;
    }
    
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::idle(const T& value) noexcept {
        idle_ = value;
        return *this;
    }
    
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::mouse_over(const T& value) noexcept {
        mouse_over_ = value;
        return *this;
    }
    
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::touched(const T& value) noexcept {
        touched_ = value;
        return *this;
    }
    
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::selected(const T& value) noexcept {
        selected_ = value;
        return *this;
    }
   
    template < typename T >
    ui_image_style_templ<T>& ui_image_style_templ<T>::dragging(const T& value) noexcept {
        dragging_ = value;
        return *this;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::disabled() const noexcept {
        return disabled_;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::idle() const noexcept {
        return idle_;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::mouse_over() const noexcept {
        return mouse_over_;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::touched() const noexcept {
        return touched_;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::selected() const noexcept {
        return selected_;
    }
    
    template < typename T >
    const T& ui_image_style_templ<T>::dragging() const noexcept {
        return dragging_;
    }
}

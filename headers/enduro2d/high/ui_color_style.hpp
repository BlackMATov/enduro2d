/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

namespace e2d
{
    class ui_color_style final {
    public:
        ui_color_style() = default;

        ui_color_style& disabled(const color32& value) noexcept;
        ui_color_style& idle(const color32& value) noexcept;
        ui_color_style& mouse_over(const color32& value) noexcept;
        ui_color_style& touched(const color32& value) noexcept;
        ui_color_style& selected(const color32& value) noexcept;
        ui_color_style& dragging(const color32& value) noexcept;

        color32 disabled() const noexcept;
        color32 idle() const noexcept;
        color32 mouse_over() const noexcept;
        color32 touched() const noexcept;
        color32 selected() const noexcept;
        color32 dragging() const noexcept;
    private:
        color32 disabled_ = color32(125, 125, 125, 255);
        color32 idle_ = color32::white();
        color32 mouse_over_ = color32::white();
        color32 touched_ = color32::white();
        color32 selected_ = color32::white();
        color32 dragging_ = color32::white();
    };
}

namespace e2d
{
    inline ui_color_style& ui_color_style::disabled(const color32& value) noexcept {
        disabled_ = value;
        return *this;
    }

    inline ui_color_style& ui_color_style::idle(const color32& value) noexcept {
        idle_ = value;
        return *this;
    }

    inline ui_color_style& ui_color_style::mouse_over(const color32& value) noexcept {
        mouse_over_ = value;
        return *this;
    }

    inline ui_color_style& ui_color_style::touched(const color32& value) noexcept {
        touched_ = value;
        return *this;
    }

    inline ui_color_style& ui_color_style::selected(const color32& value) noexcept {
        selected_ = value;
        return *this;
    }

    inline ui_color_style& ui_color_style::dragging(const color32& value) noexcept {
        dragging_ = value;
        return *this;
    }

    inline color32 ui_color_style::disabled() const noexcept {
        return disabled_;
    }

    inline color32 ui_color_style::idle() const noexcept {
        return idle_;
    }

    inline color32 ui_color_style::mouse_over() const noexcept {
        return mouse_over_;
    }

    inline color32 ui_color_style::touched() const noexcept {
        return touched_;
    }

    inline color32 ui_color_style::selected() const noexcept {
        return selected_;
    }

    inline color32 ui_color_style::dragging() const noexcept {
        return dragging_;
    }
}

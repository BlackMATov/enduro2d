/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

namespace e2d
{
    class ui_font_style final {
    public:
        struct style {
            color32 tint;
            color32 outline_color;
            float outline_width = 0.0f;
        };
    public:
        ui_font_style() = default;

        ui_font_style& disabled(color32 tint, color32 outline_color, float outline_width) noexcept;
        ui_font_style& idle(color32 tint, color32 outline_color, float outline_width) noexcept;
        ui_font_style& mouse_over(color32 tint, color32 outline_color, float outline_width) noexcept;
        ui_font_style& touched(color32 tint, color32 outline_color, float outline_width) noexcept;
        ui_font_style& selected(color32 tint, color32 outline_color, float outline_width) noexcept;
        ui_font_style& dragging(color32 tint, color32 outline_color, float outline_width) noexcept;

        const style& disabled() const noexcept;
        const style& idle() const noexcept;
        const style& mouse_over() const noexcept;
        const style& touched() const noexcept;
        const style& selected() const noexcept;
        const style& dragging() const noexcept;
    private:
        style disabled_ {color32(125, 125, 125, 255), color32(), 0.0f};
        style idle_ {color32::white(), color32(), 0.0f};
        style mouse_over_ {color32::white(), color32(), 0.0f};
        style touched_ {color32::white(), color32(), 0.0f};
        style selected_ {color32::white(), color32(), 0.0f};
        style dragging_ {color32::white(), color32(), 0.0f};
    };
}

namespace e2d
{
    inline ui_font_style& ui_font_style::disabled(color32 tint, color32 outline_color, float outline_width) noexcept {
        disabled_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline ui_font_style& ui_font_style::idle(color32 tint, color32 outline_color, float outline_width) noexcept {
        idle_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline ui_font_style& ui_font_style::mouse_over(color32 tint, color32 outline_color, float outline_width) noexcept {
        mouse_over_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline ui_font_style& ui_font_style::touched(color32 tint, color32 outline_color, float outline_width) noexcept {
        touched_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline ui_font_style& ui_font_style::selected(color32 tint, color32 outline_color, float outline_width) noexcept {
        selected_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline ui_font_style& ui_font_style::dragging(color32 tint, color32 outline_color, float outline_width) noexcept {
        dragging_ = {tint, outline_color, outline_width};
        return *this;
    }

    inline const ui_font_style::style& ui_font_style::disabled() const noexcept {
        return disabled_;
    }

    inline const ui_font_style::style& ui_font_style::idle() const noexcept {
        return idle_;
    }

    inline const ui_font_style::style& ui_font_style::mouse_over() const noexcept {
        return mouse_over_;
    }

    inline const ui_font_style::style& ui_font_style::touched() const noexcept {
        return touched_;
    }

    inline const ui_font_style::style& ui_font_style::selected() const noexcept {
        return selected_;
    }

    inline const ui_font_style::style& ui_font_style::dragging() const noexcept {
        return dragging_;
    }
}

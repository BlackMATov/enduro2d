/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"
#include "../factory.hpp"
#include "../assets/ui_color_style_asset.hpp"
#include "../assets/ui_image_style_asset.hpp"
#include "../assets/ui_font_style_asset.hpp"

namespace e2d
{
    class ui_style final {
    public:
        class ui_style_state final {
        public:
            enum type {
                disabled,
                mouse_over,
                touched,
                selected,
                dragging,
                count_
            };
            using bits = std::bitset<u32(count_)>;
        public:
            ui_style_state& set(type flag, bool value = true) noexcept;
            ui_style_state& set_all() noexcept;

            bool get(type flag) const noexcept;
        public:
            bits flags;
        };

        class style_changed_tag {};
        using type = ui_style_state::type;
        using bits = ui_style_state::bits;
    public:
        ui_style();
        ui_style(const ui_style&) = default;

        ui_style& propagate(type flag, bool value) noexcept;
        ui_style& propagate_all() noexcept;
        ui_style& propagate(ui_style_state flags) noexcept;

        bool propagate(type flag) const noexcept;
        ui_style_state propagate() const noexcept;

        ui_style& set(type flag, bool value) noexcept;

        bool operator[](type flag) const noexcept;
        ui_style_state current() const noexcept;
    private:
        ui_style_state bits_;
        ui_style_state propagate_bits_;
    };
    
    template <>
    class factory_loader<ui_style> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_style& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };

    template <>
    class factory_loader<ui_style::style_changed_tag> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_style::style_changed_tag& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_color_style_comp final {
    public:
        ui_color_style_comp() = default;

        ui_color_style_comp& style(const ui_color_style_asset::ptr& value) noexcept;
        const ui_color_style_asset::ptr& style() const noexcept;
    private:
        ui_color_style_asset::ptr style_;
    };
    
    template <>
    class factory_loader<ui_color_style_comp> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_color_style_comp& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_image_style_comp final {
    };
    
    template <>
    class factory_loader<ui_image_style_comp> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_image_style_comp& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_font_style_comp final {
    };
    
    template <>
    class factory_loader<ui_font_style_comp> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_font_style_comp& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline ui_style::ui_style_state& ui_style::ui_style_state::set(type flag, bool value) noexcept {
        flags.set(u32(flag), value);
        return *this;
    }

    inline ui_style::ui_style_state& ui_style::ui_style_state::set_all() noexcept {
        flags = bits(~0ull);
        return *this;
    }

    inline bool ui_style::ui_style_state::get(type flag) const noexcept {
        return flags[u32(flag)];
    }
    
    inline ui_style::ui_style() {
        propagate_bits_.set(type::disabled);
    }

    inline ui_style& ui_style::propagate(type flag, bool value) noexcept {
        propagate_bits_.set(flag, value);
        return *this;
    }

    inline ui_style& ui_style::propagate_all() noexcept {
        propagate_bits_.set_all();
        return *this;
    }
        
    inline ui_style& ui_style::propagate(ui_style_state flags) noexcept {
        propagate_bits_ = flags;
        return *this;
    }

    inline bool ui_style::propagate(type flag) const noexcept {
        return propagate_bits_.get(flag);
    }
        
    inline ui_style::ui_style_state ui_style::propagate() const noexcept {
        return propagate_bits_;
    }

    inline ui_style& ui_style::set(type flag, bool value) noexcept {
        bits_.set(flag, value);
        return *this;
    }

    inline bool ui_style::operator[](type flag) const noexcept {
        return bits_.get(flag);
    }
        
    inline ui_style::ui_style_state ui_style::current() const noexcept {
        return bits_;
    }
}

namespace e2d
{
    inline ui_color_style_comp& ui_color_style_comp::style(const ui_color_style_asset::ptr& value) noexcept {
        style_ = value;
        return *this;
    }

    inline const ui_color_style_asset::ptr& ui_color_style_comp::style() const noexcept {
        return style_;
    }
}

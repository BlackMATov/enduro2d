/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"
#include "../factory.hpp"

namespace e2d
{
    class ui_button final {
    public:
        struct click_evt {
            v2f pos; // in screen space
        };
    public:
        ui_button() = default;
    };

    template <>
    class factory_loader<ui_button> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_button& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_selectable final {
    public:
        struct changed_evt {
            bool selected;
        };
    public:
        ui_selectable() = default;
        
        // TODO: selected state may be changed without event
        ui_selectable& selected(bool value) noexcept;
        bool selected() const noexcept;
    private:
        bool selected_ = false;
    };

    template <>
    class factory_loader<ui_selectable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_selectable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_draggable final {
    public:
        struct drag_begin_evt {
        };

        struct drag_update_evt {
        };

        struct drag_end_evt {
        };
    public:
        ui_draggable() = default;
    };

    template <>
    class factory_loader<ui_draggable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_draggable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_scrollable final {
    public:
        struct scroll_begin_evt {
        };
        struct scroll_update_evt {
        };
        struct scroll_end_evt {
        };
        struct overscroll_evt {
        };
    public:
        ui_scrollable() = default;
        
        ui_scrollable& separate_axes(bool value) noexcept;
        bool separate_axes() const noexcept;
        
        ui_scrollable& overscroll_enabled(bool value) noexcept;
        bool overscroll_enabled() const noexcept;
    private:
        // only one axis can be changed when scrolling
        bool separate_axes_ = true;
        bool overscroll_enabled_ = true;
    };

    template <>
    class factory_loader<ui_scrollable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_scrollable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    class ui_controller_events final {
    public:
        using events_t = vector<std::any>;
    public:
        ui_controller_events() = default;

        template < typename T >
        ui_controller_events& add_event(T&& data);
        const events_t& events() const noexcept;
    private:
        events_t events_;
    };

    template < typename T >
    ui_controller_events& ui_controller_events::add_event(T&& data) {
        events_.push_back(std::move(data));
        return *this;
    }

    inline const ui_controller_events::events_t& ui_controller_events::events() const noexcept {
        return events_;
    }
}

namespace e2d
{
    class ui_controller_event_name final {
    public:
        ui_controller_event_name() = default;

        ui_controller_event_name& set_name(str value);
        const str& name() const noexcept;
    private:
        str name_;
    };
    
    template <>
    class factory_loader<ui_controller_event_name> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            ui_controller_event_name& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

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

    public:
        v3f start_pos;
        v3f node_pos;
        v3f diff;
        bool started = false;
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

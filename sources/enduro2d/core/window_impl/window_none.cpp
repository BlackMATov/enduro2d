/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "window.hpp"

#if defined(E2D_WINDOW_MODE) && E2D_WINDOW_MODE == E2D_WINDOW_MODE_NONE

namespace e2d
{
    class window::state final : private e2d::noncopyable {
    public:
        using listeners_t = vector<event_listener_uptr>;
    public:
        listeners_t listeners;
        std::recursive_mutex rmutex;
        v2u virtual_size;
        str title;
        bool vsync = false;
        bool fullscreen = false;
        bool cursor_hidden = false;
        bool should_close = false;
        bool enabled = true;
        bool visible = true;
        bool focused = true;
        bool minimized = false;
    public:
        state(const v2u& size, str_view title, bool vsync, bool fullscreen)
        : virtual_size(size)
        , title(make_utf8(title))
        , vsync(vsync)
        , fullscreen(fullscreen) {}
        ~state() noexcept = default;

        template < typename F, typename... Args >
        void for_all_listeners(const F& f, const Args&... args) noexcept {
            std::lock_guard<std::recursive_mutex> guard(rmutex);
            for ( const event_listener_uptr& listener : listeners ) {
                if ( listener ) {
                    std::invoke(f, listener.get(), args...);
                }
            }
        }
    };

    window::window(const v2u& size, str_view title, bool vsync, bool fullscreen)
    : state_(new state(size, title, vsync, fullscreen)) {}
    window::~window() noexcept = default;

    void window::hide() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->visible = false;
    }

    void window::show() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->visible = true;
    }

    void window::restore() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        if ( !state_->focused ) {
            state_->focused = true;
            state_->for_all_listeners(&event_listener::on_window_focus, true);
        }
        if ( state_->minimized ) {
            state_->minimized = false;
            state_->for_all_listeners(&event_listener::on_window_minimize, false);
        }
    }

    void window::minimize() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        if ( state_->focused ) {
            state_->focused = false;
            state_->for_all_listeners(&event_listener::on_window_focus, false);
        }
        if ( !state_->minimized ) {
            state_->minimized = true;
            state_->for_all_listeners(&event_listener::on_window_minimize, true);
        }
    }

    bool window::enabled() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->enabled;
    }

    bool window::visible() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->visible;
    }

    bool window::focused() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->focused;
    }

    bool window::minimized() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->minimized;
    }

    bool window::fullscreen() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->fullscreen;
    }

    bool window::toggle_fullscreen(bool yesno) noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->fullscreen = yesno;
        return true;
    }

    void window::hide_cursor() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->cursor_hidden = true;
    }

    void window::show_cursor() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->cursor_hidden = false;
    }

    bool window::is_cursor_hidden() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->cursor_hidden;
    }

    v2u window::real_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->virtual_size;
    }

    v2u window::virtual_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->virtual_size;
    }

    v2u window::framebuffer_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->virtual_size;
    }

    const str& window::title() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->title;
    }

    void window::set_title(str_view title) {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->title = make_utf8(title);
    }

    bool window::should_close() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->should_close;
    }

    void window::set_should_close(bool yesno) noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->should_close = yesno;
    }

    void window::bind_context() noexcept {
    }

    void window::swap_buffers() noexcept {
    }

    bool window::poll_events() noexcept {
        return false;
    }

    window::event_listener& window::register_event_listener(event_listener_uptr listener) {
        E2D_ASSERT(listener);
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->listeners.push_back(std::move(listener));
        return *state_->listeners.back();
    }

    void window::unregister_event_listener(const event_listener& listener) noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        for ( auto iter = state_->listeners.begin(); iter != state_->listeners.end(); ) {
            if ( iter->get() == &listener ) {
                iter = state_->listeners.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

#endif

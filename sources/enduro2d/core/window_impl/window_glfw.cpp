/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "window.hpp"

#if defined(E2D_WINDOW_MODE) && E2D_WINDOW_MODE == E2D_WINDOW_MODE_GLFW

#include <GLFW/glfw3.h>

namespace
{
    using namespace e2d;

    class glfw_state;
    using glfw_state_ptr = std::shared_ptr<glfw_state>;

    class glfw_state final : private noncopyable {
    public:
        glfw_state() {
            glfwSetErrorCallback(error_handler);
            if ( !glfwInit() ) {
                throw bad_window_operation();
            }
        }

        ~glfw_state() noexcept {
            glfwTerminate();
            glfwSetErrorCallback(nullptr);
        }
    public:
        static bool poll_events() noexcept {
            std::lock_guard<std::mutex> guard(mutex_);
            if ( shared_state_ ) {
                glfwPollEvents();
                return true;
            }
            return false;
        }

        static glfw_state_ptr get_shared_state() {
            std::lock_guard<std::mutex> guard(mutex_);
            if ( !shared_state_ ) {
                shared_state_ = std::make_shared<glfw_state>();
            }
            return shared_state_;
        }
    private:
        static std::mutex mutex_;
        static std::shared_ptr<glfw_state> shared_state_;
    private:
        static void error_handler(int error, const char* message) noexcept {
            try {
                the<debug>().error(
                    "GLFW: error(%0) message(%1)",
                    error, message);
            } catch (...) {
                // nothing
            }
        }
    };

    std::mutex glfw_state::mutex_;
    std::shared_ptr<glfw_state> glfw_state::shared_state_;

    mouse_button convert_glfw_mouse_button(int m) noexcept {
        #define DEFINE_CASE(x,y) case GLFW_MOUSE_BUTTON_##x: return mouse_button::y
        switch ( m ) {
            DEFINE_CASE(LEFT, left);
            DEFINE_CASE(RIGHT, right);
            DEFINE_CASE(MIDDLE, middle);
            DEFINE_CASE(4, x1);
            DEFINE_CASE(5, x2);
            default: return mouse_button::unknown;
        }
        #undef DEFINE_CASE
    }

    keyboard_key convert_glfw_keyboard_key(int k) noexcept {
        #define DEFINE_CASE(x,y) case GLFW_KEY_##x: return keyboard_key::y
        switch ( k ) {
            DEFINE_CASE(0, _0);
            DEFINE_CASE(1, _1);
            DEFINE_CASE(2, _2);
            DEFINE_CASE(3, _3);
            DEFINE_CASE(4, _4);
            DEFINE_CASE(5, _5);
            DEFINE_CASE(6, _6);
            DEFINE_CASE(7, _7);
            DEFINE_CASE(8, _8);
            DEFINE_CASE(9, _9);

            DEFINE_CASE(A, a);
            DEFINE_CASE(B, b);
            DEFINE_CASE(C, c);
            DEFINE_CASE(D, d);
            DEFINE_CASE(E, e);
            DEFINE_CASE(F, f);
            DEFINE_CASE(G, g);
            DEFINE_CASE(H, h);
            DEFINE_CASE(I, i);
            DEFINE_CASE(J, j);
            DEFINE_CASE(K, k);
            DEFINE_CASE(L, l);
            DEFINE_CASE(M, m);
            DEFINE_CASE(N, n);
            DEFINE_CASE(O, o);
            DEFINE_CASE(P, p);
            DEFINE_CASE(Q, q);
            DEFINE_CASE(R, r);
            DEFINE_CASE(S, s);
            DEFINE_CASE(T, t);
            DEFINE_CASE(U, u);
            DEFINE_CASE(V, v);
            DEFINE_CASE(W, w);
            DEFINE_CASE(X, x);
            DEFINE_CASE(Y, y);
            DEFINE_CASE(Z, z);

            DEFINE_CASE(F1, f1);
            DEFINE_CASE(F2, f2);
            DEFINE_CASE(F3, f3);
            DEFINE_CASE(F4, f4);
            DEFINE_CASE(F5, f5);
            DEFINE_CASE(F6, f6);
            DEFINE_CASE(F7, f7);
            DEFINE_CASE(F8, f8);
            DEFINE_CASE(F9, f9);
            DEFINE_CASE(F10, f10);
            DEFINE_CASE(F11, f11);
            DEFINE_CASE(F12, f12);
            DEFINE_CASE(F13, f13);
            DEFINE_CASE(F14, f14);
            DEFINE_CASE(F15, f15);
            DEFINE_CASE(F16, f16);
            DEFINE_CASE(F17, f17);
            DEFINE_CASE(F18, f18);
            DEFINE_CASE(F19, f19);
            DEFINE_CASE(F20, f20);
            DEFINE_CASE(F21, f21);
            DEFINE_CASE(F22, f22);
            DEFINE_CASE(F23, f23);
            DEFINE_CASE(F24, f24);
            DEFINE_CASE(F25, f25);

            DEFINE_CASE(MINUS, minus);
            DEFINE_CASE(EQUAL, equal);
            DEFINE_CASE(BACKSPACE, backspace);
            DEFINE_CASE(WORLD_1, section_sign);
            DEFINE_CASE(GRAVE_ACCENT, grave_accent);

            DEFINE_CASE(LEFT_BRACKET, lbracket);
            DEFINE_CASE(RIGHT_BRACKET, rbracket);
            DEFINE_CASE(SEMICOLON, semicolon);
            DEFINE_CASE(APOSTROPHE, apostrophe);
            DEFINE_CASE(BACKSLASH, backslash);

            DEFINE_CASE(COMMA, comma);
            DEFINE_CASE(PERIOD, period);
            DEFINE_CASE(SLASH, slash);

            DEFINE_CASE(ESCAPE, escape);
            DEFINE_CASE(TAB, tab);
            DEFINE_CASE(CAPS_LOCK, caps_lock);
            DEFINE_CASE(SPACE, space);
            DEFINE_CASE(ENTER, enter);

            DEFINE_CASE(LEFT_SHIFT, lshift);
            DEFINE_CASE(RIGHT_SHIFT, rshift);
            DEFINE_CASE(LEFT_CONTROL, lcontrol);
            DEFINE_CASE(RIGHT_CONTROL, rcontrol);
            DEFINE_CASE(LEFT_ALT, lalt);
            DEFINE_CASE(RIGHT_ALT, ralt);
            DEFINE_CASE(LEFT_SUPER, lsuper);
            DEFINE_CASE(RIGHT_SUPER, rsuper);
            DEFINE_CASE(MENU, menu);

            DEFINE_CASE(PRINT_SCREEN, print_screen);
            DEFINE_CASE(SCROLL_LOCK, scroll_lock);
            DEFINE_CASE(PAUSE, pause);

            DEFINE_CASE(INSERT, insert);
            DEFINE_CASE(DELETE, del);
            DEFINE_CASE(HOME, home);
            DEFINE_CASE(END, end);
            DEFINE_CASE(PAGE_UP, page_up);
            DEFINE_CASE(PAGE_DOWN, page_down);

            DEFINE_CASE(LEFT, left);
            DEFINE_CASE(UP, up);
            DEFINE_CASE(RIGHT, right);
            DEFINE_CASE(DOWN, down);

            DEFINE_CASE(KP_0, kp_0);
            DEFINE_CASE(KP_1, kp_1);
            DEFINE_CASE(KP_2, kp_2);
            DEFINE_CASE(KP_3, kp_3);
            DEFINE_CASE(KP_4, kp_4);
            DEFINE_CASE(KP_5, kp_5);
            DEFINE_CASE(KP_6, kp_6);
            DEFINE_CASE(KP_7, kp_7);
            DEFINE_CASE(KP_8, kp_8);
            DEFINE_CASE(KP_9, kp_9);

            DEFINE_CASE(NUM_LOCK, kp_num_lock);
            DEFINE_CASE(KP_DIVIDE, kp_divide);
            DEFINE_CASE(KP_MULTIPLY, kp_multiply);
            DEFINE_CASE(KP_SUBTRACT, kp_subtract);
            DEFINE_CASE(KP_ADD, kp_add);
            DEFINE_CASE(KP_ENTER, kp_enter);
            DEFINE_CASE(KP_EQUAL, kp_equal);
            DEFINE_CASE(KP_DECIMAL, kp_decimal);

            default: return keyboard_key::unknown;
        }
        #undef DEFINE_CASE
    }

    mouse_button_action convert_glfw_mouse_button_action(int ma) noexcept {
        switch ( ma ) {
            case GLFW_PRESS:
                return mouse_button_action::press;
            case GLFW_RELEASE:
                return mouse_button_action::release;
            default:
                return mouse_button_action::unknown;
        }
    }

    keyboard_key_action convert_glfw_keyboard_key_action(int ka) noexcept {
        switch ( ka ) {
            case GLFW_PRESS:
                return keyboard_key_action::press;
            case GLFW_REPEAT:
                return keyboard_key_action::repeat;
            case GLFW_RELEASE:
                return keyboard_key_action::release;
            default:
                return keyboard_key_action::unknown;
        }
    }
}

namespace e2d
{
    class window::state final : private e2d::noncopyable {
    public:
        using window_uptr = std::unique_ptr<
            GLFWwindow, void(*)(GLFWwindow*)>;
        using listeners_t = vector<event_listener_uptr>;
    public:
        listeners_t listeners;
        std::recursive_mutex rmutex;
        glfw_state_ptr shared_state;
        window_uptr window;
        v2u real_size;
        v2u virtual_size;
        v2u framebuffer_size;
        str title;
        bool vsync = false;
        bool fullscreen = false;
        bool cursor_hidden = false;
    public:
        state(const v2u& size, str_view ntitle, bool nvsync, bool nfullscreen)
        : shared_state(glfw_state::get_shared_state())
        , window(nullptr, glfwDestroyWindow)
        , real_size(size)
        , virtual_size(size)
        , framebuffer_size(size)
        , title(ntitle)
        , vsync(nvsync)
        , fullscreen(nfullscreen)
        {
            window = open_window_(
                size,
                make_utf8(ntitle),
                vsync,
                fullscreen);

            if ( !window ) {
                throw bad_window_operation();
            }

            update_window_sizes();
            glfwSetWindowUserPointer(window.get(), this);

            glfwSetCharCallback(window.get(), input_char_callback_);
            glfwSetCursorPosCallback(window.get(), move_cursor_callback_);
            glfwSetScrollCallback(window.get(), mouse_scroll_callback_);
            glfwSetMouseButtonCallback(window.get(), mouse_button_callback_);
            glfwSetKeyCallback(window.get(), keyboard_key_callback_);
            glfwSetWindowSizeCallback(window.get(), window_size_callback_);
            glfwSetWindowCloseCallback(window.get(), window_close_callback_);
            glfwSetWindowFocusCallback(window.get(), window_focus_callback_);
            glfwSetWindowIconifyCallback(window.get(), window_minimize_callback_);
        }

        ~state() noexcept {
            // reset window before shared state
            window.reset();
        }

        void update_window_sizes() noexcept {
            std::lock_guard<std::recursive_mutex> guard(rmutex);
            E2D_ASSERT(window);
            {
                int w = 0, h = 0;
                glfwGetWindowSize(window.get(), &w, &h);
                real_size = make_vec2(w, h).cast_to<u32>();
            }
            {
                int w = 0, h = 0;
                glfwGetFramebufferSize(window.get(), &w, &h);
                framebuffer_size = make_vec2(w, h).cast_to<u32>();
            }
        }

        template < typename F, typename... Args >
        void for_all_listeners(const F& f, const Args&... args) noexcept {
            std::lock_guard<std::recursive_mutex> guard(rmutex);
            for ( const event_listener_uptr& listener : listeners ) {
                if ( listener ) {
                    std::invoke(f, *listener.get(), args...);
                }
            }
        }

        event_listener& on_new_event_listener(event_listener& listener) noexcept {
            double cursor_x = 0.0, cursor_y = 0.0;
            E2D_ASSERT(window);
            glfwGetCursorPos(window.get(), &cursor_x, &cursor_y);
            listener.on_move_cursor(make_vec2(cursor_x, cursor_y).cast_to<f32>());
            return listener;
        }
    private:
        static window_uptr open_window_(
            const v2u& virtual_size,
            const str& title,
            bool vsync,
            bool fullscreen) noexcept
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            if ( !monitor ) {
                return {nullptr, glfwDestroyWindow};
            }
            const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);
            if ( !video_mode ) {
                return {nullptr, glfwDestroyWindow};
            }
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
            glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
        #if defined(E2D_BUILD_MODE) && E2D_BUILD_MODE == E2D_BUILD_MODE_DEBUG
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        #endif
            v2i real_size = fullscreen
                ? make_vec2(video_mode->width, video_mode->height)
                : virtual_size.cast_to<i32>();
            GLFWwindow* w = glfwCreateWindow(
                real_size.x,
                real_size.y,
                title.c_str(),
                fullscreen ? monitor : nullptr,
                nullptr);
            if ( w ) {
                glfwMakeContextCurrent(w);
                glfwSwapInterval(vsync ? 1 : 0);
            }
            return {w, glfwDestroyWindow};
        }

        static void input_char_callback_(GLFWwindow* window, u32 uchar) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_input_char,
                    static_cast<char32_t>(uchar));
            }
        }

        static void move_cursor_callback_(GLFWwindow* window, double pos_x, double pos_y) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_move_cursor,
                    make_vec2(pos_x, pos_y).cast_to<f32>());
            }
        }

        static void mouse_scroll_callback_(GLFWwindow* window, double delta_x, double delta_y) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_mouse_scroll,
                    make_vec2(delta_x, delta_y).cast_to<f32>());
            }
        }

        static void mouse_button_callback_(GLFWwindow* window, int button, int action, int mods) noexcept {
            E2D_UNUSED(mods);
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_mouse_button,
                    convert_glfw_mouse_button(button),
                    convert_glfw_mouse_button_action(action));
            }
        }

        static void keyboard_key_callback_(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept {
            E2D_UNUSED(mods);
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_keyboard_key,
                    convert_glfw_keyboard_key(key),
                    math::numeric_cast<u32>(scancode),
                    convert_glfw_keyboard_key_action(action));
            }
        }

        static void window_size_callback_(GLFWwindow* window, int w, int h) noexcept {
            E2D_UNUSED(w, h);
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->update_window_sizes();
            }
        }

        static void window_close_callback_(GLFWwindow* window) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_window_close);
            }
        }

        static void window_focus_callback_(GLFWwindow* window, int focused) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_window_focus,
                    !!focused);
            }
        }

        static void window_minimize_callback_(GLFWwindow* window, int minimized) noexcept {
            state* self = static_cast<state*>(glfwGetWindowUserPointer(window));
            if ( self ) {
                self->for_all_listeners(
                    &event_listener::on_window_minimize,
                    !!minimized);
            }
        }
    };

    //
    // class window
    //

    window::window(const v2u& size, str_view title, bool vsync, bool fullscreen)
    : state_(new state(size, title, vsync, fullscreen)) {}
    window::~window() noexcept = default;

    void window::hide() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwHideWindow(state_->window.get());
    }

    void window::show() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwShowWindow(state_->window.get());
    }

    void window::restore() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwRestoreWindow(state_->window.get());
    }

    void window::minimize() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwIconifyWindow(state_->window.get());
    }

    bool window::enabled() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        if ( !state_->window || state_->window.get() != glfwGetCurrentContext() ) {
            return false;
        }
        int w = 0, h = 0;
        glfwGetWindowSize(state_->window.get(), &w, &h);
        return w > 0 && h > 0;
    }

    bool window::visible() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        return GLFW_TRUE == glfwGetWindowAttrib(state_->window.get(), GLFW_VISIBLE);
    }

    bool window::focused() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        return GLFW_TRUE == glfwGetWindowAttrib(state_->window.get(), GLFW_FOCUSED);
    }

    bool window::minimized() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        return GLFW_TRUE == glfwGetWindowAttrib(state_->window.get(), GLFW_ICONIFIED);
    }

    bool window::fullscreen() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->fullscreen;
    }

    bool window::toggle_fullscreen(bool yesno) noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        if ( state_->fullscreen == yesno ) {
            return true;
        }
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        if ( !monitor ) {
            return false;
        }
        const GLFWvidmode* video_mode = glfwGetVideoMode(monitor);
        if ( !video_mode ) {
            return false;
        }
        v2i real_size = yesno
            ? make_vec2(video_mode->width, video_mode->height)
            : state_->virtual_size.cast_to<i32>();
        E2D_ASSERT(state_->window);
        glfwSetWindowMonitor(
            state_->window.get(),
            yesno ? monitor : nullptr,
            yesno ? 0 : math::max(0, video_mode->width / 2 - real_size.x / 2),
            yesno ? 0 : math::max(0, video_mode->height / 2 - real_size.y / 2),
            real_size.x,
            real_size.y,
            GLFW_DONT_CARE);
        state_->fullscreen = yesno;
        return true;
    }

    void window::hide_cursor() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwSetInputMode(state_->window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        state_->cursor_hidden = true;
    }

    void window::show_cursor() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwSetInputMode(state_->window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        state_->cursor_hidden = false;
    }

    bool window::is_cursor_hidden() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->cursor_hidden;
    }

    v2u window::real_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->real_size;
    }

    v2u window::virtual_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->virtual_size;
    }

    v2u window::framebuffer_size() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->framebuffer_size;
    }

    const str& window::title() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        return state_->title;
    }

    void window::set_title(str_view title) {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->title = make_utf8(title);
        E2D_ASSERT(state_->window);
        glfwSetWindowTitle(state_->window.get(), state_->title.c_str());
    }

    bool window::should_close() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        return GLFW_TRUE == glfwWindowShouldClose(state_->window.get());
    }

    void window::set_should_close(bool yesno) noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwSetWindowShouldClose(state_->window.get(), yesno ? GLFW_TRUE : GLFW_FALSE);
    }

    void window::bind_context() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(state_->window);
        glfwMakeContextCurrent(state_->window.get());
    }

    void window::swap_buffers() noexcept {
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        E2D_ASSERT(
            state_->window &&
            state_->window.get() == glfwGetCurrentContext());
        glfwSwapBuffers(state_->window.get());
    }

    bool window::poll_events() noexcept {
        return glfw_state::poll_events();
    }

    window::event_listener& window::register_event_listener(event_listener_uptr listener) {
        E2D_ASSERT(listener);
        std::lock_guard<std::recursive_mutex> guard(state_->rmutex);
        state_->listeners.push_back(std::move(listener));
        return state_->on_new_event_listener(*state_->listeners.back());
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

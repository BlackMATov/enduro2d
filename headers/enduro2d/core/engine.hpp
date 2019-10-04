/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_core.hpp"

namespace e2d
{
    //
    // engine
    //

    class engine final : public module<engine> {
    public:
        class application;
        using application_uptr = std::unique_ptr<application>;
    public:
        class debug_parameters;
        class window_parameters;
        class timer_parameters;
        class parameters;
    public:
        engine(int argc, char *argv[], const parameters& params);
        ~engine() noexcept final;

        template < typename Application, typename... Args >
        bool start(Args&&... args);
        bool start(application_uptr app);

        secf time() const noexcept;
        secf delta_time() const noexcept;

        u32 frame_rate() const noexcept;
        u32 frame_count() const noexcept;
        secf realtime_time() const noexcept;
    private:
        class internal_state;
        std::unique_ptr<internal_state> state_;
    };

    //
    // engine::application
    //

    class engine::application : private e2d::noncopyable {
    public:
        virtual ~application() noexcept = default;
        virtual bool initialize();
        virtual void shutdown() noexcept;
        virtual bool frame_tick();
        virtual void frame_render();
    };

    //
    // engine::debug_parameters
    //

    class engine::debug_parameters {
    public:
        debug_parameters& log_filename(str_view value);
        debug_parameters& file_logging(bool value) noexcept;
        debug_parameters& console_logging(bool value) noexcept;

        const str& log_filename() const noexcept;
        bool file_logging() const noexcept;
        bool console_logging() const noexcept;
    private:
        str log_filename_{"log.txt"};
        bool file_logging_{true};
        bool console_logging_{true};
    };

    //
    // engine::window_parameters
    //

    class engine::window_parameters {
    public:
        window_parameters& caption(str_view value);
        window_parameters& size(const v2u& value) noexcept;
        window_parameters& vsync(bool value) noexcept;
        window_parameters& fullscreen(bool value) noexcept;

        const str& caption() const noexcept;
        const v2u& size() const noexcept;
        bool vsync() const noexcept;
        bool fullscreen() const noexcept;
    private:
        str caption_{"Enduro2D"};
        v2u size_{640, 480};
        bool vsync_{false};
        bool fullscreen_{false};
    };

    //
    // engine::timer_parameters
    //

    class engine::timer_parameters {
    public:
        timer_parameters& minimal_framerate(u32 value) noexcept;
        timer_parameters& maximal_framerate(u32 value) noexcept;

        u32 minimal_framerate() const noexcept;
        u32 maximal_framerate() const noexcept;
    private:
        u32 minimal_framerate_{30u};
        u32 maximal_framerate_{1000u};
    };

    //
    // engine::parameters
    //

    class engine::parameters {
    public:
        parameters() = delete;
        parameters(str_view game_name, str_view company_name);

        parameters& game_name(str_view value);
        parameters& company_name(str_view value);
        parameters& without_audio(bool value);
        parameters& without_network(bool value);
        parameters& without_graphics(bool value);
        parameters& debug_params(const debug_parameters& value);
        parameters& window_params(const window_parameters& value);
        parameters& timer_params(const timer_parameters& value);

        str& game_name() noexcept;
        str& company_name() noexcept;
        bool& without_audio() noexcept;
        bool& without_network() noexcept;
        bool& without_graphics() noexcept;
        debug_parameters& debug_params() noexcept;
        window_parameters& window_params() noexcept;
        timer_parameters& timer_params() noexcept;

        const str& game_name() const noexcept;
        const str& company_name() const noexcept;
        const bool& without_audio() const noexcept;
        const bool& without_network() const noexcept;
        const bool& without_graphics() const noexcept;
        const debug_parameters& debug_params() const noexcept;
        const window_parameters& window_params() const noexcept;
        const timer_parameters& timer_params() const noexcept;
    private:
        str game_name_{"noname"};
        str company_name_{"noname"};
        bool without_audio_{false};
        bool without_network_{false};
        bool without_graphics_{false};
        debug_parameters debug_params_;
        window_parameters window_params_;
        timer_parameters timer_params_;
    };
}

namespace e2d
{
    template < typename Application, typename... Args >
    bool engine::start(Args&&... args) {
        return start(std::make_unique<Application>(std::forward<Args>(args)...));
    }
}

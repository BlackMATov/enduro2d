/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    class game final : public engine::application {
    public:
        bool initialize() final {
            the<vfs>().register_scheme<archive_file_source>(
                "rpgaudio",
                the<vfs>().read(url("resources://bin/kenney_rpgaudio.zip")));

            the<vfs>().register_scheme_alias(
                "audio",
                url("rpgaudio://Audio"));
            
            auto sstream1 = the<audio>().preload_stream(the<vfs>().read(url("audio://chop.ogg")));
            auto sstream2 = the<audio>().create_stream(the<vfs>().read(url("audio://footstep00.ogg")));

            if ( !sstream1 || !sstream2 ) {
                return false;
            }

            sound_src1_ = the<audio>().create_source(sstream1);
            sound_src2_ = the<audio>().create_source(sstream2);
            
            if ( !sound_src1_ || !sound_src2_ ) {
                return false;
            }

            sound_src2_->looping(true);
            sound_src2_->play();

            return true;
        }

        bool frame_tick() final {
            const mouse& m = the<input>().mouse();
            const keyboard& k = the<input>().keyboard();

            if ( the<window>().should_close() || k.is_key_just_released(keyboard_key::escape) ) {
                return false;
            }

            if ( k.is_key_just_pressed(keyboard_key::f1) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_pressed(keyboard_key::lsuper) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }

            if ( m.is_any_button_pressed() ) {
                sound_src1_->volume(m.cursor_pos().x / the<window>().real_size().x);
                sound_src1_->play();
            }

            return true;
        }

        void frame_render() final {
            the<render>().begin_pass(render::renderpass_desc()
                .viewport(the<window>().real_size())
                .color_clear({1.f, 0.4f, 0.f, 1.f}));
        }
    private:
        sound_source_ptr sound_src1_;
        sound_source_ptr sound_src2_;
    };
}

int e2d_main(int argc, char *argv[]) {
    auto params = engine::parameters("sample_05", "enduro2d")
        .timer_params(engine::timer_parameters()
            .maximal_framerate(100));
    modules::initialize<engine>(argc, argv, params).start<game>();
    modules::shutdown<engine>();
    return 0;
}

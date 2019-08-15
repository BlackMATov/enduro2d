/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "dbgui.hpp"

namespace
{
    const char* vs_src_cstr = R"glsl(
        attribute vec2 a_position;
        attribute vec2 a_uv;
        attribute vec4 a_color;

    #ifdef E2D_SUPPORTS_UBO
        layout(std140) uniform cb_material {
            mat4 u_MVP;
        };
    #else
        uniform vec4 cb_material[4];
        #define u_MVP mat4(cb_material[0], cb_material[1], cb_material[2], cb_material[3])
    #endif

        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          v_color = a_color;
          v_uv = a_uv;
          gl_Position = vec4(a_position.x, -a_position.y, 0.0, 1.0) * u_MVP;
        }
    )glsl";

    const char* fs_src_cstr = R"glsl(
        uniform sampler2D u_texture;
        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          gl_FragColor = v_color * texture2D(u_texture, v_uv);
        }
    )glsl";
}

namespace e2d::dbgui_shaders
{
    const char* vertex_source_cstr() noexcept {
        return vs_src_cstr;
    }

    const char* fragment_source_cstr() noexcept {
        return fs_src_cstr;
    }
}

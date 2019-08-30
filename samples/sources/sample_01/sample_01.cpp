/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "../common.hpp"
using namespace e2d;

namespace
{
    const char* vs_source_cstr = R"glsl(
        attribute vec3 a_position;
        attribute vec2 a_uv;
        attribute vec4 a_color;

        uniform mat4 u_MVP;

        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          v_color = a_color;
          v_uv = a_uv;

          gl_Position = vec4(a_position, 1.0) * u_MVP;
        }
    )glsl";

    const char* fs_source_cstr = R"glsl(
        uniform sampler2D u_texture;
        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
          gl_FragColor = v_color * texture2D(u_texture, uv);
        }
    )glsl";

    struct vertex1 {
        v3f position;
        v2hu uv;
        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_position")
                .add_attribute<v2hu>("a_uv");
        }
    };

    struct vertex2 {
        color32 color;
        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<color32>("a_color").normalized();
        }
    };

    std::array<u16,36> generate_cube_indices() noexcept {
        return {
            0,  1,  2,
            2,  3,  0,

            4,  5,  6,
            6,  7,  4,

            8,  9, 10,
            10, 11, 8,

            12, 13, 14,
            14, 15, 12,

            16, 17, 18,
            18, 19, 16,

            20, 21, 22,
            22, 23, 20};
    }

    std::array<vertex1,24> generate_cube_vertices(const v3f& size) noexcept {
        f32 x = size.x * 0.5f;
        f32 y = size.y * 0.5f;
        f32 z = size.z * 0.5f;
        return {
            vertex1{{-x, -y, -z}, {0, 1}},
            vertex1{{ x, -y, -z}, {1, 1}},
            vertex1{{ x,  y, -z}, {1, 0}},
            vertex1{{-x,  y, -z}, {0, 0}},

            vertex1{{-x, -y,  z}, {0, 1}},
            vertex1{{ x, -y,  z}, {1, 1}},
            vertex1{{ x, -y, -z}, {1, 0}},
            vertex1{{-x, -y, -z}, {0, 0}},

            vertex1{{ x, -y,  z}, {0, 1}},
            vertex1{{-x, -y,  z}, {1, 1}},
            vertex1{{-x,  y,  z}, {1, 0}},
            vertex1{{ x,  y,  z}, {0, 0}},

            vertex1{{-x,  y, -z}, {0, 1}},
            vertex1{{ x,  y, -z}, {1, 1}},
            vertex1{{ x,  y,  z}, {1, 0}},
            vertex1{{-x,  y,  z}, {0, 0}},

            vertex1{{ x, -y, -z}, {0, 1}},
            vertex1{{ x, -y,  z}, {1, 1}},
            vertex1{{ x,  y,  z}, {1, 0}},
            vertex1{{ x,  y, -z}, {0, 0}},

            vertex1{{-x, -y,  z}, {0, 1}},
            vertex1{{-x, -y, -z}, {1, 1}},
            vertex1{{-x,  y, -z}, {1, 0}},
            vertex1{{-x,  y,  z}, {0, 0}}};
    }

    std::array<vertex2,24> generate_cube_colors() noexcept {
        return {
            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()},

            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()},

            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()},

            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()},

            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()},

            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()}};
    }

    class game final : public engine::application {
    public:
        bool initialize() final {
            the<vfs>().register_scheme<archive_file_source>(
                "piratepack",
                the<vfs>().read(url("resources://bin/kenney_piratepack.zip")));

            the<vfs>().register_scheme_alias(
                "ships",
                url("piratepack://PNG/Retina/Ships"));

            shader_ = the<render>().create_shader(
                vs_source_cstr, fs_source_cstr);

            texture_ = the<render>().create_texture(
                the<vfs>().read(url("ships://ship (3).png")));

            if ( !shader_ || !texture_ ) {
                return false;
            }

            const auto indices = generate_cube_indices();
            index_buffer_ = the<render>().create_index_buffer(
                indices,
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::static_draw);

            const auto vertices1 = generate_cube_vertices(make_vec3(1.f));
            vertex_buffer1_ = the<render>().create_vertex_buffer(
                vertices1,
                vertex1::decl(),
                vertex_buffer::usage::static_draw);

            const auto vertices2 = generate_cube_colors();
            vertex_buffer2_ = the<render>().create_vertex_buffer(
                vertices2,
                vertex2::decl(),
                vertex_buffer::usage::static_draw);

            if ( !index_buffer_ || !vertex_buffer1_ || !vertex_buffer2_ ) {
                return false;
            }

            material_ = render::material()
                .add_pass(render::pass_state()
                    .states(render::state_block()
                        .capabilities(render::capabilities_state()
                            .culling(true)
                            .depth_test(true))
                        .culling(render::culling_state()
                            .mode(render::culling_mode::ccw)
                            .face(render::culling_face::back)))
                    .shader(shader_)
                    .properties(render::property_block()
                        .sampler("u_texture", render::sampler_state()
                            .texture(texture_)
                            .min_filter(render::sampler_min_filter::linear)
                            .mag_filter(render::sampler_mag_filter::linear))));

            geometry_ = render::geometry()
                .indices(index_buffer_)
                .add_vertices(vertex_buffer1_)
                .add_vertices(vertex_buffer2_);

            return true;
        }

        bool frame_tick() final {
            const keyboard& k = the<input>().keyboard();

            if ( the<window>().should_close() || k.is_key_just_released(keyboard_key::escape) ) {
                return false;
            }

            if ( k.is_key_just_pressed(keyboard_key::f12) ) {
                the<dbgui>().toggle_visible(!the<dbgui>().visible());
            }

            if ( k.is_key_pressed(keyboard_key::lsuper) && k.is_key_just_released(keyboard_key::enter) ) {
                the<window>().toggle_fullscreen(!the<window>().fullscreen());
            }

            return true;
        }

        void frame_render() final {
            const auto framebuffer_size = the<window>().real_size().cast_to<f32>();
            const auto projection = math::make_perspective_lh_matrix4(
                make_deg(45.f),
                framebuffer_size.x / framebuffer_size.y,
                0.1f,
                100.f);

            const auto MVP =
                math::make_rotation_matrix4(make_rad(the<engine>().time()), 1.f, 0.f, 0.f) *
                math::make_rotation_matrix4(make_rad(the<engine>().time()), 0.f, 1.f, 0.f) *
                math::make_rotation_matrix4(make_rad(the<engine>().time()), 0.f, 0.f, 1.f) *
                math::make_translation_matrix4(0.f, 0.f, 0.f) *
                math::make_look_at_lh_matrix4({0.f, 0.f, -3.f}, v3f::zero(), v3f::unit_y()) *
                projection;

            material_.properties()
                .property("u_MVP", MVP);

            the<render>().execute(render::command_block<64>()
                .add_command(render::viewport_command(
                    the<window>().real_size()))
                .add_command(render::clear_command()
                    .color_value({1.f, 0.4f, 0.f, 1.f}))
                .add_command(render::draw_command(material_, geometry_)));
        }
    private:
        shader_ptr shader_;
        texture_ptr texture_;
        index_buffer_ptr index_buffer_;
        vertex_buffer_ptr vertex_buffer1_;
        vertex_buffer_ptr vertex_buffer2_;
        render::material material_;
        render::geometry geometry_;
    };
}

int e2d_main(int argc, char *argv[]) {
    auto params = engine::parameters("sample_01", "enduro2d")
        .timer_params(engine::timer_parameters()
            .maximal_framerate(100));
    modules::initialize<engine>(argc, argv, params).start<game>();
    modules::shutdown<engine>();
    return 0;
}

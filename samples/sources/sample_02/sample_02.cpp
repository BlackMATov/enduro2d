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

        uniform mat4 u_MVP;

        varying vec2 v_uv;

        void main(){
          v_uv = a_uv;

          gl_Position = vec4(a_position, 1.0) * u_MVP;
        }
    )glsl";

    const char* fs_source_cstr = R"glsl(
        uniform sampler2D u_texture;
        varying vec2 v_uv;

        void main(){
          vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
          gl_FragColor = texture2D(u_texture, uv);
        }
    )glsl";

    struct vertex {
        v3f position;
        v2hu uv;
        static vertex_declaration decl() noexcept {
            return vertex_declaration()
                .add_attribute<v3f>("a_position")
                .add_attribute<v2hu>("a_uv");
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

    std::array<vertex,24> generate_cube_vertices(const v3f& size) noexcept {
        f32 x = size.x * 0.5f;
        f32 y = size.y * 0.5f;
        f32 z = size.z * 0.5f;
        return {
            vertex{{-x, -y, -z}, {0, 1}},
            vertex{{ x, -y, -z}, {1, 1}},
            vertex{{ x,  y, -z}, {1, 0}},
            vertex{{-x,  y, -z}, {0, 0}},

            vertex{{-x, -y,  z}, {0, 1}},
            vertex{{ x, -y,  z}, {1, 1}},
            vertex{{ x, -y, -z}, {1, 0}},
            vertex{{-x, -y, -z}, {0, 0}},

            vertex{{ x, -y,  z}, {0, 1}},
            vertex{{-x, -y,  z}, {1, 1}},
            vertex{{-x,  y,  z}, {1, 0}},
            vertex{{ x,  y,  z}, {0, 0}},

            vertex{{-x,  y, -z}, {0, 1}},
            vertex{{ x,  y, -z}, {1, 1}},
            vertex{{ x,  y,  z}, {1, 0}},
            vertex{{-x,  y,  z}, {0, 0}},

            vertex{{ x, -y, -z}, {0, 1}},
            vertex{{ x, -y,  z}, {1, 1}},
            vertex{{ x,  y,  z}, {1, 0}},
            vertex{{ x,  y, -z}, {0, 0}},

            vertex{{-x, -y,  z}, {0, 1}},
            vertex{{-x, -y, -z}, {1, 1}},
            vertex{{-x,  y, -z}, {1, 0}},
            vertex{{-x,  y,  z}, {0, 0}}};
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

            const auto vertices = generate_cube_vertices(make_vec3(1.f));
            vertex_buffer_ = the<render>().create_vertex_buffer(
                vertices,
                vertex::decl(),
                vertex_buffer::usage::static_draw);

            render_target_ = the<render>().create_render_target(
                the<window>().real_size() / 10u,
                pixel_declaration::pixel_type::rgba8,
                pixel_declaration::pixel_type::depth16,
                render_target::external_texture::color_and_depth);

            if ( !index_buffer_ || !vertex_buffer_ || !render_target_ ) {
                return false;
            }

            rt_props_ = render::property_block()
                .sampler("u_texture", render::sampler_state()
                    .texture(render_target_->color())
                    .min_filter(render::sampler_min_filter::linear)
                    .mag_filter(render::sampler_mag_filter::linear));

            tex_props_ = render::property_block()
                .sampler("u_texture", render::sampler_state()
                    .texture(texture_)
                    .min_filter(render::sampler_min_filter::linear)
                    .mag_filter(render::sampler_mag_filter::linear));

            material_ = render::material()
                .add_pass(render::pass_state()
                    .states(render::state_block()
                        .capabilities(render::capabilities_state()
                            .blending(true)
                            .culling(true)
                            .depth_test(true))
                        .blending(render::blending_state()
                            .src_factor(render::blending_factor::src_alpha)
                            .dst_factor(render::blending_factor::one_minus_src_alpha))
                        .culling(render::culling_state()
                            .mode(render::culling_mode::ccw)
                            .face(render::culling_face::back)))
                    .shader(shader_))
                .properties(render::property_block()
                    .sampler("u_texture", render::sampler_state()
                        .texture(texture_)
                        .min_filter(render::sampler_min_filter::linear)
                        .mag_filter(render::sampler_mag_filter::linear)));

            geometry_ = render::geometry()
                .indices(index_buffer_)
                .add_vertices(vertex_buffer_);

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
                math::make_look_at_lh_matrix4({0.f, 0.f, -2.f}, v3f::zero(), v3f::unit_y()) *
                projection;

            material_.properties()
                .property("u_MVP", MVP);

            the<render>().execute(render::command_block<64>()
                .add_command(render::target_command(render_target_))
                .add_command(render::viewport_command(render_target_->size()))
                .add_command(render::clear_command()
                    .color_value({0.f, 0.4f, 0.f, 1.f}))
                .add_command(render::draw_command(material_, geometry_, tex_props_)));

            the<render>().execute(render::command_block<64>()
                .add_command(render::target_command(nullptr))
                .add_command(render::clear_command()
                    .color_value(color::blue()))
                .add_command(render::viewport_command(the<window>().real_size())
                    .scissor_rect(make_rect(v2u{100u}, the<window>().real_size() - 200u)))
                .add_command(render::clear_command()
                    .color_value({1.f, 0.4f, 0.f, 1.f}))
                .add_command(render::draw_command(material_, geometry_, rt_props_)));
        }
    private:
        shader_ptr shader_;
        texture_ptr texture_;
        index_buffer_ptr index_buffer_;
        vertex_buffer_ptr vertex_buffer_;
        render_target_ptr render_target_;
        render::property_block rt_props_;
        render::property_block tex_props_;
        render::material material_;
        render::geometry geometry_;
    };
}

int e2d_main(int argc, char *argv[]) {
    auto params = engine::parameters("sample_02", "enduro2d")
        .timer_params(engine::timer_parameters()
            .maximal_framerate(100));
    modules::initialize<engine>(argc, argv, params).start<game>();
    modules::shutdown<engine>();
    return 0;
}

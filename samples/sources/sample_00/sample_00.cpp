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

    #ifdef E2D_SUPPORTS_UBO
        layout(std140) uniform cb_pass {
            mat4 u_MVP;
            float u_time;
        };
    #else
        uniform vec4 cb_pass[5];
        #define u_MVP mat4(cb_pass[0], cb_pass[1], cb_pass[2], cb_pass[3])
        #define u_time cb_pass[4].x
    #endif

        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          v_color = a_color;
          v_uv = a_uv;

          float s = 0.7 + 0.3 * (cos(u_time * 3.0) + 1.0);
          gl_Position = vec4(a_position * s, 1.0) * u_MVP;
        }
    )glsl";

    const char* fs_source_cstr = R"glsl(
    #ifdef E2D_SUPPORTS_UBO
        layout(std140) uniform cb_pass {
            mat4 u_MVP;
            float u_time;
        };
    #else
        uniform vec4 cb_pass[5];
        #define u_time cb_pass[4].x
    #endif

        uniform sampler2D u_texture1;
        uniform sampler2D u_texture2;
        varying vec4 v_color;
        varying vec2 v_uv;

        void main(){
          vec2 uv = vec2(v_uv.s, 1.0 - v_uv.t);
          if ( u_time > 2.0 ) {
            gl_FragColor = v_color * texture2D(u_texture2, uv);
          } else {
            gl_FragColor = v_color * texture2D(u_texture1, uv);
          }
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

    std::array<u16,6> generate_quad_indices() noexcept {
        return {0, 1, 2, 2, 1, 3};
    }

    std::array<vertex1,4> generate_quad_vertices(const v2u& size) noexcept {
        f32 hw = size.x * 0.5f;
        f32 hh = size.y * 0.5f;
        return {
            vertex1{{-hw,  hh, 0.f}, {0, 1}},
            vertex1{{-hw, -hh, 0.f}, {0, 0}},
            vertex1{{ hw,  hh, 0.f}, {1, 1}},
            vertex1{{ hw, -hh, 0.f}, {1, 0}}};
    }

    std::array<vertex2,4> generate_quad_colors() noexcept {
        return {
            vertex2{color32::red()},
            vertex2{color32::green()},
            vertex2{color32::blue()},
            vertex2{color32::yellow()}};
    }

    class game final : public engine::application {
    public:
        bool initialize() final {
            auto per_pass_cb = std::make_shared<cbuffer_template>();
            (*per_pass_cb)
                .add_uniform("u_MVP", 0, cbuffer_template::value_type::m4f)
                .add_uniform("u_time", 64, cbuffer_template::value_type::f32);
            
            shader_ = the<render>().create_shader(shader_source()
                .vertex_shader(vs_source_cstr)
                .fragment_shader(fs_source_cstr)
                .add_attribute("a_position", 0, shader_source::value_type::v3f)
                .add_attribute("a_uv", 1, shader_source::value_type::v2f)
                .add_attribute("a_color", 2, shader_source::value_type::v4f)
                .set_block(per_pass_cb, shader_source::scope_type::render_pass)
                .add_sampler("u_texture1", 0, shader_source::sampler_type::_2d, shader_source::scope_type::material)
                .add_sampler("u_texture2", 1, shader_source::sampler_type::_2d, shader_source::scope_type::material));

            constants_ = the<render>().create_const_buffer(shader_, const_buffer::scope::render_pass);

            the<vfs>().register_scheme<archive_file_source>(
                "piratepack",
                the<vfs>().read(url("resources://bin/kenney_piratepack.zip")));

            the<vfs>().register_scheme_alias(
                "ships",
                url("piratepack://PNG/Retina/Ships"));

            texture1_ = the<render>().create_texture(
                the<vfs>().read(url("ships://ship (2).png")));
            texture2_ = the<render>().create_texture(
                the<vfs>().read(url("ships://ship (19).png")));

            if ( !shader_ || !texture1_ || !texture2_ || !constants_ ) {
                return false;
            }

            const auto indices = generate_quad_indices();
            index_buffer_ = the<render>().create_index_buffer(
                indices,
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::static_draw);

            const auto vertices1 = generate_quad_vertices(texture1_->size());
            vertex_buffer1_ = the<render>().create_vertex_buffer(
                vertices1,
                vertex_buffer::usage::static_draw);

            const auto vertices2 = generate_quad_colors();
            vertex_buffer2_ = the<render>().create_vertex_buffer(
                vertices2,
                vertex_buffer::usage::static_draw);

            if ( !index_buffer_ || !vertex_buffer1_ || !vertex_buffer2_ ) {
                return false;
            }
            
            vertex_attribs1_ = the<render>().create_vertex_attribs(vertex1::decl());
            vertex_attribs2_ = the<render>().create_vertex_attribs(vertex2::decl());

            if ( !vertex_attribs1_ || !vertex_attribs2_ ) {
                return false;
            }

            return true;
        }

        bool frame_tick() final {
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

            return true;
        }

        void frame_render() final {
            const auto framebuffer_size = the<window>().real_size().cast_to<f32>();
            const auto projection = math::make_orthogonal_lh_matrix4(
                framebuffer_size, 0.f, 1.f);

            the<render>().update_buffer(
                constants_,
                render::property_map()
                    .property("u_time", the<engine>().time())
                    .property("u_MVP", projection));

            the<render>().begin_pass(
                render::renderpass_desc()
                    .viewport(the<window>().real_size())
                    .color_clear({1.f, 0.4f, 0.f, 1.f})
                    .color_store(),
                constants_);

            the<render>().set_material(render::material()
                .shader(shader_)
                .sampler("u_texture1", render::sampler_state()
                    .texture(texture1_)
                    .min_filter(render::sampler_min_filter::linear)
                    .mag_filter(render::sampler_mag_filter::linear))
                .sampler("u_texture2", render::sampler_state()
                    .texture(texture2_)
                    .min_filter(render::sampler_min_filter::linear)
                    .mag_filter(render::sampler_mag_filter::linear))
                .blending(render::blending_state()
                    .enable(true)
                    .src_factor(render::blending_factor::src_alpha)
                    .dst_factor(render::blending_factor::one_minus_src_alpha))
            );

            the<render>().execute(render::bind_vertex_buffers_command()
                .add(vertex_buffer1_, vertex_attribs1_)
                .add(vertex_buffer2_, vertex_attribs2_));

            the<render>().execute(render::draw_indexed_command()
                .index_count(index_buffer_->index_count())
                .indices(index_buffer_));

            the<render>().end_pass();
        }
    private:
        shader_ptr shader_;
        texture_ptr texture1_;
        texture_ptr texture2_;
        index_buffer_ptr index_buffer_;
        vertex_buffer_ptr vertex_buffer1_;
        vertex_buffer_ptr vertex_buffer2_;
        vertex_attribs_ptr vertex_attribs1_;
        vertex_attribs_ptr vertex_attribs2_;
        const_buffer_ptr constants_;
    };
}

int e2d_main(int argc, char *argv[]) {
    auto params = engine::parameters("sample_00", "enduro2d")
        .timer_params(engine::timer_parameters()
            .maximal_framerate(100));
    modules::initialize<engine>(argc, argv, params).start<game>();
    modules::shutdown<engine>();
    return 0;
}

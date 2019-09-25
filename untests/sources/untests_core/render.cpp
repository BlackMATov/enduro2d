/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_core.hpp"
using namespace e2d;

namespace
{
    class safe_engine_initializer final : private noncopyable {
    public:
        safe_engine_initializer() {
            modules::initialize<engine>(0, nullptr,
                engine::parameters("renderer_untests", "enduro2d"));
        }

        ~safe_engine_initializer() noexcept {
            modules::shutdown<engine>();
        }
    };
}

TEST_CASE("render"){
    safe_engine_initializer initializer;
    SECTION("sampler_state"){
        {
            const auto ss = render::sampler_state();
            REQUIRE_FALSE(ss.texture());
            REQUIRE(ss.s_wrap() == render::sampler_wrap::repeat);
            REQUIRE(ss.t_wrap() == render::sampler_wrap::repeat);
            REQUIRE(ss.min_filter() == render::sampler_min_filter::linear);
            REQUIRE(ss.mag_filter() == render::sampler_mag_filter::linear);
        }
        {
            const auto ss = render::sampler_state()
                .wrap(render::sampler_wrap::clamp, render::sampler_wrap::mirror)
                .filter(render::sampler_min_filter::linear, render::sampler_mag_filter::nearest);
            REQUIRE(ss.s_wrap() == render::sampler_wrap::clamp);
            REQUIRE(ss.t_wrap() == render::sampler_wrap::mirror);
            REQUIRE(ss.min_filter() == render::sampler_min_filter::linear);
            REQUIRE(ss.mag_filter() == render::sampler_mag_filter::nearest);
        }
    }
    SECTION("property_block"){
        {
            const auto pb1 = render::property_block()
                .property("f", 1.f)
                .property("i", 42);
            REQUIRE(*pb1.property<f32>("f") == 1.f);
            REQUIRE(*pb1.property<i32>("i") == 42);
            REQUIRE_FALSE(pb1.property<f32>("i"));
            REQUIRE_FALSE(pb1.property<i32>("f"));
            REQUIRE(pb1.property("i"));
            REQUIRE(pb1.property("f"));
            REQUIRE(std::get<i32>(*pb1.property("i")) == 42);
            REQUIRE(std::get<f32>(*pb1.property("f")) == 1.f);
            {
                f32 acc = 0.f;
                pb1.foreach_by_properties([&acc](str_hash n, const render::property_value& v){
                    if ( n == make_hash("f") ) {
                        acc += std::get<f32>(v);
                    }
                    if ( n == make_hash("i") ) {
                        acc += std::get<i32>(v);
                    }
                });
                REQUIRE(math::approximately(acc, 43.f));
            }
        }
        {
            const auto pb1 = render::property_block()
                .property("f", 1.f)
                .property("i", 42);
            const auto pb2 = render::property_block()
                .property("ff", 2.f)
                .property("ii", 84);
            auto pb3 = render::property_block()
                .merge(pb1)
                .merge(pb2);
            REQUIRE(pb3.property_count() == 4);
            REQUIRE(pb3.property("i"));
            REQUIRE(pb3.property("f"));
            REQUIRE(pb3.property("ii"));
            REQUIRE(pb3.property("ff"));
            pb3.clear();
            REQUIRE(pb3.property_count() == 0);
            REQUIRE_FALSE(pb3.property("i"));
            REQUIRE_FALSE(pb3.property("f"));
            REQUIRE_FALSE(pb3.property("ii"));
            REQUIRE_FALSE(pb3.property("ff"));
        }
        {
            const auto pb1 = render::property_block()
                .property("i", 42)
                .property("f", 1.f);
            const auto pb2 = render::property_block()
                .property("i", 40)
                .property("ii", 20)
                .merge(pb1);
            REQUIRE(pb2.property_count() == 3);
            REQUIRE(*pb2.property<i32>("i") == 42);
            REQUIRE(*pb2.property<i32>("ii") == 20);
            REQUIRE(*pb2.property<f32>("f") == 1.f);
        }
    }
    SECTION("index_declaration"){
        index_declaration id;
        REQUIRE(id.type() == index_declaration::index_type::unsigned_short);
        REQUIRE(id.bytes_per_index() == 2);

        index_declaration id2(index_declaration::index_type::unsigned_short);
        REQUIRE(id2.type() == index_declaration::index_type::unsigned_short);
        REQUIRE(id2.bytes_per_index() == 2);

        REQUIRE(id == id2);
        REQUIRE_FALSE(id != id2);

        index_declaration id4 = id;
        REQUIRE(id4 == id2);
    }
    SECTION("vertex_declaration"){
        vertex_declaration vd;
        REQUIRE(vd.attribute_count() == 0);
        REQUIRE(vd.bytes_per_vertex() == 0);

        REQUIRE(&vd == &vd.add_attribute<v2f>("hello"));
        REQUIRE(vd.attribute_count() == 1);
        REQUIRE(vd.bytes_per_vertex() == 8);

        vertex_declaration::attribute_info ai = vd.attribute(0);
        REQUIRE(ai.name == make_hash("hello"));
        REQUIRE(ai.columns == 2);
        REQUIRE(ai.stride == 0);
        REQUIRE(ai.type == vertex_declaration::attribute_type::floating_point);
        REQUIRE_FALSE(ai.normalized);

        REQUIRE(&vd == &vd.skip_bytes(4));
        REQUIRE(vd.bytes_per_vertex() == 12);

        REQUIRE(&vd == &vd.add_attribute<u16>("world").normalized());
        REQUIRE(vd.attribute_count() == 2);
        REQUIRE(vd.bytes_per_vertex() == 14);

        vertex_declaration::attribute_info ai2 = vd.attribute(1);
        REQUIRE(ai2.name == make_hash("world"));
        REQUIRE(ai2.columns == 1);
        REQUIRE(ai2.stride == 12);
        REQUIRE(ai2.type == vertex_declaration::attribute_type::unsigned_short);
        REQUIRE(ai2.normalized);

        auto vd2 = vertex_declaration()
            .add_attribute("hello", 1, 2, vertex_declaration::attribute_type::floating_point, false)
            .skip_bytes(4)
            .add_attribute("world", 1, 1, vertex_declaration::attribute_type::unsigned_short, true);
        REQUIRE(vd == vd2);
        REQUIRE_FALSE(vd != vd2);

        auto vd3 = vertex_declaration()
            .add_attribute<v2f>("hello")
            .add_attribute<v2f>("world");
        REQUIRE_FALSE(vd == vd3);
        REQUIRE(vd != vd3);

        REQUIRE(vd3.bytes_per_vertex() == 16);
        vd3.skip_bytes(4);
        REQUIRE(vd3.bytes_per_vertex() == 20);

        vertex_declaration vd4 = vd2;
        REQUIRE(vd4 == vd);
        vd4 = vd3;
        REQUIRE(vd4 != vd);
        REQUIRE(vd4 == vd3);
    }
    SECTION("update_texture"){
        if ( modules::is_initialized<render>() ) {
            render& r = the<render>();
            {
                texture_ptr tex = r.create_texture(v2u(128,128), pixel_declaration::pixel_type::rgba8);
                REQUIRE(tex != nullptr);

                buffer src(tex->size().x * tex->size().y * 4u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }
                REQUIRE_NOTHROW(r.update_texture(tex, src, b2u(0, 0, 128, 128)));
            }
            {
                texture_ptr tex = r.create_texture(v2u(128,128), pixel_declaration::pixel_type::l8);
                REQUIRE(tex != nullptr);

                buffer src(tex->size().x * tex->size().y * 1u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }
                REQUIRE_NOTHROW(r.update_texture(tex, src, b2u(0, 0, 128, 128)));
            }
            {
                texture_ptr tex = r.create_texture(v2u(128,128), pixel_declaration::pixel_type::rgb8);
                REQUIRE(tex != nullptr);

                buffer src(tex->size().x * tex->size().y * 3u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }
                REQUIRE_NOTHROW(r.update_texture(tex, src, b2u(0, 0, 128, 128)));
            }
            {
                texture_ptr tex = r.create_texture(v2u(57,31), pixel_declaration::pixel_type::rgba8);
                REQUIRE(tex != nullptr);

                buffer src(tex->size().x * tex->size().y * 4u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }
                REQUIRE_NOTHROW(r.update_texture(tex, src, b2u(0, 0, 57, 31)));
            }
            {
                texture_ptr tex = r.create_texture(v2u(128,128), pixel_declaration::pixel_type::rgba8);
                REQUIRE(tex != nullptr);

                buffer src(31u * 44u * 4u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }
                REQUIRE_NOTHROW(r.update_texture(tex, src, b2u(22, 17, 31, 44)));
            }
            {
                texture_ptr tex = r.create_texture(v2u(128,128), pixel_declaration::pixel_type::rgba8);
                REQUIRE(tex != nullptr);

                buffer src(31u * 44u * 4u);
                for ( auto& c : src ) {
                    c = rand() % 255;
                }

                image img(v2u(31, 44), image_data_format::la8, src);
                REQUIRE_THROWS_AS(
                    r.update_texture(tex, img, v2u(11,27)),
                    bad_render_operation);
            }

            if ( r.device_capabilities().dxt_compression_supported ) {
                str resources;
                REQUIRE(filesystem::extract_predef_path(
                    resources,
                    filesystem::predef_path::resources));

                image src;
                REQUIRE(images::try_load_image(src, make_read_file(path::combine(resources, "bin/images/dds/ship_dxt5.dds"))));

                texture_ptr tex = r.create_texture(src.size(), pixel_declaration::pixel_type::rgba_dxt5);
                REQUIRE(tex != nullptr);
                REQUIRE_NOTHROW(r.update_texture(tex, src, v2u(0,0)));
            }
        }
    }
}

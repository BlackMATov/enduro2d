/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_high.hpp"
using namespace e2d;

namespace
{
    class safe_starter_initializer final : private noncopyable {
    public:
        safe_starter_initializer() {
            modules::initialize<starter>(0, nullptr,
                starter::parameters(
                    engine::parameters("library_untests", "enduro2d")));
        }

        ~safe_starter_initializer() noexcept {
            modules::shutdown<starter>();
        }
    };

    class fake_asset final : public content_asset<fake_asset, int> {
    public:
        static const char* type_name() noexcept { return "fake_asset"; }
        static load_async_result load_async(const library& library, str_view address) {
            E2D_UNUSED(library, address);
            return stdex::make_resolved_promise(fake_asset::create(42));
        }
    };

    class big_fake_asset final : public content_asset<big_fake_asset, int> {
    public:
        static const char* type_name() noexcept { return "big_fake_asset"; }
        static load_async_result load_async(const library& library, str_view address) {
            E2D_UNUSED(library, address);
            return the<deferrer>().do_in_worker_thread([](){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                return big_fake_asset::create(42);
            });
        }
    };
}

TEST_CASE("library"){
    safe_starter_initializer initializer;
    library& l = the<library>();
    {
        {
            auto p = l.load_asset_async<fake_asset>("");
            REQUIRE(l.loading_asset_count() == 0);
        }
        REQUIRE(1u == l.unload_unused_assets());
    }
    {
        binary_asset::ptr b1;
        binary_asset::ptr b2;

        {
            auto p1 = l.load_asset_async<binary_asset>("binary_asset.bin");
            auto p2 = l.load_asset_async<binary_asset>("binary_asset.bin");

            the<deferrer>().active_safe_wait_promise(p1);
            the<deferrer>().active_safe_wait_promise(p2);

            b1 = p1.get();
            b2 = p2.get();
            REQUIRE(b1 == b2);
        }

        b1.reset();
        b2.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(1u == l.unload_unused_assets());
        REQUIRE(l.cache().asset_count<binary_asset>() == 0);
    }
    {
        {
            auto p1 = l.load_asset_async<big_fake_asset>("");
            REQUIRE(l.loading_asset_count() == 1);
            the<deferrer>().active_safe_wait_promise(p1);
            REQUIRE(l.loading_asset_count() == 0);

            auto p2 = l.load_asset_async<binary_asset>("none_asset");
            the<deferrer>().active_safe_wait_promise(p2);
            REQUIRE(l.loading_asset_count() == 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(1u == l.unload_unused_assets());
    }
    {
        auto text_res = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res);
        REQUIRE(text_res->content() == "hello");

        auto text_res_from_cache = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res_from_cache);
        REQUIRE(text_res_from_cache.get() == text_res.get());

        REQUIRE(0u == l.unload_unused_assets());
        REQUIRE(l.cache().asset_count() == 1);
        REQUIRE(l.cache().asset_count<text_asset>() == 1);

        text_res.reset();
        text_res_from_cache.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(1u == l.unload_unused_assets());
        REQUIRE(l.cache().asset_count() == 0);
        REQUIRE(l.cache().asset_count<text_asset>() == 0);
    }
    {
        auto text_res = l.load_asset<text_asset>("text_asset.txt");
        REQUIRE(text_res);
        REQUIRE(text_res->content() == "hello");

        auto binary_res = l.load_asset<binary_asset>("binary_asset.bin");
        REQUIRE(binary_res);
        REQUIRE(binary_res->content() == buffer("world", 5));

        REQUIRE(0u == l.unload_unused_assets());
        REQUIRE(l.cache().asset_count() == 2);

        text_res.reset();
        binary_res.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(2u == l.unload_unused_assets());
        REQUIRE(l.cache().asset_count() == 0);
    }
    {
        auto empty_res = l.load_asset<binary_asset>("empty_asset");
        REQUIRE_FALSE(empty_res);
    }
    {
        auto image_res = l.load_asset<image_asset>("image.png");
        REQUIRE(image_res);
        REQUIRE(!image_res->content().empty());

        REQUIRE(l.cache().find<image_asset>("image.png"));
        REQUIRE(l.cache().find<binary_asset>("image.png"));

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        l.unload_unused_assets();
        REQUIRE(l.cache().find<image_asset>("image.png"));
        REQUIRE_FALSE(l.cache().find<binary_asset>("image.png"));

        image_res.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        l.unload_unused_assets();
        REQUIRE_FALSE(l.cache().find<image_asset>("image.png"));
        REQUIRE_FALSE(l.cache().find<binary_asset>("image.png"));
    }
    {
        if ( modules::is_initialized<audio>() ) {
            auto sound_res = l.load_asset<sound_asset>("sound.json");
            REQUIRE(sound_res);
            REQUIRE(sound_res->content());

            auto music_res = l.load_asset<sound_asset>("music.json");
            REQUIRE(music_res);
            REQUIRE(music_res->content());

            auto fake_sound_res = l.load_asset<sound_asset>("fake_sound.json");
            REQUIRE_FALSE(fake_sound_res);
        }
    }
    {
        if ( modules::is_initialized<render>() ) {
            auto shader_res = l.load_asset<shader_asset>("shader.json");
            REQUIRE(shader_res);
            REQUIRE(shader_res->content());

            auto texture_res = l.load_asset<texture_asset>("image.png");
            REQUIRE(texture_res);
            REQUIRE(texture_res->content());

            auto material_res = l.load_asset<material_asset>("material.json");
            REQUIRE(material_res);

            {
                auto atlas_res = l.load_asset<atlas_asset>("atlas.json");
                REQUIRE(atlas_res);

                REQUIRE(atlas_res->content().texture() == texture_res);

                REQUIRE(atlas_res->find_nested_asset("sprite"));

                sprite_asset::ptr spr = atlas_res->find_nested_asset<sprite_asset>("sprite");
                REQUIRE(spr);
                REQUIRE(spr->content().pivot() == v2f(1.f,2.f));
                REQUIRE(spr->content().texrect() == b2f(5.f,6.f,7.f,8.f));
                REQUIRE(spr->content().texture()== texture_res);
            }

            {
                auto sprite_res = l.load_asset<sprite_asset>("sprite.json");
                REQUIRE(sprite_res);
                REQUIRE(sprite_res->content().pivot() == v2f(1.f, 2.f));
                REQUIRE(sprite_res->content().texrect() == b2f(5.f, 6.f, 7.f, 8.f));
                REQUIRE(sprite_res->content().texture() == texture_res);
            }

            {
                auto flipbook_res = l.load_asset<flipbook_asset>("flipbook.json");
                REQUIRE(flipbook_res);

                REQUIRE(flipbook_res->content().frames().size() == 2);
                REQUIRE(flipbook_res->content().sequences().size() == 2);

                const flipbook::frame* frame_0 = flipbook_res->content().find_frame(0);
                REQUIRE(frame_0);
                REQUIRE(frame_0->sprite);
                REQUIRE(frame_0->sprite == l.load_asset<atlas_asset, sprite_asset>("atlas.json:/sprite"));

                const flipbook::frame* frame_1 = flipbook_res->content().find_frame(1);
                REQUIRE(frame_1);
                REQUIRE(frame_1->sprite);
                REQUIRE(frame_1->sprite == l.load_asset<sprite_asset>("sprite.json"));

                const flipbook::sequence* sequence_0 = flipbook_res->content().find_sequence("sequence_0");
                REQUIRE(sequence_0);
                REQUIRE(sequence_0->name == make_hash("sequence_0"));
                REQUIRE(math::approximately(sequence_0->fps, 24.f));
                REQUIRE(sequence_0->frames == vector<std::size_t>{0, 1, 0, 1});

                const flipbook::sequence* sequence_1 = flipbook_res->content().find_sequence("sequence_1");
                REQUIRE(sequence_1);
                REQUIRE(sequence_1->name == make_hash("sequence_1"));
                REQUIRE(math::approximately(sequence_1->fps, 30.f));
                REQUIRE(sequence_1->frames == vector<std::size_t>{1, 0, 0, 1});
            }

            {
                auto model_res = l.load_asset<model_asset>("model.json");
                REQUIRE(model_res);
                REQUIRE(model_res->content().mesh());
                REQUIRE_FALSE(model_res->content().mesh()->content().vertices().empty());
                REQUIRE(model_res->content().mesh()->content().indices_submesh_count() == 1);
                REQUIRE_FALSE(model_res->content().mesh()->content().indices(0).empty());
            }

            {
                auto prefab_res = l.load_asset<prefab_asset>("prefab.json");
                REQUIRE(prefab_res);
            }
        }
    }
    {
        if ( modules::is_initialized<render>() ) {
            auto material_res = l.load_asset<material_asset>("material.json");
            REQUIRE(material_res);
            {
                auto texture_res = l.load_asset<texture_asset>("image.png");
                REQUIRE(texture_res);
                REQUIRE(texture_res->content());

                const auto* sampler = material_res->content().properties().sampler("s");
                REQUIRE(sampler);
                REQUIRE(sampler->s_wrap() == render::sampler_wrap::clamp);
                REQUIRE(sampler->t_wrap() == render::sampler_wrap::repeat);
                REQUIRE(sampler->min_filter() == render::sampler_min_filter::linear);
                REQUIRE(sampler->mag_filter() == render::sampler_mag_filter::linear);
                REQUIRE(texture_res->content() == sampler->texture());
            }
            {
                const auto* property = material_res->content().properties().property("i");
                REQUIRE(property);
                REQUIRE(property->index() == 0);
                REQUIRE(std::get<i32>(*property) == 42);
            }
            REQUIRE(material_res->content().pass_count() == 1);
            const auto& pass = material_res->content().pass(0);
            {
                const auto* property = pass.properties().property("f");
                REQUIRE(property);
                REQUIRE(property->index() == 1);
                REQUIRE(math::approximately(std::get<f32>(*property), 4.2f));
            }
            {
                const auto* property = pass.properties().property("v1");
                REQUIRE(property);
                REQUIRE(property->index() == 2);
                REQUIRE(std::get<v2i>(*property) == v2i(1,2));
            }
            {
                const auto* property = pass.properties().property("v2");
                REQUIRE(property);
                REQUIRE(property->index() == 6);
                REQUIRE(std::get<v3f>(*property) == v3f(3.f));
            }
            {
                const auto* property = pass.properties().property("v3");
                REQUIRE(property);
                REQUIRE(property->index() == 4);
                REQUIRE(std::get<v4i>(*property) == v4i(1,2,3,4));
            }
            {
                const auto* property = pass.properties().property("m1");
                REQUIRE(property);
                REQUIRE(property->index() == 8);
                REQUIRE(std::get<m2f>(*property) == m2f(1,2,3,4));

                const auto* property2 = pass.properties().property("m4");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                const auto* property = pass.properties().property("m2");
                REQUIRE(property);
                REQUIRE(property->index() == 9);
                REQUIRE(std::get<m3f>(*property) == m3f(1,2,3,4,5,6,7,8,9));

                const auto* property2 = pass.properties().property("m5");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                const auto* property = pass.properties().property("m3");
                REQUIRE(property);
                REQUIRE(property->index() == 10);
                REQUIRE(std::get<m4f>(*property) == m4f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16));

                const auto* property2 = pass.properties().property("m6");
                REQUIRE(property2);
                REQUIRE(*property2 == *property);
            }
            {
                REQUIRE(pass.states().depth().range_near() == 1.f);
                REQUIRE(pass.states().depth().range_far() == 2.f);
                REQUIRE(pass.states().depth().write() == false);
                REQUIRE(pass.states().depth().func() == render::compare_func::greater);

                REQUIRE(pass.states().stencil().write() == 2u);
                REQUIRE(pass.states().stencil().func() == render::compare_func::never);
                REQUIRE(pass.states().stencil().ref() == 4u);
                REQUIRE(pass.states().stencil().mask() == 5u);
                REQUIRE(pass.states().stencil().pass() == render::stencil_op::incr);
                REQUIRE(pass.states().stencil().sfail() == render::stencil_op::decr);
                REQUIRE(pass.states().stencil().zfail() == render::stencil_op::invert);

                REQUIRE(pass.states().culling().mode() == render::culling_mode::cw);
                REQUIRE(pass.states().culling().face() == render::culling_face::front);

                REQUIRE(pass.states().capabilities().culling());
                REQUIRE(pass.states().capabilities().blending());
                REQUIRE(pass.states().capabilities().depth_test());
                REQUIRE(pass.states().capabilities().stencil_test());

                REQUIRE(pass.states().blending().constant_color() == color::white());
                REQUIRE(pass.states().blending().color_mask() == render::blending_color_mask::gba);

                REQUIRE(pass.states().blending().src_rgb_factor() == render::blending_factor::dst_alpha);
                REQUIRE(pass.states().blending().src_alpha_factor() == render::blending_factor::dst_alpha);

                REQUIRE(pass.states().blending().dst_rgb_factor() == render::blending_factor::dst_color);
                REQUIRE(pass.states().blending().dst_alpha_factor() == render::blending_factor::src_color);

                REQUIRE(pass.states().blending().rgb_equation() == render::blending_equation::subtract);
                REQUIRE(pass.states().blending().alpha_equation() == render::blending_equation::reverse_subtract);
            }
        }
    }
}

TEST_CASE("asset_dependencies") {
    safe_starter_initializer initializer;
    library& l = the<library>();
    {
        auto ad = asset_dependencies()
            .add_dependency<text_asset>("text_asset.txt")
            .add_dependency<binary_asset>("binary_asset.bin");
        auto g1_p = ad.load_async(l);
        the<deferrer>().active_safe_wait_promise(g1_p);
        asset_group g1 = g1_p.get();
        REQUIRE(g1.find_asset<text_asset>("text_asset.txt"));
        REQUIRE(g1.find_asset<binary_asset>("binary_asset.bin"));

        ad.add_dependency<text_asset>("none_asset");
        auto g2_p = ad.load_async(l);
        the<deferrer>().active_safe_wait_promise(g2_p);
    }
    {
        if ( modules::is_initialized<render>() ) {
            auto ad = asset_dependencies()
                .add_dependency<atlas_asset>("atlas.json:/sprite");
            auto g1_p = ad.load_async(l);
            the<deferrer>().active_safe_wait_promise(g1_p);
            asset_group g1 = g1_p.get();
            REQUIRE(g1.find_asset<atlas_asset>("atlas.json")
                == l.load_asset<atlas_asset>("atlas.json"));
            REQUIRE(g1.find_asset<atlas_asset, sprite_asset>("atlas.json:/sprite")
                == l.load_asset<atlas_asset, sprite_asset>("atlas.json:/sprite"));

            ad.add_dependency<sprite_asset>("atlas.json:/sprite");
            auto g2_p = ad.load_async(l);
            the<deferrer>().active_safe_wait_promise(g2_p);
        }
    }
}

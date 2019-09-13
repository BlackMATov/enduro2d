/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/label_system.hpp>

#include <enduro2d/high/assets/font_asset.hpp>
#include <enduro2d/high/assets/texture_asset.hpp>

#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/label.hpp>
#include <enduro2d/high/components/renderer.hpp>
#include <enduro2d/high/components/model_renderer.hpp>

namespace
{
    using namespace e2d;

    class geometry_builder {
    public:
        geometry_builder() = default;

        void add_quad(
            const b2f& rect,
            const b2f& texrect,
            const color32& color)
        {
            const u32 start_vertex = math::numeric_cast<u32>(vertices_.size());

            // Y
            // ^
            // | 3 - 2
            // | | / |
            // | 0 - 1
            // +------> X

            {
                const v2f& gp = rect.position;
                const v2f& gs = rect.size;

                vertices_.emplace_back(gp.x + 0.0f, gp.y + 0.0f, 0.f);
                vertices_.emplace_back(gp.x + gs.x, gp.y + 0.0f, 0.f);
                vertices_.emplace_back(gp.x + gs.x, gp.y + gs.y, 0.f);
                vertices_.emplace_back(gp.x + 0.0f, gp.y + gs.y, 0.f);
            }

            {
                const v2f& tp = texrect.position;
                const v2f& ts = texrect.size;

                uvs_.emplace_back(tp.x + 0.0f, tp.y + 0.0f);
                uvs_.emplace_back(tp.x + ts.x, tp.y + 0.0f);
                uvs_.emplace_back(tp.x + ts.x, tp.y + ts.y);
                uvs_.emplace_back(tp.x + 0.0f, tp.y + ts.y);
            }

            {
                indices_.emplace_back(start_vertex + 0u);
                indices_.emplace_back(start_vertex + 1u);
                indices_.emplace_back(start_vertex + 2u);
                indices_.emplace_back(start_vertex + 2u);
                indices_.emplace_back(start_vertex + 3u);
                indices_.emplace_back(start_vertex + 0u);
            }

            {
                colors_.emplace_back(color);
                colors_.emplace_back(color);
                colors_.emplace_back(color);
                colors_.emplace_back(color);
            }
        }

        void update_model(model_renderer& mr) {
            mesh new_mesh;
            new_mesh.set_vertices(vertices_);
            new_mesh.set_indices(0, indices_);
            new_mesh.set_uvs(0, uvs_);
            new_mesh.set_colors(0, colors_);

            model new_model;
            new_model.set_mesh(mesh_asset::create(std::move(new_mesh)));
            new_model.regenerate_geometry(the<render>());

            if ( mr.model() ) {
                mr.model()->fill(std::move(new_model));
            } else {
                mr.model(model_asset::create(std::move(new_model)));
            }
        }

        void clear() noexcept {
            vertices_.clear();
            uvs_.clear();
            indices_.clear();
            colors_.clear();
        }
    private:
        vector<v3f> vertices_;
        vector<v2f> uvs_;
        vector<u32> indices_;
        vector<color32> colors_;
    };
}

namespace
{
    using namespace e2d;

    f32 calculate_halign_offset(
        label::haligns halign,
        f32 string_width,
        f32 region_width) noexcept
    {
        switch ( halign ) {
        case label::haligns::left:
            return 0.0f;
        case label::haligns::center:
            return (region_width - string_width) * 0.5f;
        case label::haligns::right:
            return region_width - string_width;
        default:
            E2D_ASSERT_MSG(false, "unexpected label halign");
            return 0.f;
        }
    }

    f32 calculate_valign_offset(
        label::valigns valign,
        f32 leading,
        u32 glyph_ascent,
        u32 line_height,
        std::size_t string_count,
        f32 region_height) noexcept
    {
        const f32 label_height = string_count > 1u
            ? line_height + line_height * leading * (string_count - 1u)
            : line_height;

        switch ( valign ) {
        case label::valigns::top:
            return region_height;
        case label::valigns::center:
            return (region_height + label_height) * 0.5f;
        case label::valigns::bottom:
            return label_height;
        case label::valigns::baseline:
            return f32(glyph_ascent);
        default:
            E2D_ASSERT_MSG(false, "unexpected label valign");
            return 0.f;
        }
    }

    std::size_t calculate_string_count(str32_view text) noexcept {
        std::size_t count{0u};
        for ( std::size_t i = 0; i < text.size(); ++i ) {
            if ( text[i] == '\n' ) {
                ++count;
            }
        }
        return count;
    }
}

namespace
{
    using namespace e2d;

    void update_label_material(const label& l, renderer& r) {
        auto texture_res = l.font() && !l.font()->content().empty()
            ? l.font()->find_nested_asset<texture_asset>(l.font()->content().info().atlas_file)
            : nullptr;

        auto texture = texture_res
            ? texture_res->content()
            : nullptr;

        const f32 glyph_dilate = math::clamp(l.glyph_dilate(), -1.f, 1.0f);
        const f32 outline_width = math::clamp(l.outline_width(), 0.f, 1.f - glyph_dilate);
        const v4f outline_color = make_vec4(color(l.outline_color()));

        r.properties(render::property_block()
            .sampler("u_texture", render::sampler_state()
                .texture(texture)
                .min_filter(render::sampler_min_filter::linear)
                .mag_filter(render::sampler_mag_filter::linear))
            .property("u_glyph_dilate", glyph_dilate)
            .property("u_outline_width", outline_width)
            .property("u_outline_color", outline_color));
    }

    void update_label_geometry(
        const v2f& region_size,
        label& l,
        model_renderer& mr,
        geometry_builder& gb)
    {
        if ( !l.font() || l.font()->content().empty() || l.text().empty() ) {
            gb.update_model(mr);
            return;
        }

        const font& f = l.font()->content();
        const str32& text = make_utf32(l.text());

        //
        // update glyphs
        //

        struct glyph_desc {
            const font::glyph_info* glyph{nullptr};
            f32 kerning{0.f};
        };

        vector<glyph_desc> glyphs;
        glyphs.reserve(text.size());

        for ( std::size_t i = 0, e = text.size(); i < e; ++i ) {
            glyph_desc desc;
            desc.glyph = f.find_glyph(text[i]);
            desc.kerning = i > 0
                ? f.get_kerning(text[i-1], text[i])
                : 0.f;
            glyphs.push_back(std::move(desc));

            if ( !desc.glyph && text[i] != '\n' ) {
                the<debug>().warning("LABEL: Missing font glyph:\n"
                    "--> Code Point: %0",
                    text[i]);
            }
        }

        f32 tracking_width = 0.f;
        if ( const font::glyph_info* sp = f.find_glyph(' '); sp ) {
            tracking_width = sp->advance * l.tracking();
        }

        //
        // update strings
        //

        struct string_desc {
            f32 width{0.f};
            std::size_t start{0};
            std::size_t length{0};

            string_desc(std::size_t start)
            : start(start) {}
        };

        vector<string_desc> strings;
        strings.reserve(calculate_string_count(text));

        f32 last_space_width = 0.f;
        std::size_t last_space_index = std::size_t(-1);

        strings.push_back(string_desc(0u));

        for ( std::size_t i = 0, e = text.size(); i < e; ++i ) {
            const u32 code_point = text[i];
            const glyph_desc& glyph = glyphs[i];

            if ( code_point == ' ' ) {
                last_space_width = strings.back().width;
                last_space_index = i;
            }

            bool new_line = false;

            if ( code_point == '\n' ) {
                new_line = true;
                strings.back().length = i - strings.back().start;
            } else if ( glyph.glyph ) {
                strings.back().width +=
                    glyph.kerning +
                    glyph.glyph->advance +
                    tracking_width;

                const bool break_line =
                    l.text_width() > 0.f &&
                    last_space_index != std::size_t(-1) &&
                    strings.back().width > l.text_width();

                if ( break_line ) {
                    new_line = true;
                    strings.back().width = last_space_width;
                    strings.back().length = last_space_index - strings.back().start;
                    i = last_space_index;
                }
            }

            if ( i == e - 1 ) {
                new_line = true;
                strings.back().length = i + 1 - strings.back().start;
            }

            if ( new_line ) {
                if ( i < e - 1 ) {
                    strings.push_back(string_desc(i + 1u));
                }
                last_space_width = 0.f;
                last_space_index = std::size_t(-1);
            }
        }

        //
        // update geometry
        //

        const f32 max_y = calculate_valign_offset(
            l.valign(),
            l.leading(),
            f.info().glyph_ascent,
            f.info().line_height,
            strings.size(),
            region_size.y);
        v2f cursor = v2f::unit_y() * max_y;
        v2f x_range = v2f(1.0f, -1.0f) * std::numeric_limits<f32>::max();

        for ( std::size_t i = 0, ie = strings.size(); i < ie; ++i ) {
            cursor.x = calculate_halign_offset(l.halign(), strings[i].width, region_size.x);
            x_range[0] = math::min(x_range[0], cursor.x);
            for ( std::size_t j = strings[i].start, je = strings[i].start + strings[i].length; j < je; ++j ) {
                const glyph_desc& glyph = glyphs[j];
                if ( !glyph.glyph ) {
                    continue;
                }

                cursor.x += glyph.kerning;

                b2f rect = make_rect(cursor, glyph.glyph->tex_rect.size.cast_to<f32>());
                rect.position += glyph.glyph->offset.cast_to<f32>();

                b2f texrect = glyph.glyph->tex_rect.cast_to<f32>();
                texrect.position /= f.info().atlas_size.cast_to<f32>();
                texrect.size /= f.info().atlas_size.cast_to<f32>();

                gb.add_quad(rect, texrect, l.tint());
                cursor.x += glyph.glyph->advance + tracking_width;
            }
            cursor.y -= f.info().line_height * l.leading();
            x_range[1] = math::max(x_range[1], cursor.x);
        }
        l.set_preferred_size({x_range[1] - x_range[0], max_y - cursor.y});

        //
        // update model
        //

        gb.update_model(mr);
    }

    void update_dirty_labels(ecs::registry& owner) {
        geometry_builder gb;
        owner.for_joined_components<label::dirty, label, renderer, model_renderer, actor>([&gb](
            const ecs::const_entity&,
            const label::dirty&,
            label& l,
            renderer& r,
            model_renderer& mr,
            const actor& act
        ){
            update_label_material(l, r);
            v2f size = act.node() ? act.node()->size() : v2f();
            update_label_geometry(size, l, mr, gb);
            gb.clear();
        });
        owner.remove_all_components<label::dirty>();
    }
}

namespace e2d
{
    //
    // label_system::internal_state
    //

    class label_system::internal_state final : private noncopyable {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner) {
            update_dirty_labels(owner);
        }
    };

    //
    // label_system
    //

    label_system::label_system()
    : state_(new internal_state()) {}
    label_system::~label_system() noexcept = default;

    void label_system::process(ecs::registry& owner) {
        state_->process(owner);
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "dbgui_impl/dbgui.hpp"

namespace
{
    using namespace e2d;

    class imgui_event_listener final : public window::event_listener {
    public:
        imgui_event_listener(ImGuiIO& io, window& w)
        : io_(io)
        , window_(w) {}

        void on_input_char(char32_t uchar) noexcept final {
            if ( uchar <= std::numeric_limits<ImWchar>::max() ) {
                io_.AddInputCharacter(static_cast<ImWchar>(uchar));
            }
        }

        void on_move_cursor(const v2f& pos) noexcept final {
            const v2f real_size = window_.real_size().cast_to<f32>();
            if ( math::minimum(real_size) > 0.f ) {
                io_.MousePos = pos * (v2f(io_.DisplaySize) / real_size);
            }
        }

        void on_mouse_scroll(const v2f& delta) noexcept final {
            io_.MouseWheel += delta.y;
            io_.MouseWheelH += delta.x;
        }

        void on_keyboard_key(keyboard_key key, u32 scancode, keyboard_key_action act) noexcept final {
            E2D_UNUSED(scancode);
            auto key_i = utils::enum_to_underlying(key);
            if ( key_i < std::size(io_.KeysDown) ) {
                switch ( act ) {
                    case keyboard_key_action::press:
                    case keyboard_key_action::repeat:
                        io_.KeysDown[key_i] = true;
                        break;
                    case keyboard_key_action::release:
                        io_.KeysDown[key_i] = false;
                        break;
                    case keyboard_key_action::unknown:
                        break;
                }
            }
        }
    private:
        ImGuiIO& io_;
        window& window_;
    };
}

namespace e2d
{
    class dbgui::internal_state final : private e2d::noncopyable {
    public:
        internal_state(debug& d, input& i, render& r, window& w)
        : debug_(d)
        , input_(i)
        , render_(r)
        , window_(w)
        , context_(ImGui::CreateContext())
        , listener_(w.register_event_listener<imgui_event_listener>(bind_context(), w))
        {
            ImGuiIO& io = bind_context();
            setup_key_map_(io);
            setup_config_flags_(io);
            setup_internal_resources_(io);
        }

        ~internal_state() noexcept {
            window_.unregister_event_listener(listener_);
            ImGui::DestroyContext(context_);
        }
    public:
        ImGuiIO& bind_context() noexcept {
            ImGui::SetCurrentContext(context_);
            return ImGui::GetIO();
        }

        bool visible() const noexcept {
            return visible_;
        }

        void toggle_visible(bool yesno) noexcept {
            visible_ = yesno;
        }

        void frame_tick() {
            ImGuiIO& io = bind_context();
            const mouse& m = input_.mouse();
            const keyboard& k = input_.keyboard();

            io.MouseDown[0] =
                m.is_button_just_pressed(mouse_button::left) ||
                m.is_button_pressed(mouse_button::left);

            io.MouseDown[1] =
                m.is_button_just_pressed(mouse_button::right) ||
                m.is_button_pressed(mouse_button::right);

            io.KeyCtrl =
                k.is_key_just_pressed(keyboard_key::lcontrol) ||
                k.is_key_just_pressed(keyboard_key::rcontrol) ||
                k.is_key_pressed(keyboard_key::lcontrol) ||
                k.is_key_pressed(keyboard_key::rcontrol);

            io.KeyShift =
                k.is_key_just_pressed(keyboard_key::lshift) ||
                k.is_key_just_pressed(keyboard_key::rshift) ||
                k.is_key_pressed(keyboard_key::lshift) ||
                k.is_key_pressed(keyboard_key::rshift);

            io.KeyAlt =
                k.is_key_just_pressed(keyboard_key::lalt) ||
                k.is_key_just_pressed(keyboard_key::ralt) ||
                k.is_key_pressed(keyboard_key::lalt) ||
                k.is_key_pressed(keyboard_key::ralt);

            io.KeySuper =
                k.is_key_just_pressed(keyboard_key::lsuper) ||
                k.is_key_just_pressed(keyboard_key::rsuper) ||
                k.is_key_pressed(keyboard_key::lsuper) ||
                k.is_key_pressed(keyboard_key::rsuper);

            io.DisplaySize = window_.real_size().cast_to<f32>();
            io.DisplayFramebufferScale = v2f::unit();

            if ( ImGui::GetFrameCount() > 0 ) {
                ImGui::EndFrame();
            }

            ImGui::NewFrame();
        }

        void frame_render() {
            ImGui::Render();

            ImDrawData* draw_data = ImGui::GetDrawData();
            if ( !draw_data || !draw_data->CmdListsCount ) {
                return;
            }

            const v2f display_size = draw_data->DisplaySize;
            const b2f display_size_r = make_rect(display_size);

            const m4f projection =
                math::make_translation_matrix4(display_size * v2f(-0.5f, 0.5f)) *
                math::make_orthogonal_lh_matrix4(display_size, 0.f, 1.f);

            auto ib = create_index_buffer(*draw_data);
            auto vb = create_vertex_buffer(*draw_data);

            if ( !ib || !vb ) {
                return;
            }

            render_.begin_pass(render::renderpass_desc()
                .color_load()
                .color_store()
                .viewport(display_size_r.cast_to<u32>())
                .target(nullptr));
            
            render_.update_buffer(
                const_buffer_,
                render::property_map()
                    .assign("u_MVP", projection));
            
            std::size_t idx_offset = 0;
            std::size_t vtx_offset = 0;

            for ( int i = 0; i < draw_data->CmdListsCount; ++i ) {
                const ImDrawList* cmd_list = draw_data->CmdLists[i];

                render_.execute(render::bind_vertex_buffers_command()
                    .add(vb, attribs_, vtx_offset));

                for ( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i ) {
                    const ImDrawCmd& pcmd = cmd_list->CmdBuffer[cmd_i];

                    const b2f clip_r(
                        pcmd.ClipRect.x,
                        display_size.y - pcmd.ClipRect.w,
                        pcmd.ClipRect.z - pcmd.ClipRect.x,
                        pcmd.ClipRect.w - pcmd.ClipRect.y);

                    if ( math::minimum(clip_r.position) >= 0.f
                        && math::overlaps(clip_r, display_size_r)
                        && pcmd.TextureId )
                    {
                        texture_ptr texture = pcmd.TextureId
                            ? *static_cast<texture_ptr*>(pcmd.TextureId)
                            : texture_ptr();

                        render_.set_material(render::material(material_)
                            .sampler("u_texture", render::sampler_state()
                                .texture(texture)
                                .min_filter(render::sampler_min_filter::linear)
                                .mag_filter(render::sampler_mag_filter::linear)));
                        render_.execute(render::scissor_command(clip_r.cast_to<u32>()));
                        render_.execute(render::draw_indexed_command()
                            .topo(render::topology::triangles)
                            .indices(ib)
                            .index_count(pcmd.ElemCount)
                            .index_offset(idx_offset));
                    }

                    idx_offset += pcmd.ElemCount * sizeof(ImDrawIdx);
                }
                vtx_offset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            }

            render_.end_pass();
        }
    private:
        void setup_key_map_(ImGuiIO& io) noexcept {
            const auto map_key = [&io](ImGuiKey_ imgui_key, keyboard_key key) noexcept {
                E2D_ASSERT(imgui_key < ImGuiKey_COUNT);
                io.KeyMap[imgui_key] = utils::enum_to_underlying(key);
            };

            map_key(ImGuiKey_Tab, keyboard_key::tab);

            map_key(ImGuiKey_LeftArrow, keyboard_key::left);
            map_key(ImGuiKey_RightArrow, keyboard_key::right);
            map_key(ImGuiKey_UpArrow, keyboard_key::up);
            map_key(ImGuiKey_DownArrow, keyboard_key::down);

            map_key(ImGuiKey_PageUp, keyboard_key::page_up);
            map_key(ImGuiKey_PageDown, keyboard_key::page_down);

            map_key(ImGuiKey_Home, keyboard_key::home);
            map_key(ImGuiKey_End, keyboard_key::end);
            map_key(ImGuiKey_Insert, keyboard_key::insert);
            map_key(ImGuiKey_Delete, keyboard_key::del);

            map_key(ImGuiKey_Backspace, keyboard_key::backspace);
            map_key(ImGuiKey_Space, keyboard_key::space);
            map_key(ImGuiKey_Enter, keyboard_key::enter);
            map_key(ImGuiKey_Escape, keyboard_key::escape);

            map_key(ImGuiKey_A, keyboard_key::a);
            map_key(ImGuiKey_C, keyboard_key::c);
            map_key(ImGuiKey_V, keyboard_key::v);
            map_key(ImGuiKey_X, keyboard_key::x);
            map_key(ImGuiKey_Y, keyboard_key::y);
            map_key(ImGuiKey_Z, keyboard_key::z);
        }

        void setup_config_flags_(ImGuiIO& io) noexcept {
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        void setup_internal_resources_(ImGuiIO& io) {
            {
                auto templ = std::make_shared<cbuffer_template>(cbuffer_template()
                    .add_uniform("u_MVP", 0, cbuffer_template::value_type::m4f));

                shader_ = render_.create_shader(shader_source()
                    .vertex_shader(dbgui_shaders::vertex_source_cstr())
                    .fragment_shader(dbgui_shaders::fragment_source_cstr())
                    .add_attribute("a_position", 0, shader_source::value_type::v3f)
                    .add_attribute("a_uv", 1, shader_source::value_type::v2f)
                    .add_attribute("a_color", 2, shader_source::value_type::v4f)
                    .set_block(templ, shader_source::scope_type::material)
                    .add_sampler("u_texture", 0, shader_source::sampler_type::_2d, shader_source::scope_type::material));

                if ( !shader_ ) {
                    throw bad_dbgui_operation();
                }
            }
            {
                const_buffer_ = render_.create_const_buffer(
                    shader_,
                    const_buffer::scope::material);

                if ( !const_buffer_ ) {
                    throw bad_dbgui_operation();
                }
            }
            {
                unsigned char* font_pixels;
                int font_width, font_height;
                io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);

                texture_ = render_.create_texture(image(
                    make_vec2(font_width, font_height).cast_to<u32>(),
                    image_data_format::rgba8,
                    buffer(
                        font_pixels,
                        math::numeric_cast<std::size_t>(font_width * font_height) * sizeof(u32))));

                if ( !texture_ ) {
                    throw bad_dbgui_operation();
                }

                io.Fonts->TexID = &texture_;
            }
            {
                material_ = render::material()
                    .blending(render::blending_state()
                        .enable(true)
                        .src_factor(render::blending_factor::src_alpha)
                        .dst_factor(render::blending_factor::one_minus_src_alpha))
                    .shader(shader_)
                    .constants(const_buffer_);
            }
            {
                attribs_ = render_.create_vertex_attribs(
                    vertex_declaration()
                        .add_attribute<v2f>("a_position")
                        .add_attribute<v2f>("a_uv")
                        .add_attribute<color32>("a_color").normalized());
                
                if ( !attribs_ ) {
                    throw bad_dbgui_operation();
                }
            }
        }

        index_buffer_ptr create_index_buffer(ImDrawData& draw_data) {
            const std::size_t data_size = draw_data.TotalIdxCount * sizeof(ImDrawIdx);

            index_buffer_ptr ib = render_.create_index_buffer(
                data_size,
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::stream_draw);
            if ( !ib ) {
                return nullptr;
            }

            std::size_t offset = 0;
            for ( int i = 0; i < draw_data.CmdListsCount; ++i ) {
                const ImDrawList& cmd_list = *draw_data.CmdLists[i];
                render_.update_buffer(ib,
                    buffer_view(cmd_list.IdxBuffer.Data, cmd_list.IdxBuffer.Size * sizeof(ImDrawIdx)),
                    offset);
                offset += cmd_list.IdxBuffer.Size;
            }
            return ib;
        }

        vertex_buffer_ptr create_vertex_buffer(ImDrawData& draw_data) {
            const std::size_t data_size = draw_data.TotalVtxCount * sizeof(ImDrawVert);

            vertex_buffer_ptr vb = render_.create_vertex_buffer(
                data_size,
                vertex_buffer::usage::stream_draw);
            if ( !vb ) {
                return nullptr;
            }

            std::size_t offset = 0;
            for ( int i = 0; i < draw_data.CmdListsCount; ++i ) {
                const ImDrawList& cmd_list = *draw_data.CmdLists[i];
                render_.update_buffer(vb,
                    buffer_view(cmd_list.VtxBuffer.Data, cmd_list.VtxBuffer.Size * sizeof(ImDrawVert)),
                    offset);
                offset += cmd_list.VtxBuffer.Size * sizeof(ImDrawVert);
            }
            return vb;
        }
    private:
        debug& debug_;
        input& input_;
        render& render_;
        window& window_;
        bool visible_{false};
        ImGuiContext* context_{nullptr};
        window::event_listener& listener_;
    private:
        shader_ptr shader_;
        texture_ptr texture_;
        render::material material_;
        const_buffer_ptr const_buffer_;
        vertex_attribs_ptr attribs_;
    };
}

namespace e2d
{
    dbgui::dbgui(debug& d, input& i, render& r, window& w)
    : state_(new internal_state(d, i, r, w)) {}
    dbgui::~dbgui() noexcept = default;

    bool dbgui::visible() const noexcept {
        return state_->visible();
    }

    void dbgui::toggle_visible(bool yesno) noexcept {
        state_->toggle_visible(yesno);
    }

    void dbgui::frame_tick() {
        state_->frame_tick();

        if ( visible() ) {
            dbgui_widgets::show_main_menu();
        }
    }

    void dbgui::frame_render() {
        state_->frame_render();
    }
}

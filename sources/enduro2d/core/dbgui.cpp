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
            /*ImGuiIO& io = bind_context();
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

            ImGui::NewFrame();*/
        }

        void frame_render() {
            /*ImGui::Render();

            ImDrawData* draw_data = ImGui::GetDrawData();
            if ( !draw_data ) {
                return;
            }

            const v2f display_size = draw_data->DisplaySize;
            const b2f display_size_r = make_rect(display_size);

            const m4f projection =
                math::make_translation_matrix4(display_size * v2f(-0.5f, 0.5f)) *
                math::make_orthogonal_lh_matrix4(display_size, 0.f, 1.f);

            for ( int i = 0; i < draw_data->CmdListsCount; ++i ) {
                const ImDrawList* cmd_list = draw_data->CmdLists[i];

                update_index_buffer({
                    cmd_list->IdxBuffer.Data,
                    math::numeric_cast<std::size_t>(cmd_list->IdxBuffer.Size) * sizeof(ImDrawIdx)});

                update_vertex_buffer({
                    cmd_list->VtxBuffer.Data,
                    math::numeric_cast<std::size_t>(cmd_list->VtxBuffer.Size) * sizeof(ImDrawVert)});

                const auto geometry = render::geometry()
                    .indices(index_buffer_)
                    .add_vertices(vertex_buffer_);

                std::size_t first_index = 0;
                for ( int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; ++cmd_i ) {
                    const ImDrawCmd& pcmd = cmd_list->CmdBuffer[cmd_i];

                    const b2f clip_r(
                        pcmd.ClipRect.x,
                        display_size.y - pcmd.ClipRect.w,
                        pcmd.ClipRect.z - pcmd.ClipRect.x,
                        pcmd.ClipRect.w - pcmd.ClipRect.y);

                    if ( math::minimum(clip_r.position) >= 0.f
                        && math::overlaps(clip_r, display_size_r) )
                    {
                        texture_ptr texture = pcmd.TextureId
                            ? *static_cast<texture_ptr*>(pcmd.TextureId)
                            : texture_ptr();

                        mprops_
                            .property("u_MVP", projection)
                            .sampler("u_texture", render::sampler_state()
                                .texture(texture)
                                .min_filter(render::sampler_min_filter::linear)
                                .mag_filter(render::sampler_mag_filter::linear));

                        render_.execute(render::command_block<8>()
                            .add_command(render::viewport_command(
                                display_size_r.cast_to<u32>(),
                                clip_r.cast_to<u32>()))
                            .add_command(render::draw_command(material_, geometry, mprops_)
                                .index_range(first_index, pcmd.ElemCount)));
                    }

                    first_index += pcmd.ElemCount;
                }
            }*/
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
            /*{
                shader_ = render_.create_shader(
                    dbgui_shaders::vertex_source_cstr(),
                    dbgui_shaders::fragment_source_cstr());

                if ( !shader_ ) {
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
                    .add_pass(render::pass_state()
                        .states(render::state_block()
                            .capabilities(render::capabilities_state()
                                .blending(true))
                            .blending(render::blending_state()
                                .src_factor(render::blending_factor::src_alpha)
                                .dst_factor(render::blending_factor::one_minus_src_alpha)))
                        .shader(shader_));
            }*/
        }

        static std::size_t calculate_new_buffer_size(
            std::size_t esize, std::size_t osize, std::size_t nsize)
        {
            std::size_t msize = std::size_t(-1) / esize * esize;
            if ( nsize > msize ) {
                throw bad_dbgui_operation();
            }
            if ( osize >= msize / 2u ) {
                return msize;
            }
            return math::max(osize * 2u, nsize);
        }

        void update_index_buffer(buffer_view indices) {
            /*if ( index_buffer_ && index_buffer_->buffer_size() >= indices.size() ) {
                render_.update_buffer(index_buffer_, indices, 0u);
                return;
            }

            const std::size_t new_buffer_size = calculate_new_buffer_size(
                sizeof(ImDrawIdx),
                index_buffer_ ? index_buffer_->buffer_size() : 0u,
                indices.size());

            buffer new_buffer_data(new_buffer_size);
            std::memcpy(new_buffer_data.data(), indices.data(), indices.size());

            index_buffer_ = render_.create_index_buffer(
                new_buffer_data,
                index_declaration::index_type::unsigned_short,
                index_buffer::usage::dynamic_draw);

            if ( !index_buffer_ ) {
                debug_.error("DBGUI: Failed to create index buffer:\n"
                    "--> Size: %0",
                    new_buffer_size);
            }*/
        }

        void update_vertex_buffer(buffer_view vertices) {
            /*if ( vertex_buffer_ && vertex_buffer_->buffer_size() >= vertices.size() ) {
                render_.update_buffer(vertex_buffer_, vertices, 0u);
                return;
            }

            const std::size_t new_buffer_size = calculate_new_buffer_size(
                sizeof(ImDrawVert),
                vertex_buffer_ ? vertex_buffer_->buffer_size() : 0u,
                vertices.size());

            buffer new_buffer_data(new_buffer_size);
            std::memcpy(new_buffer_data.data(), vertices.data(), vertices.size());

            vertex_buffer_ = render_.create_vertex_buffer(
                new_buffer_data,
                vertex_declaration()
                    .add_attribute<v2f>("a_position")
                    .add_attribute<v2f>("a_uv")
                    .add_attribute<color32>("a_color").normalized(),
                vertex_buffer::usage::dynamic_draw);

            if ( !vertex_buffer_ ) {
                debug_.error("DBGUI: Failed to create vertex buffer:\n"
                    "--> Size: %0",
                    new_buffer_size);
            }*/
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
        //shader_ptr shader_;
        texture_ptr texture_;
        //render::material material_;
        //render::property_block mprops_;
        index_buffer_ptr index_buffer_;
        vertex_buffer_ptr vertex_buffer_;
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

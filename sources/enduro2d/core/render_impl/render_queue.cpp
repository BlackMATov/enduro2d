/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/render_queue.hpp>

namespace e2d
{
    render_queue::command_encoder_ptr render_queue::create_pass(
        const render::renderpass_desc& desc,
        const render::sampler_block& samplers,
        const const_buffer_ptr& constants)
    {
    }

    void render_queue::submit() {
    }
}

namespace e2d
{
    render_queue::memory_manager::memory_manager() {}

    const_buffer_ptr render_queue::memory_manager::alloc_cbuffer(
        const cbuffer_template_ptr& templ)
    {
    }

    u8* render_queue::memory_manager::alloc(size_t size, size_t align) {
    }

    void render_queue::memory_manager::discard() {
    }
}

namespace e2d
{
    void render_queue::command_encoder::draw_mesh(
        sort_key key,
        const material_ptr& mtr)
    {
    }
}

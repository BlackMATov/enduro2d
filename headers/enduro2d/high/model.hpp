/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

#include "assets/mesh_asset.hpp"

namespace e2d
{
    class model final {
    public:
        model() = default;
        ~model() noexcept = default;

        model(model&& other) noexcept;
        model& operator=(model&& other) noexcept;

        model(const model& other);
        model& operator=(const model& other);

        void clear() noexcept;
        void swap(model& other) noexcept;

        model& assign(model&& other) noexcept;
        model& assign(const model& other);

        model& set_mesh(const mesh_asset::ptr& mesh);
        const mesh_asset::ptr& mesh() const noexcept;

        // It can only be called from the main thread
        void regenerate_geometry(render& render);

        render::topology topo() const noexcept;
        const index_buffer_ptr& indices() const noexcept;
        const vertex_buffer_ptr& vertices(std::size_t index) const noexcept;
        const vertex_attribs_ptr& attribute(std::size_t index) const noexcept;
        std::size_t vertices_count() const noexcept;
    private:
        mesh_asset::ptr mesh_;
        index_buffer_ptr indices_;
        std::array<vertex_buffer_ptr, render_cfg::max_attribute_count> vertices_;
        std::array<vertex_attribs_ptr, render_cfg::max_attribute_count> attributes_;
        std::size_t vertices_count_ = 0;
        render::topology topology_ = render::topology::triangles;
    };

    void swap(model& l, model& r) noexcept;
    bool operator==(const model& l, const model& r) noexcept;
    bool operator!=(const model& l, const model& r) noexcept;
}

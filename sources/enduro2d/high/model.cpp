/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/model.hpp>

namespace
{
    using namespace e2d;

    const vertex_declaration vertex_buffer_decl = vertex_declaration()
        .add_attribute<v3f>("a_vertex");

    const vertex_declaration uv_buffer_decls[] = {
        vertex_declaration().add_attribute<v2f>("a_st0"),
        vertex_declaration().add_attribute<v2f>("a_st1"),
        vertex_declaration().add_attribute<v2f>("a_st2"),
        vertex_declaration().add_attribute<v2f>("a_st3")};

    const vertex_declaration color_buffer_decls[] = {
        vertex_declaration().add_attribute<color32>("a_color0").normalized(),
        vertex_declaration().add_attribute<color32>("a_color1").normalized(),
        vertex_declaration().add_attribute<color32>("a_color2").normalized(),
        vertex_declaration().add_attribute<color32>("a_color3").normalized()};

    const vertex_declaration normal_buffer_decl = vertex_declaration()
        .add_attribute<v3f>("a_normal");

    const vertex_declaration tangent_buffer_decl = vertex_declaration()
        .add_attribute<v3f>("a_tangent");

    const vertex_declaration bitangent_buffer_decl = vertex_declaration()
        .add_attribute<v3f>("a_bitangent");

    void create_index_buffer(
        render& render,
        const mesh& mesh,
        index_buffer_ptr& index_buffer)
    {
        index_buffer = nullptr;

        std::size_t index_count{0u};
        for ( std::size_t i = 0; i < mesh.indices_submesh_count(); ++i ) {
            index_count += mesh.indices(i).size();
        }

        if ( index_count > 0 ) {
            vector<u32> indices;
            indices.reserve(index_count);

            for ( std::size_t i = 0; i < mesh.indices_submesh_count(); ++i ) {
                indices.insert(indices.end(), mesh.indices(i).begin(), mesh.indices(i).end());
            }

            index_buffer = render.create_index_buffer(
                indices,
                index_declaration::index_type::unsigned_int,
                index_buffer::usage::static_draw);
        }
    }

    void create_vertex_buffers(
        render& render,
        const mesh& mesh,
        std::array<vertex_buffer_ptr, render_cfg::max_attribute_count>& buffers,
        std::array<vertex_attribs_ptr, render_cfg::max_attribute_count>& attributes,
        std::size_t& buffer_count)
    {
        buffer_count = 0;
        std::size_t vert_count = 0;

        if ( const vector<v3f>& vertices = mesh.vertices(); vertices.size() ) {
            const vertex_buffer_ptr vertex_buffer = render.create_vertex_buffer(
                vertices,
                vertex_buffer::usage::static_draw);
            const vertex_attribs_ptr vertex_attribs = render.create_vertex_attribs(
                vertex_buffer_decl);

            if ( vertex_buffer && vertex_attribs ) {
                vert_count = vertices.size();
                buffers[buffer_count] = vertex_buffer;
                attributes[buffer_count] = vertex_attribs;
                ++buffer_count;
            }
        }

        {
            const std::size_t uv_count = math::min(
                mesh.uvs_channel_count(),
                std::size(uv_buffer_decls));
            for ( std::size_t i = 0; i < uv_count; ++i ) {
                if ( const vector<v2f>& uvs = mesh.uvs(i); uvs.size() ) {
                    const vertex_buffer_ptr uv_buffer = render.create_vertex_buffer(
                        uvs,
                        vertex_buffer::usage::static_draw);
                    const vertex_attribs_ptr uv_attribs = render.create_vertex_attribs(
                        uv_buffer_decls[i]);

                    if ( uv_buffer && uv_attribs ) {
                        if ( vert_count ) {
                            E2D_ASSERT(vert_count == uvs.size());
                        } else {
                            vert_count = uvs.size();
                        }
                        buffers[buffer_count] = uv_buffer;
                        attributes[buffer_count] = uv_attribs;
                        ++buffer_count;
                    }
                }
            }
        }

        {
            const std::size_t color_count = math::min(
                mesh.colors_channel_count(),
                std::size(uv_buffer_decls));
            for ( std::size_t i = 0; i < color_count; ++i ) {
                if ( const vector<color32>& colors = mesh.colors(i); colors.size() ) {
                    const vertex_buffer_ptr color_buffer = render.create_vertex_buffer(
                        colors,
                        vertex_buffer::usage::static_draw);
                    const vertex_attribs_ptr color_attribs = render.create_vertex_attribs(
                        color_buffer_decls[i]);

                    if ( color_buffer && color_attribs ) {
                        if ( vert_count ) {
                            E2D_ASSERT(vert_count == colors.size());
                        } else {
                            vert_count = colors.size();
                        }
                        buffers[buffer_count] = color_buffer;
                        attributes[buffer_count] = color_attribs;
                        ++buffer_count;
                    }
                }
            }
        }
        
        if ( const vector<v3f>& normals = mesh.normals(); normals.size() ) {
            const vertex_buffer_ptr normal_buffer = render.create_vertex_buffer(
                normals,
                vertex_buffer::usage::static_draw);
            const vertex_attribs_ptr normal_attribs = render.create_vertex_attribs(
                normal_buffer_decl);

            if ( normal_buffer && normal_attribs ) {
                if ( vert_count ) {
                    E2D_ASSERT(vert_count == normals.size());
                } else {
                    vert_count = normals.size();
                }
                buffers[buffer_count] = normal_buffer;
                attributes[buffer_count] = normal_attribs;
                ++buffer_count;
            }
        }
        
        if ( const vector<v3f>& tangents = mesh.tangents(); tangents.size() ) {
            const vertex_buffer_ptr tangent_buffer = render.create_vertex_buffer(
                tangents,
                vertex_buffer::usage::static_draw);
            const vertex_attribs_ptr tangent_attribs = render.create_vertex_attribs(
                tangent_buffer_decl);

            if ( tangent_buffer && tangent_attribs ) {
                if ( vert_count ) {
                    E2D_ASSERT(vert_count == tangents.size());
                } else {
                    vert_count = tangents.size();
                }
                buffers[buffer_count] = tangent_buffer;
                attributes[buffer_count] = tangent_attribs;
                ++buffer_count;
            }
        }
        
        if ( const vector<v3f>& bitangents = mesh.bitangents(); bitangents.size() ) {
            const vertex_buffer_ptr bitangent_buffer = render.create_vertex_buffer(
                bitangents,
                vertex_buffer::usage::static_draw);
            const vertex_attribs_ptr bitangent_attribs = render.create_vertex_attribs(
                bitangent_buffer_decl);

            if ( bitangent_buffer && bitangent_attribs ) {
                if ( vert_count ) {
                    E2D_ASSERT(vert_count == bitangents.size());
                } else {
                    vert_count = bitangents.size();
                }
                buffers[buffer_count] = bitangent_buffer;
                attributes[buffer_count] = bitangent_attribs;
                ++buffer_count;
            }
        }
    }
}

namespace e2d
{
    model::model(model&& other) noexcept {
        assign(std::move(other));
    }

    model& model::operator=(model&& other) noexcept {
        return assign(std::move(other));
    }

    model::model(const model& other) {
        assign(other);
    }

    model& model::operator=(const model& other) {
        return assign(other);
    }

    void model::clear() noexcept {
        mesh_.reset();
        indices_ = nullptr;
        vertices_ = {};
        attributes_ = {};
        vertices_count_ = 0;
    }

    void model::swap(model& other) noexcept {
        using std::swap;
        swap(mesh_, other.mesh_);
        swap(indices_, other.indices_);
        swap(vertices_, other.vertices_);
        swap(attributes_, other.attributes_);
        swap(vertices_count_, other.vertices_count_);
        swap(topology_, other.topology_);
    }

    model& model::assign(model&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    model& model::assign(const model& other) {
        if ( this != &other ) {
            model m;
            m.mesh_ = other.mesh_;
            m.indices_ = other.indices_;
            m.vertices_ = other.vertices_;
            m.attributes_ = other.attributes_;
            m.vertices_count_ = other.vertices_count_;
            m.topology_ = other.topology_;
            swap(m);
        }
        return *this;
    }

    model& model::set_mesh(const mesh_asset::ptr& mesh) {
        model m;
        m.mesh_ = mesh;
        swap(m);
        return *this;
    }

    const mesh_asset::ptr& model::mesh() const noexcept {
        return mesh_;
    }

    void model::regenerate_geometry(render& render) {
        if ( mesh_ ) {
            create_index_buffer(render, mesh_->content(), indices_);
            create_vertex_buffers(render, mesh_->content(), vertices_, attributes_, vertices_count_);
        } else {
            model m;
            swap(m);
        }
    }
    
    std::size_t model::vertices_count() const noexcept {
        return vertices_count_;
    }

    render::topology model::topo() const noexcept {
        return topology_;
    }

    const index_buffer_ptr& model::indices() const noexcept {
        return indices_;
    }

    const vertex_buffer_ptr& model::vertices(std::size_t index) const noexcept {
        E2D_ASSERT(index < vertices_count_);
        return vertices_[index];
    }
    
    const vertex_attribs_ptr& model::attribute(std::size_t index) const noexcept {
        E2D_ASSERT(index < vertices_count_);
        return attributes_[index];
    }
}

namespace e2d
{
    void swap(model& l, model& r) noexcept {
        l.swap(r);
    }

    bool operator==(const model& l, const model& r) noexcept {
        if ( l.mesh() != r.mesh() ) {
            return false;
        }
        if ( l.topo() != r.topo() ) {
            return false;
        }
        if ( l.indices() != r.indices() ) {
            return false;
        }
        if ( l.vertices_count() != r.vertices_count() ) {
            return false;
        }
        for ( std::size_t i = 0, e = l.vertices_count(); i < e; ++i ) {
            if ( l.vertices(i) != r.vertices(i) || l.attribute(i) != r.attribute(i) ) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const model& l, const model& r) noexcept {
        return !(l == r);
    }
}

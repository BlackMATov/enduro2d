/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../core/_all.hpp"

#include <3rdparty/ecs.hpp/ecs.hpp>

namespace e2d
{
    namespace ecs
    {
        using namespace ecs_hpp;
    }
}

namespace e2d
{
    class atlas_asset;
    class binary_asset;
    class flipbook_asset;
    class image_asset;
    class material_asset;
    class mesh_asset;
    class model_asset;
    class shader_asset;
    class shape_asset;
    class sprite_asset;
    class text_asset;
    class texture_asset;

    class actor;
    class camera;
    class flipbook_player;
    class flipbook_source;
    class model_renderer;
    class renderer;
    class scene;
    class sprite_renderer;

    class flipbook_system;
    class render_system;

    template < typename Asset, typename Content >
    class content_asset;
    class asset;

    template < typename T >
    class asset_cache;
    class asset_cache_base;

    template < typename Asset >
    class asset_dependency;
    class asset_dependency_base;

    class asset_group;
    class asset_dependencies;

    class library;

    class atlas;
    class flipbook;
    class model;
    class node;
    class sprite;
    class starter;
    class world;
}

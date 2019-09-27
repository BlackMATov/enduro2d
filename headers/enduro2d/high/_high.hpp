/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../core/_all.hpp"

#include <ecs.hpp/ecs.hpp>

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
    class font_asset;
    class image_asset;
    class json_asset;
    class material_asset;
    class mesh_asset;
    class model_asset;
    class prefab_asset;
    class shader_asset;
    class shape_asset;
    class sound_asset;
    class spine_asset;
    class sprite_asset;
    class sprite_9p_asset;
    class text_asset;
    class texture_asset;
    class xml_asset;
    class ui_color_style_asset;
    class ui_image_style_asset;
    class ui_font_style_asset;

    class actor;
    class camera;
    class flipbook_player;
    class label;
    class model_renderer;
    class renderer;
    class scene;
    class spine_player;
    class spine_player_cmd;
    class spine_player_evt;
    class sprite_renderer;
    class sprite_9p_renderer;
    class touchable;
    class input_event;
    class ui_layout;
    class ui_button;
    class ui_selectable;
    class ui_draggable;

    class actor_system;
    class flipbook_system;
    class label_system;
    class render_system;
    class spine_system;
    class input_event_system;
    class ui_system;

    template < typename Asset, typename Content >
    class content_asset;
    class asset;

    class factory;
    class library;
    class asset_cache;
    class asset_group;
    class asset_dependencies;

    class atlas;
    class flipbook;
    class gobject;
    class model;
    class node;
    class prefab;
    class spine;
    class sprite;
    class sprite_9p;
    class starter;
    class ui_color_style;
    class ui_image_style;
    class ui_font_style;
    class world;
}

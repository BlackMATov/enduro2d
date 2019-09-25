/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/assets/spine_asset.hpp>

#include <enduro2d/high/assets/json_asset.hpp>
#include <enduro2d/high/assets/binary_asset.hpp>
#include <enduro2d/high/assets/texture_asset.hpp>

#include <spine/spine.h>
#include <spine/extension.h>

namespace
{
    using namespace e2d;

    class spine_asset_loading_exception final : public asset_loading_exception {
        const char* what() const noexcept final {
            return "spine asset loading exception";
        }
    };

    const char* spine_asset_schema_source = R"json({
        "type" : "object",
        "required" : [ "atlas", "skeleton" ],
        "additionalProperties" : false,
        "properties" : {
            "atlas" : { "$ref" : "#/common_definitions/address" },
            "skeleton" : { "$ref" : "#/common_definitions/address" },
            "skeleton_scale" : { "type" : "number" },
            "default_animation_mix" : { "type" : "number" },
            "animation_mixes" : { "$ref": "#/definitions/animation_mixes" }
        },
        "definitions" : {
            "animation_mixes" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/animation_mix" }
            },
            "animation_mix" : {
                "type" : "object",
                "required" : [ "from", "to", "duration" ],
                "additionalProperties" : false,
                "properties" : {
                    "from" : { "$ref": "#/common_definitions/name" },
                    "to" : { "$ref": "#/common_definitions/name" },
                    "duration" : { "type" : "number" }
                }
            }
        }
    })json";

    const rapidjson::SchemaDocument& spine_asset_schema() {
        static std::mutex mutex;
        static std::unique_ptr<rapidjson::SchemaDocument> schema;

        std::lock_guard<std::mutex> guard(mutex);
        if ( !schema ) {
            rapidjson::Document doc;
            if ( doc.Parse(spine_asset_schema_source).HasParseError() ) {
                the<debug>().error("ASSETS: Failed to parse spine asset schema");
                throw spine_asset_loading_exception();
            }
            json_utils::add_common_schema_definitions(doc);
            schema = std::make_unique<rapidjson::SchemaDocument>(doc);
        }

        return *schema;
    }

    struct animation_mix {
        str from;
        str to;
        secf duration;
    };

    animation_mix parse_animation_mix(const rapidjson::Value& root) {
        animation_mix mix;

        E2D_ASSERT(root.HasMember("from") && root["from"].IsString());
        if ( !json_utils::try_parse_value(root["from"], mix.from) ) {
            the<debug>().error("SPINE: Incorrect formating of 'from' property");
            throw spine_asset_loading_exception();
        }

        E2D_ASSERT(root.HasMember("to") && root["to"].IsString());
        if ( !json_utils::try_parse_value(root["to"], mix.to) ) {
            the<debug>().error("SPINE: Incorrect formating of 'to' property");
            throw spine_asset_loading_exception();
        }

        E2D_ASSERT(root.HasMember("duration") && root["duration"].IsNumber());
        if ( !json_utils::try_parse_value(root["duration"], mix.duration) ) {
            the<debug>().error("SPINE: Incorrect formating of 'duration' property");
            throw spine_asset_loading_exception();
        }

        return mix;
    }

    struct atlas_internal_state {
        asset_group loaded;
        asset_dependencies loading;
    };

    stdex::promise<spine::atlas_ptr> load_atlas(
        const library& library,
        const str& parent_address,
        const str& atlas_address)
    {
        const str atlas_path = path::combine(parent_address, atlas_address);
        const str atlas_folder = path::parent_path(atlas_path);

        return library.load_asset_async<binary_asset>(atlas_path)
        .then([
            &library,
            atlas_folder
        ](const binary_asset::load_result& atlas_data){
            auto atlas_internal = std::make_unique<atlas_internal_state>();

            spine::atlas_ptr atlas(
                spAtlas_create(
                    reinterpret_cast<const char*>(atlas_data->content().data()),
                    math::numeric_cast<int>(atlas_data->content().size()),
                    atlas_folder.c_str(),
                    atlas_internal.get()),
                spAtlas_dispose);

            if ( !atlas ) {
                the<debug>().error("SPINE: Failed to create preload atlas");
                throw spine_asset_loading_exception();
            }

            return stdex::make_tuple_promise(std::make_tuple(
                atlas_internal->loading.load_async(library),
                stdex::make_resolved_promise(atlas_data)));
        })
        .then([
            atlas_folder
        ](const std::tuple<
            asset_group,
            binary_asset::load_result
        >& results){
            auto atlas_internal = std::make_unique<atlas_internal_state>();
            atlas_internal->loaded = std::get<0>(results);

            spine::atlas_ptr atlas(
                spAtlas_create(
                    reinterpret_cast<const char*>(std::get<1>(results)->content().data()),
                    math::numeric_cast<int>(std::get<1>(results)->content().size()),
                    atlas_folder.c_str(),
                    atlas_internal.get()),
                spAtlas_dispose);

            if ( !atlas ) {
                the<debug>().error("SPINE: Failed to create preloaded atlas");
                throw spine_asset_loading_exception();
            }

            for ( const spAtlasPage* page = atlas->pages; page; page = page->next ) {
                if ( !page->rendererObject ) {
                    the<debug>().error("SPINE: Failed to create preloaded atlas");
                    throw spine_asset_loading_exception();
                }
            }

            atlas->rendererObject = nullptr;
            return atlas;
        });
    }

    stdex::promise<spine::skeleton_data_ptr> load_skeleton_data(
        const library& library,
        const str& parent_address,
        const str& skeleton_address,
        f32 skeleton_scale,
        const spine::atlas_ptr& atlas)
    {
        return library.load_asset_async<binary_asset>(
            path::combine(parent_address, skeleton_address))
        .then([
            atlas,
            skeleton_scale,
            skeleton_address
        ](const binary_asset::load_result& skeleton_data){
            if ( strings::ends_with(skeleton_address, ".skel") ) {
                using skeleton_bin_ptr = std::unique_ptr<
                    spSkeletonBinary,
                    decltype(&::spSkeletonBinary_dispose)>;

                skeleton_bin_ptr binary_skeleton(
                    spSkeletonBinary_create(atlas.get()),
                    spSkeletonBinary_dispose);

                if ( !binary_skeleton ) {
                    the<debug>().error("SPINE: Failed to create binary skeleton");
                    throw spine_asset_loading_exception();
                }

                binary_skeleton->scale = skeleton_scale;

                spine::skeleton_data_ptr data_skeleton(
                    spSkeletonBinary_readSkeletonData(
                        binary_skeleton.get(),
                        reinterpret_cast<const unsigned char*>(skeleton_data->content().data()),
                        math::numeric_cast<int>(skeleton_data->content().size())),
                    spSkeletonData_dispose);

                if ( !data_skeleton ) {
                    the<debug>().error("SPINE: Failed to read binary skeleton data:\n"
                        "--> Error: %0",
                        binary_skeleton->error);
                    throw spine_asset_loading_exception();
                }

                return data_skeleton;
            } else {
                using skeleton_json_ptr = std::unique_ptr<
                    spSkeletonJson,
                    decltype(&::spSkeletonJson_dispose)>;

                skeleton_json_ptr json_skeleton(
                    spSkeletonJson_create(atlas.get()),
                    spSkeletonJson_dispose);

                if ( !json_skeleton ) {
                    the<debug>().error("SPINE: Failed to create json skeleton");
                    throw spine_asset_loading_exception();
                }

                json_skeleton->scale = skeleton_scale;

                spine::skeleton_data_ptr data_skeleton(
                    spSkeletonJson_readSkeletonData(
                        json_skeleton.get(),
                        reinterpret_cast<const char*>(skeleton_data->content().data())),
                    spSkeletonData_dispose);

                if ( !data_skeleton ) {
                    the<debug>().error("SPINE: Failed to read json skeleton data:\n"
                        "--> Error: %0",
                        json_skeleton->error);
                    throw spine_asset_loading_exception();
                }

                return data_skeleton;
            }
        });
    }

    stdex::promise<spine> parse_spine(
        const library& library,
        const str& parent_address,
        const rapidjson::Value& root)
    {
        f32 skeleton_scale{1.0f};
        if ( root.HasMember("skeleton_scale") ) {
            if ( !json_utils::try_parse_value(root["skeleton_scale"], skeleton_scale) ) {
                the<debug>().error("SPINE: Incorrect formating of 'skeleton_scale' property");
            }
        }

        secf default_animation_mix{0.5f};
        if ( root.HasMember("default_animation_mix") ) {
            if ( !json_utils::try_parse_value(root["default_animation_mix"], default_animation_mix) ) {
                the<debug>().error("SPINE: Incorrect formating of 'default_animation_mix' property");
            }
        }

        vector<animation_mix> animation_mixes;
        if ( root.HasMember("animation_mixes") ) {
            E2D_ASSERT(root["animation_mixes"].IsArray());
            const auto& mixes_json = root["animation_mixes"];
            animation_mixes.reserve(mixes_json.Size());
            for ( rapidjson::SizeType i = 0; i < mixes_json.Size(); ++i ) {
                E2D_ASSERT(mixes_json[i].IsObject());
                animation_mixes.push_back(
                    parse_animation_mix(mixes_json[i]));
            }
        }

        E2D_ASSERT(root.HasMember("atlas") && root["atlas"].IsString());
        const str atlas_address = root["atlas"].GetString();

        E2D_ASSERT(root.HasMember("skeleton") && root["skeleton"].IsString());
        const str skeleton_address = root["skeleton"].GetString();

        return load_atlas(
            library,
            parent_address,
            atlas_address)
        .then([
            &library,
            parent_address,
            atlas_address,
            skeleton_scale,
            skeleton_address
        ](const spine::atlas_ptr& atlas){
            return stdex::make_tuple_promise(std::make_tuple(
                stdex::make_resolved_promise(atlas),
                load_skeleton_data(
                    library,
                    parent_address,
                    skeleton_address,
                    skeleton_scale,
                    atlas)));
        })
        .then([
            default_animation_mix,
            animation_mixes = std::move(animation_mixes)
        ](const std::tuple<
            spine::atlas_ptr,
            spine::skeleton_data_ptr
        >& results){
            spine content;
            content.set_atlas(std::get<0>(results));
            content.set_skeleton(std::get<1>(results));
            content.set_default_mix(default_animation_mix);
            for ( const animation_mix& mix : animation_mixes ) {
                content.set_animation_mix(mix.from, mix.to, mix.duration);
            }
            return content;
        });
    }
}

namespace e2d
{
    spine_asset::load_async_result spine_asset::load_async(
        const library& library, str_view address)
    {
        return library.load_asset_async<json_asset>(address)
        .then([
            &library,
            address = str(address),
            parent_address = path::parent_path(address)
        ](const json_asset::load_result& spine_data){
            return the<deferrer>().do_in_worker_thread([address, spine_data](){
                const rapidjson::Document& doc = *spine_data->content();
                rapidjson::SchemaValidator validator(spine_asset_schema());

                if ( doc.Accept(validator) ) {
                    return;
                }

                rapidjson::StringBuffer sb;
                if ( validator.GetInvalidDocumentPointer().StringifyUriFragment(sb) ) {
                    the<debug>().error("ASSET: Failed to validate asset json:\n"
                        "--> Address: %0\n"
                        "--> Invalid schema keyword: %1\n"
                        "--> Invalid document pointer: %2",
                        address,
                        validator.GetInvalidSchemaKeyword(),
                        sb.GetString());
                } else {
                    the<debug>().error("ASSET: Failed to validate asset json");
                }

                throw spine_asset_loading_exception();
            })
            .then([&library, parent_address, spine_data](){
                return parse_spine(
                    library, parent_address, *spine_data->content());
            })
            .then([](auto&& content){
                return spine_asset::create(
                    std::forward<decltype(content)>(content));
            });
        });
    }
}

extern "C" void _spAtlasPage_createTexture(spAtlasPage* self, const char* path) {
    try {
        E2D_ASSERT(self->atlas->rendererObject);
        atlas_internal_state& atlas_internal =
            *static_cast<atlas_internal_state*>(self->atlas->rendererObject);
        auto texture_res = atlas_internal.loaded.find_asset<texture_asset>(path);
        if ( texture_res ) {
            self->width = math::numeric_cast<int>(texture_res->content()->size().x);
            self->height = math::numeric_cast<int>(texture_res->content()->size().y);
            self->rendererObject = texture_res.release();
        } else {
            atlas_internal.loading.add_dependency<texture_asset>(path);
        }
    } catch(...) {
        // nothing
    }
}

extern "C" void _spAtlasPage_disposeTexture(spAtlasPage* self) {
    E2D_UNUSED(texture_asset::load_result(
        static_cast<texture_asset*>(self->rendererObject), false));
}

extern "C" char* _spUtil_readFile(const char* path, int* length) {
    E2D_ASSERT_MSG(false, "unimplemented by design");
    E2D_UNUSED(path, length);
    return nullptr;
}

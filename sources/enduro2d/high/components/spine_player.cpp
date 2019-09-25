/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/spine_player.hpp>

#include <spine/spine.h>

namespace e2d
{
    spine_player::spine_player(const spine_asset::ptr& spine) {
        this->spine(spine);
    }

    spine_player& spine_player::spine(const spine_asset::ptr& value) {
        clipping_ptr new_clipping = clipping_ptr(
            spSkeletonClipping_create(),
            spSkeletonClipping_dispose);
        if ( !new_clipping ) {
            throw std::bad_alloc();
        }

        skeleton_ptr new_skeleton;
        if ( value && value->content().skeleton() ) {
            new_skeleton = skeleton_ptr(
                spSkeleton_create(value->content().skeleton().get()),
                spSkeleton_dispose);
            if ( !new_skeleton ) {
                throw std::bad_alloc();
            }
        }

        animation_ptr new_animation;
        if ( value && value->content().animation() ) {
            new_animation = animation_ptr(
                spAnimationState_create(value->content().animation().get()),
                spAnimationState_dispose);
            if ( !new_animation ) {
                throw std::bad_alloc();
            }
        }

        spine_ = value;
        clipping_ = std::move(new_clipping);
        skeleton_ = std::move(new_skeleton);
        animation_ = std::move(new_animation);
        return *this;
    }

    spine_player& spine_player::materials(
        flat_map<str_hash, material_asset::ptr>&& value) noexcept
    {
        materials_ = std::move(value);
        return *this;
    }

    spine_player& spine_player::materials(
        const flat_map<str_hash, material_asset::ptr>& value)
    {
        materials_ = value;
        return *this;
    }

    spine_player& spine_player::skin(const str& value) noexcept {
        if ( !spSkeleton_setSkinByName(skeleton_.get(), value.empty() ? nullptr : value.c_str()) ) {
            the<debug>().error("SPINE_PLAYER: can't set skin '%0'", value);
        }
        return *this;
    }

    spine_player& spine_player::attachment(const str& slot, const str& name) noexcept {
        if ( !spSkeleton_setAttachment(skeleton_.get(), slot.c_str(), name.c_str()) ) {
            the<debug>().error("SPINE_PLAYER: can't set attachment '%0' to slot '%1'", name, slot);
        }
        return *this;
    }

    bool spine_player::has_skin(const str& name) const noexcept {
        return spine_
            && spine()->content().skeleton()
            && spSkeletonData_findSkin(spine()->content().skeleton().get(), name.c_str());
    }

    bool spine_player::has_animation(const str& name) const noexcept {
        return spine_
            && spine()->content().skeleton()
            && spSkeletonData_findAnimation(spine()->content().skeleton().get(), name.c_str());
    }

    const spine_asset::ptr& spine_player::spine() const noexcept {
        return spine_;
    }
    
    const spine_player::clipping_ptr& spine_player::clipper() const noexcept {
        return clipping_;
    }

    const spine_player::skeleton_ptr& spine_player::skeleton() const noexcept {
        return skeleton_;
    }

    const spine_player::animation_ptr& spine_player::animation() const noexcept {
        return animation_;
    }

    material_asset::ptr spine_player::find_material(str_hash name) const noexcept {
        const auto iter = materials_.find(name);
        return iter != materials_.end()
            ? iter->second
            : nullptr;
    }

    const flat_map<str_hash, material_asset::ptr>& spine_player::materials() const noexcept {
        return materials_;
    }
}

namespace e2d
{
    const char* factory_loader<spine_player>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "spine" : { "$ref": "#/common_definitions/address" },
            "materials" : { "$ref": "#/definitions/materials" },
            "skin" : { "$ref": "#/common_definitions/name" },
            "attachments" : { "$ref": "#/definitions/attachments" }
        },
        "definitions" : {
            "materials" : {
                "type" : "object",
                "required" : [],
                "additionalProperties" : false,
                "properties" : {
                    "normal" : { "$ref": "#/common_definitions/address" },
                    "additive" : { "$ref": "#/common_definitions/address" },
                    "multiply" : { "$ref": "#/common_definitions/address" },
                    "screen" : { "$ref": "#/common_definitions/address" }
                }
            },
            "attachments" : {
                "type" : "array",
                "items" : { "$ref": "#/definitions/attachment" }
            },
            "attachment" : {
                "type" : "object",
                "required" : [ "slot", "name" ],
                "additionalProperties" : false,
                "properties" : {
                    "slot" : { "$ref": "#/common_definitions/name" },
                    "name" : { "$ref": "#/common_definitions/name" }
                }
            }
        }
    })json";

    bool factory_loader<spine_player>::operator()(
        spine_player& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("spine") ) {
            auto spine = ctx.dependencies.find_asset<spine_asset>(
                path::combine(ctx.parent_address, ctx.root["spine"].GetString()));
            if ( !spine ) {
                the<debug>().error("SPINE_PLAYER: Dependency 'spine' is not found:\n"
                    "--> Parent address: %0\n"
                    "--> Dependency address: %1",
                    ctx.parent_address,
                    ctx.root["spine"].GetString());
                return false;
            }
            component.spine(spine);
        }

        if ( ctx.root.HasMember("materials") ) {
            const rapidjson::Value& materials_root = ctx.root["materials"];
            flat_map<str_hash, material_asset::ptr> materials;
            materials.reserve(materials_root.MemberCount());
            for ( rapidjson::Value::ConstMemberIterator material_root = materials_root.MemberBegin();
                material_root != materials_root.MemberEnd();
                ++material_root )
            {
                str_hash material_name;
                if ( !json_utils::try_parse_value(material_root->name, material_name) ) {
                    the<debug>().error("SPINE_PLAYER: Incorrect formatting of 'material.name' property");
                    return false;
                }

                str material_address;
                if ( !json_utils::try_parse_value(material_root->value, material_address) ) {
                    the<debug>().error("SPINE_PLAYER: Incorrect formatting of 'material.address' property");
                    return false;
                }

                auto material = ctx.dependencies.find_asset<material_asset>(
                    path::combine(ctx.parent_address, material_address));
                if ( !material ) {
                    the<debug>().error("SPINE_PLAYER: Dependency 'material' is not found:\n"
                        "--> Parent address: %0\n"
                        "--> Dependency address: %1",
                        ctx.parent_address,
                        material_address);
                    return false;
                }
                materials.emplace(material_name, material);
            }
            component.materials(std::move(materials));
        }

        if ( ctx.root.HasMember("skin") ) {
            str skin;
            if ( !json_utils::try_parse_value(ctx.root["skin"], skin) ) {
                the<debug>().error("SPINE_PLAYER: Incorrect formatting of 'skin' property");
                return false;
            }
            component.skin(skin);
        }

        if ( ctx.root.HasMember("attachments") ) {
            const auto& attachments_json = ctx.root["attachments"];
            E2D_ASSERT(attachments_json.IsArray());

            for ( rapidjson::SizeType i = 0; i < attachments_json.Size(); ++i ) {
                E2D_ASSERT(attachments_json[i].IsObject());
                const auto& attachment_json = attachments_json[i];

                str slot;
                E2D_ASSERT(attachment_json.HasMember("slot"));
                if ( !json_utils::try_parse_value(attachment_json["slot"], slot) ) {
                    the<debug>().error("SPINE_PLAYER: Incorrect formatting of 'attachment.slot' property");
                    return false;
                }

                str name;
                E2D_ASSERT(attachment_json.HasMember("name"));
                if ( !json_utils::try_parse_value(attachment_json["name"], name) ) {
                    the<debug>().error("SPINE_PLAYER: Incorrect formatting of 'attachment.name' property");
                    return false;
                }

                component.attachment(slot, name);
            }
        }

        return true;
    }

    bool factory_loader<spine_player>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        if ( ctx.root.HasMember("spine") ) {
            dependencies.add_dependency<spine_asset>(
                path::combine(ctx.parent_address, ctx.root["spine"].GetString()));
        }

        if ( ctx.root.HasMember("materials") ) {
            const rapidjson::Value& materials_root = ctx.root["materials"];
            for ( rapidjson::Value::ConstMemberIterator material_root = materials_root.MemberBegin();
                material_root != materials_root.MemberEnd();
                ++material_root )
            {
                dependencies.add_dependency<material_asset>(
                    path::combine(ctx.parent_address, material_root->value.GetString()));
            }
        }

        return true;
    }
}

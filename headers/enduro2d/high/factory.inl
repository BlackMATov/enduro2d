/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "factory.hpp"

namespace e2d
{
    //
    // typed_factory_creator
    //

    template < typename Component >
    typed_factory_creator<Component>::typed_factory_creator() {
        rapidjson::Document doc;
        if ( doc.Parse(loader_.schema_source).HasParseError() ) {
            the<debug>().error("FACTORY: Failed to parse factory loader schema");
            throw bad_factory_operation();
        }
        json_utils::add_common_schema_definitions(doc);
        json_utils::add_render_schema_definitions(doc);
        schema_ = std::make_unique<rapidjson::SchemaDocument>(doc);
    }

    template < typename Component >
    bool typed_factory_creator<Component>::validate_json(
        const rapidjson::Value& root) const
    {
        rapidjson::SchemaValidator validator(*schema_);
        if ( root.Accept(validator) ) {
            return true;
        }
        rapidjson::StringBuffer sb;
        if ( !validator.GetInvalidDocumentPointer().StringifyUriFragment(sb) ) {
            the<debug>().error("FACTORY: Failed to validate component json");
            return false;
        }
        the<debug>().error("FACTORY: Failed to validate component json:\n"
            "--> Invalid schema keyword: %0\n"
            "--> Invalid document pointer: %1",
            validator.GetInvalidSchemaKeyword(),
            sb.GetString());
        return false;
    }

    template < typename Component >
    bool typed_factory_creator<Component>::fill_prototype(
        ecs::prototype& prototype,
        const factory_loader<>::fill_context& ctx) const
    {
        Component component;
        prototype.apply_to_component(component);

        if ( !loader_(component, ctx) ) {
            return false;
        }

        prototype.component<Component>(component);
        return true;
    }

    template < typename Component >
    bool typed_factory_creator<Component>::collect_dependencies(
        asset_dependencies& dependencies,
        const factory_loader<>::collect_context& ctx) const
    {
        return loader_(dependencies, ctx);
    }

    //
    // factory
    //

    template < typename Component >
    factory& factory::register_component(str_hash type) {
        std::lock_guard<std::mutex> guard(mutex_);
        if ( creators_.count(type) > 0 ) {
            throw bad_factory_operation();
        }
        factory_creator_iptr creator(new typed_factory_creator<Component>());
        creators_.emplace(type, std::move(creator));
        return *this;
    }
}

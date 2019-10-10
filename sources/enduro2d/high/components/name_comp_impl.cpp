/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "name_comp_impl.hpp"

namespace e2d
{
    const char* factory_loader<name_comp_impl>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "name" : { "type" : "string" }
        }
    })json";

    bool factory_loader<name_comp_impl>::operator()(
        name_comp_impl& component,
        const fill_context& ctx) const
    {
        E2D_ASSERT(ctx.root.HasMember("name"));
        component.name(ctx.root["name"].GetString());
        return true;
    }

    bool factory_loader<name_comp_impl>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

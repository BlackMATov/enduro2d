/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/convex_hull_screenspace_collider.hpp>

namespace e2d
{
    const char* factory_loader<convex_hull_screenspace_collider>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
        }
    })json";
    
    bool factory_loader<convex_hull_screenspace_collider>::operator()(
        convex_hull_screenspace_collider& component,
        const fill_context& ctx) const
    {
        // TODO
        return true;
    }

    bool factory_loader<convex_hull_screenspace_collider>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

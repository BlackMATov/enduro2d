/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/pivot_2d.hpp>

namespace e2d
{
    pivot_2d::pivot_2d(const v2f& p)
    : pivot_(p) {}

    pivot_2d& pivot_2d::pivot(const v2f& value) noexcept {
        pivot_ = value;
        return *this;
    }

    const v2f& pivot_2d::pivot() const noexcept {
        return pivot_;
    }

    const char* factory_loader<pivot_2d>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "pivot" : { "$ref": "#/common_definitions/v2" }
        }
    })json";

    bool factory_loader<pivot_2d>::operator()(
        pivot_2d& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("pivot") ) {
            v2f pivot;
            if ( !json_utils::try_parse_value(ctx.root["pivot"], pivot) ) {
                the<debug>().error("PIVOT_2D: Incorrect formatting of 'pivot' property");
            }
            component.pivot(pivot);
        }
        return true;
    }

    bool factory_loader<pivot_2d>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

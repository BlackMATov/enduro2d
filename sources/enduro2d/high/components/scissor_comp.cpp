/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/scissor_comp.hpp>

namespace e2d
{
    scissor_comp& scissor_comp::rect(const b2u& value) noexcept {
        rect_ = value;
        return *this;
    }

    const b2u& scissor_comp::rect() const noexcept {
        return rect_;
    }

    const char* factory_loader<scissor_comp>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "rect" : { "$ref": "#/common_definitions/b2" }
        }
    })json";

    bool factory_loader<scissor_comp>::operator()(
        scissor_comp& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("rect") ) {
            b2u r;
            if ( !json_utils::try_parse_value(ctx.root["rect"], r) ) {
                the<debug>().error("SCISSOR_COMP: Incorrect formatting of 'rect' property");
                return false;
            }
            component.rect(r);
        }
        return true;
    }

    bool factory_loader<scissor_comp>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

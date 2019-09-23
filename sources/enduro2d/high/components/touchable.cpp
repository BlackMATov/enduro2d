/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/touchable.hpp>

namespace e2d
{
    touchable::touchable(bool stop)
    : stop_propagation_(stop) {}
    
    touchable& touchable::stop_propagation(bool value) noexcept {
        stop_propagation_ = value;
        return *this;
    }

    bool touchable::stop_propagation() const noexcept {
        return stop_propagation_;
    }

    touchable::capture::capture(u32 depth, bool stop, const input_event::data_ptr& ev_data)
    : depth_(depth)
    , stop_propagation_(stop)
    , ev_data_(ev_data) {}
        
    bool touchable::capture::stop_propagation() const noexcept {
        return stop_propagation_;
    }

    u32 touchable::capture::depth() const noexcept {
        return depth_;
    }

    const input_event::data_ptr& touchable::capture::data() const noexcept {
        return ev_data_;
    }
    
    const char* factory_loader<touchable>::schema_source = R"json({
        "type" : "object",
        "required" : [],
        "additionalProperties" : false,
        "properties" : {
            "stop_propagation" : { "type" : "boolean" }
        }
    })json";

    bool factory_loader<touchable>::operator()(
        touchable& component,
        const fill_context& ctx) const
    {
        if ( ctx.root.HasMember("stop_propagation") ) {
            component.stop_propagation(ctx.root["stop_propagation"].GetBool());
        }
        return true;
    }

    bool factory_loader<touchable>::operator()(
        asset_dependencies& dependencies,
        const collect_context& ctx) const
    {
        E2D_UNUSED(dependencies, ctx);
        return true;
    }
}

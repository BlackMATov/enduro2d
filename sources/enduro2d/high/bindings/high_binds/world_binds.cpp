/*******************************************************************************
* This file is part of the "Enduro2D"
* For conditions of distribution and use, see copyright notice in LICENSE.md
* Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
******************************************************************************/

#include "_high_binds.hpp"

#include <enduro2d/high/world.hpp>

namespace e2d::bindings::high
{
    void bind_world(sol::state& l) {
        l["e2d"].get_or_create<sol::table>()
        ["high"].get_or_create<sol::table>()
        .new_usertype<world>("world");
    }
}

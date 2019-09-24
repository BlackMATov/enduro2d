/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../library.hpp"
#include "../sprite_9p.hpp"

namespace e2d
{
    class sprite_9p_asset final : public content_asset<sprite_9p_asset, sprite_9p> {
    public:
        static const char* type_name() noexcept { return "sprite_9p_asset"; }
        static load_async_result load_async(const library& library, str_view address);
    };
}

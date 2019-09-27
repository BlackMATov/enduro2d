/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../library.hpp"

namespace e2d
{
    class cbuffer_template_asset final : public content_asset<cbuffer_template_asset, cbuffer_template_cptr> {
    public:
        static const char* type_name() noexcept { return "cbuffer_template_asset"; }
        static load_async_result load_async(const library& library, str_view address);
    };
}

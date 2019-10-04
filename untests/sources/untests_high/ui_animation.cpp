/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_high.hpp"
using namespace e2d;

TEST_CASE("ui_animation") {
    SECTION("custom") {
        ui_animation::parallel()
            .add(ui_animation::custom([](f32 f) {}))
            .add(ui_animation::custom([](f32 f, actor& act) { act.node()->scale(v3f(math::lerp(1.0f, 2.0f, f))); }));
    }
}

/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_base.hpp"
#include "configs.hpp"

//
// E2D_ASSERT
//

#if defined(E2D_BUILD_MODE) && E2D_BUILD_MODE == E2D_BUILD_MODE_DEBUG
#  if defined(E2D_COMPILER) && E2D_COMPILER == E2D_COMPILER_MSVC
#    include <crtdbg.h>
#    define E2D_ASSERT(expr)          _ASSERT_EXPR((expr), nullptr)
#    define E2D_ASSERT_MSG(expr, msg) _ASSERT_EXPR((expr), _CRT_WIDE(msg))
#  else
#    define E2D_ASSERT(expr)          assert((expr))
#    define E2D_ASSERT_MSG(expr, msg) assert((expr) && (msg))
#  endif
#else
#  define E2D_ASSERT(expr)            ((void)0)
#  define E2D_ASSERT_MSG(expr, msg)   ((void)0)
#endif

//
// E2D_ALLOCA
//

#if defined(E2D_COMPILER) && E2D_COMPILER == E2D_COMPILER_MSVC
#  define E2D_ALLOCA(size) _alloca((size))
#else
#  define E2D_ALLOCA(size) alloca((size))
#endif

#define E2D_CLEAR_ALLOCA(size)\
    std::memset(E2D_ALLOCA((size)), 0, (size))

//
// E2D_UNUSED
//

template < typename... Ts >
constexpr void E2D_UNUSED(Ts&&...) noexcept {}

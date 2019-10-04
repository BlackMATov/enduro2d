/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

namespace e2d::detail
{
    template < typename T >
    struct func_info_;
    
    template < typename R, typename... Args >
    struct func_info_<R (Args...)> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = void;
    };

    template < typename R, typename... Args >
    struct func_info_<R (*)(Args...)> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = void;
    };

    template < typename R, typename... Args >
    struct func_info_<R (Args...) noexcept> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = void;
    };

    template < typename R, typename... Args >
    struct func_info_<R (*)(Args...) noexcept> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = void;
    };
    
    template < typename Cl, typename R, typename... Args >
    struct func_info_<R (Cl::*)(Args...)> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = Cl;
    };

    #define DEFINE_FUNC_INFO(cv_qual) \
        template < typename Cl, typename R, typename... Args > \
        struct func_info_<R (Cl::*)(Args...) cv_qual> { \
            using args = std::tuple<Args...>; \
            using result = R; \
            using clazz = Cl; \
        };
    DEFINE_FUNC_INFO(const);
    DEFINE_FUNC_INFO(volatile);
    DEFINE_FUNC_INFO(const volatile);
    DEFINE_FUNC_INFO(noexcept);
    DEFINE_FUNC_INFO(const noexcept);
    DEFINE_FUNC_INFO(volatile noexcept);
    DEFINE_FUNC_INFO(const volatile noexcept);
    DEFINE_FUNC_INFO(&);
    DEFINE_FUNC_INFO(const &);
    DEFINE_FUNC_INFO(volatile &);
    DEFINE_FUNC_INFO(const volatile &);
    DEFINE_FUNC_INFO(& noexcept);
    DEFINE_FUNC_INFO(const & noexcept);
    DEFINE_FUNC_INFO(volatile & noexcept);
    DEFINE_FUNC_INFO(const volatile & noexcept);
    DEFINE_FUNC_INFO(&&);
    DEFINE_FUNC_INFO(const &&);
    DEFINE_FUNC_INFO(volatile &&);
    DEFINE_FUNC_INFO(const volatile &&);
    DEFINE_FUNC_INFO(&& noexcept);
    DEFINE_FUNC_INFO(const && noexcept);
    DEFINE_FUNC_INFO(volatile && noexcept);
    DEFINE_FUNC_INFO(const volatile && noexcept);
    #undef DEFINE_FUNC_INFO

    template < typename R, typename... Args >
    struct func_info_<std::function<R (Args...)>> {
        using args = std::tuple<Args...>;
        using result = R;
        using clazz = void;
    };

    template < typename T, bool L >
    struct func_info_1_ {
        using type = func_info_<T>;
    };
    
    template < typename T >
    struct func_info_1_<T, true> {
        using type = func_info_<decltype(&T::operator())>;
    };

    template < typename T >
    struct func_info_2_ {
        using type = typename func_info_1_<T, std::is_class_v<T>>::type;
    };
}

namespace e2d
{
    template < typename T >
    using func_info = typename detail::func_info_2_<T>::type;

    template < typename FN, typename... Args >
    void safe_call(FN&& fn, Args&& ...args) {
        using type = std::remove_reference_t<FN>;
        if constexpr( std::is_function_v<type> ) {
            if ( fn ) {
                fn(std::forward<Args>(args)...);
            }
        } else if constexpr( std::is_object_v<type> ) {
            fn(std::forward<Args>(args)...);
        } else {
            static_assert(false, "unsupported invocable type");
        }
    }
}

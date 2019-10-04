/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_utils.hpp"
using namespace e2d;

class ClTest {
public:
    i32 foo(u32);
    i8 foo2() const volatile && noexcept;
    void foo3(f32) noexcept;
    static f64 foo4();
    friend u64 foo5(i32) noexcept;
    void foo6(i32, i32) volatile & noexcept;
};

TEST_CASE("func_info") {
    {
        using fi = func_info<void(i32)>;
        static_assert(std::tuple_size_v<fi::args> == 1);
        static_assert(std::is_same_v<i32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<void, fi::result>);
    }
    {
        using fi = func_info<void(*)(i32, f32)>;
        static_assert(std::tuple_size_v<fi::args> == 2);
        static_assert(std::is_same_v<i32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<f32, std::tuple_element<1, fi::args>::type>);
        static_assert(std::is_same_v<void, fi::result>);
    }
    {
        using fi = func_info<decltype(&ClTest::foo)>;
        static_assert(std::tuple_size_v<fi::args> == 1);
        static_assert(std::is_same_v<u32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<i32, fi::result>);
        static_assert(std::is_same_v<ClTest, fi::clazz>);
    }
    {
        using fi = func_info<decltype(&ClTest::foo2)>;
        static_assert(std::tuple_size_v<fi::args> == 0);
        static_assert(std::is_same_v<i8, fi::result>);
        static_assert(std::is_same_v<ClTest, fi::clazz>);
    }
    {
        using fi = func_info<decltype(&ClTest::foo3)>;
        static_assert(std::tuple_size_v<fi::args> == 1);
        static_assert(std::is_same_v<f32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<void, fi::result>);
        static_assert(std::is_same_v<ClTest, fi::clazz>);
    }
    {
        using fi = func_info<decltype(&ClTest::foo4)>;
        static_assert(std::tuple_size_v<fi::args> == 0);
        static_assert(std::is_same_v<f64, fi::result>);
        static_assert(std::is_same_v<void, fi::clazz>);
    }
    {
        using fi = func_info<decltype(&foo5)>;
        static_assert(std::tuple_size_v<fi::args> == 1);
        static_assert(std::is_same_v<i32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<u64, fi::result>);
        static_assert(std::is_same_v<void, fi::clazz>);
    }
    {
        using fi = func_info<decltype(&ClTest::foo6)>;
        static_assert(std::tuple_size_v<fi::args> == 2);
        static_assert(std::is_same_v<i32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<i32, std::tuple_element<1, fi::args>::type>);
        static_assert(std::is_same_v<void, fi::result>);
        static_assert(std::is_same_v<ClTest, fi::clazz>);
    }
    {
        auto lambda = [](f32 a, f64 b) { return a + b; };
        using fi = func_info<decltype(lambda)>;
        static_assert(std::is_same_v<f32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<f64, std::tuple_element<1, fi::args>::type>);
        static_assert(std::tuple_size_v<fi::args> == 2);
        static_assert(std::is_same_v<f64, fi::result>);
    }
    {
        i32 i = 0;
        auto lambda = [i](f32 a, f64 b) mutable { ++i; return a + b + f64(i); };
        using fi = func_info<decltype(lambda)>;
        static_assert(std::is_same_v<f32, std::tuple_element<0, fi::args>::type>);
        static_assert(std::is_same_v<f64, std::tuple_element<1, fi::args>::type>);
        static_assert(std::tuple_size_v<fi::args> == 2);
        static_assert(std::is_same_v<f64, fi::result>);
    }
}

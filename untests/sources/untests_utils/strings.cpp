/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_utils.hpp"
using namespace e2d;

namespace
{
    const char* null_utf8 = nullptr;
    const wchar_t* null_wide = nullptr;
    const char16_t* null_utf16 = nullptr;
    const char32_t* null_utf32 = nullptr;

    str_view null_view = {null_utf8, 0};
}

TEST_CASE("strings") {
    {
        REQUIRE(str_hash().empty());
        REQUIRE(str_hash("").empty());
        REQUIRE_FALSE(str_hash("1").empty());

        REQUIRE(str_hash().hash() == str_hash().hash());
        REQUIRE(str_hash().hash() == str_hash({null_utf8,0}).hash());
        REQUIRE(str_hash().hash() == wstr_hash({null_wide,0}).hash());
        REQUIRE(str_hash().hash() == str16_hash({null_utf16,0}).hash());
        REQUIRE(str_hash().hash() == str32_hash({null_utf32,0}).hash());
        REQUIRE(str_hash("hello").hash() == str_hash("hello").hash());
        REQUIRE(str_hash("world").hash() == str_hash("world").hash());
        REQUIRE(str_hash("hello").hash() != str_hash("world").hash());

        REQUIRE(str_hash() == str_hash());
        REQUIRE(str_hash() == make_hash({null_utf8,0}));
        REQUIRE(wstr_hash() == make_hash({null_wide,0}));
        REQUIRE(str16_hash() == make_hash({null_utf16,0}));
        REQUIRE(str32_hash() == make_hash({null_utf32,0}));
        REQUIRE(str_hash("hello") == make_hash("hello"));
        REQUIRE(str_hash("world") == make_hash("world"));
        REQUIRE(str_hash("hello") != make_hash("world"));

        REQUIRE(str_hash("hello").assign("world") == make_hash("world"));
        REQUIRE(str_hash("hello").assign(make_hash("world")) == make_hash("world"));
        {
            str_hash s("hello");
            s.clear();
            REQUIRE(s.empty());
            REQUIRE(s == str_hash());
        }
        {
            str_hash s1("hello");
            str_hash s2("world");
            s1.swap(s2);
            REQUIRE(s2 == str_hash("hello"));
            REQUIRE(s1 == make_hash("world"));
        }
    }
    {
        REQUIRE(make_utf8("hello") == "hello");
        REQUIRE(make_utf8(L"hello") == "hello");
        REQUIRE(make_utf8(u"hello") == "hello");
        REQUIRE(make_utf8(U"hello") == "hello");

        REQUIRE(make_wide("hello") == L"hello");
        REQUIRE(make_wide(L"hello") == L"hello");
        REQUIRE(make_wide(u"hello") == L"hello");
        REQUIRE(make_wide(U"hello") == L"hello");

        REQUIRE(make_utf16("hello") == u"hello");
        REQUIRE(make_utf16(L"hello") == u"hello");
        REQUIRE(make_utf16(u"hello") == u"hello");
        REQUIRE(make_utf16(U"hello") == u"hello");

        REQUIRE(make_utf32("hello") == U"hello");
        REQUIRE(make_utf32(L"hello") == U"hello");
        REQUIRE(make_utf32(u"hello") == U"hello");
        REQUIRE(make_utf32(U"hello") == U"hello");
    }
    {
        REQUIRE(make_utf8(str_view(null_utf8, 0)) == make_utf8(u""));
        REQUIRE(make_utf8(wstr_view(null_wide, 0)) == make_utf8(L""));
        REQUIRE(make_utf8(str16_view(null_utf16, 0)) == make_utf8(u""));
        REQUIRE(make_utf8(str32_view(null_utf32, 0)) == make_utf8(U""));

        REQUIRE(make_wide(str_view(null_utf8, 0)) == make_wide(u""));
        REQUIRE(make_wide(wstr_view(null_wide, 0)) == make_wide(L""));
        REQUIRE(make_wide(str16_view(null_utf16, 0)) == make_wide(u""));
        REQUIRE(make_wide(str32_view(null_utf32, 0)) == make_wide(U""));

        REQUIRE(make_utf16(str_view(null_utf8, 0)) == make_utf16(u""));
        REQUIRE(make_utf16(wstr_view(null_wide, 0)) == make_utf16(L""));
        REQUIRE(make_utf16(str16_view(null_utf16, 0)) == make_utf16(u""));
        REQUIRE(make_utf16(str32_view(null_utf32, 0)) == make_utf16(U""));

        REQUIRE(make_utf32(str_view(null_utf8, 0)) == make_utf32(u""));
        REQUIRE(make_utf32(wstr_view(null_wide, 0)) == make_utf32(L""));
        REQUIRE(make_utf32(str16_view(null_utf16, 0)) == make_utf32(u""));
        REQUIRE(make_utf32(str32_view(null_utf32, 0)) == make_utf32(U""));
    }
    {
        using strings::try_parse;

        {
            i8 v{111};
            REQUIRE((try_parse("42", v) && v == 42));
            REQUIRE((try_parse("+42", v) && v == 42));
            REQUIRE((try_parse("127", v) && v == 127));
            REQUIRE((try_parse("0", v) && v == 0));
            REQUIRE((try_parse("0xF", v) && v == 15));
            REQUIRE((try_parse("052", v) && v == 42));
            REQUIRE((try_parse("-052", v) && v == -42));
            REQUIRE((try_parse("-128", v) && v == -128));

            u8 uv{111};
            REQUIRE((try_parse("42", uv) && uv == 42));
            REQUIRE((try_parse("+42", uv) && uv == 42));
            REQUIRE((try_parse("255", uv) && uv == 255));
            REQUIRE((try_parse("0xFF", uv) && uv == 255));
            REQUIRE((try_parse("052", uv) && uv == 42));
            REQUIRE((try_parse("0", uv) && uv == 0));
        }
        {
            i8 v{111};
            REQUIRE((!try_parse(str_view(), v) && v == 111));
            REQUIRE((!try_parse("", v) && v == 111));
            REQUIRE((!try_parse("  \t", v) && v == 111));
            REQUIRE((!try_parse("42hello", v) && v == 111));
            REQUIRE((!try_parse("world42", v) && v == 111));
            REQUIRE((!try_parse("42 ", v) && v == 111));
            REQUIRE((!try_parse("-129", v) && v == 111));
            REQUIRE((!try_parse("128", v) && v == 111));
            REQUIRE((!try_parse("4.2", v) && v == 111));

            u8 uv{111};
            REQUIRE((!try_parse(str_view(), uv) && uv == 111));
            REQUIRE((!try_parse("", uv) && uv == 111));
            REQUIRE((!try_parse("  \t", uv) && uv == 111));
            REQUIRE((!try_parse("42hello", uv) && uv == 111));
            REQUIRE((!try_parse("world42", uv) && uv == 111));
            REQUIRE((!try_parse("42 ", uv) && uv == 111));
            REQUIRE((!try_parse("-1", uv) && uv == 111));
            REQUIRE((!try_parse("256", uv) && uv == 111));
            REQUIRE((!try_parse("4.2", uv) && uv == 111));
        }
        {
            f32 v32{11.22f};
            REQUIRE((try_parse("4.23E5", v32) && math::approximately(v32, 4.23e5f)));
            REQUIRE((try_parse("4.23E-5", v32) && math::approximately(v32, 4.23e-5f)));
            REQUIRE((try_parse("4.23E+5", v32) && math::approximately(v32, 4.23e+5f)));
            REQUIRE((try_parse("42", v32) && math::approximately(v32, 42.f)));
            REQUIRE((try_parse("+42", v32) && math::approximately(v32, 42.f)));
            REQUIRE((try_parse("-2.43", v32) && math::approximately(v32, -2.43f)));
            REQUIRE((try_parse("-24", v32) && math::approximately(v32, -24.f)));

            f64 v64{11.22f};
            REQUIRE((try_parse("4.23E5", v64) && math::approximately(v64, 4.23e5)));
            REQUIRE((try_parse("4.23E-5", v64) && math::approximately(v64, 4.23e-5)));
            REQUIRE((try_parse("4.23E+5", v64) && math::approximately(v64, 4.23e+5)));
            REQUIRE((try_parse("42", v64) && math::approximately(v64, 42.)));
            REQUIRE((try_parse("+42", v64) && math::approximately(v64, 42.)));
            REQUIRE((try_parse("-2.43", v64) && math::approximately(v64, -2.43)));
            REQUIRE((try_parse("-24", v64) && math::approximately(v64, -24.)));
        }
        {
            f32 v32{11.22f};
            REQUIRE((!try_parse("", v32) && math::approximately(v32, 11.22f)));
            REQUIRE((!try_parse("  \t", v32) && math::approximately(v32, 11.22f)));
            REQUIRE((!try_parse("1.0E100", v32) && math::approximately(v32, 11.22f)));
            REQUIRE((!try_parse("1..4", v32) && math::approximately(v32, 11.22f)));
            REQUIRE((!try_parse("..14", v32) && math::approximately(v32, 11.22f)));
            REQUIRE((!try_parse("14..", v32) && math::approximately(v32, 11.22f)));

            f64 v64{11.22};
            REQUIRE((!try_parse("", v64) && math::approximately(v64, 11.22)));
            REQUIRE((!try_parse("  \t", v64) && math::approximately(v64, 11.22)));
            REQUIRE((!try_parse("1.0E400", v64) && math::approximately(v64, 11.22)));
            REQUIRE((!try_parse("1..4", v64) && math::approximately(v64, 11.22)));
            REQUIRE((!try_parse("..14", v64) && math::approximately(v64, 11.22)));
            REQUIRE((!try_parse("14..", v64) && math::approximately(v64, 11.22)));
        }
    }
    {
        using strings::wildcard_match;

        char invalid_utf[] = "\xe6\x97\xa5\xd1\x88\xfa";
        REQUIRE_THROWS_AS(wildcard_match(invalid_utf, "???"), std::exception);

        const auto mark_string = [](const char* str) -> str_view {
            static char string_buf[1024] = {0};
            std::memset(string_buf, '*', sizeof(string_buf));
            const std::size_t str_len = std::strlen(str);
            E2D_ASSERT(str_len < sizeof(string_buf));
            std::memcpy(string_buf, str, str_len);
            return str_view(string_buf, str_len);
        };

        const auto mark_pattern = [](const char* str) -> str_view {
            static char pattern_buf[1024] = {0};
            std::memset(pattern_buf, '*', sizeof(pattern_buf));
            const std::size_t str_len = std::strlen(str);
            E2D_ASSERT(str_len < sizeof(pattern_buf));
            std::memcpy(pattern_buf, str, str_len);
            return str_view(pattern_buf, str_len);
        };

        // 你好!
        REQUIRE(wildcard_match(u8"\u4F60\u597D!", u8"\u4F60\u597D!") == true);
        REQUIRE(wildcard_match(u8"\u4F60\u597D!", u8"?\u597D!") == true);
        REQUIRE(wildcard_match(u8"\u4F60\u597D!", u8"\u4F60?!") == true);

        REQUIRE(wildcard_match(
            // 你好你好你好你好世界世界世界世界世界世界世界世界彡ಠ
            mark_string(u8"\u4F60\u597D\u4F60\u597D\u4F60\u597D\u4F60\u597D\u4E16\u754C\u4E16\u754C\u4E16\u754C\u4E16\u754C\u4E16\u754C\u4E16\u754C\u4E16\u754C\u4E16\u754C\u5F61\u0CA0"),
            // 你好你好你好你好*世界世界彡*ಠ
            mark_pattern(u8"\u4F60\u597D\u4F60\u597D\u4F60\u597D\u4F60\u597D*\u4E16\u754C\u4E16\u754C\u5F61*\u0CA0")) == true);

        REQUIRE(wildcard_match("", "") == true);
        REQUIRE(wildcard_match("a", "") == false);
        REQUIRE(wildcard_match("", "*") == true);
        REQUIRE(wildcard_match("", "?") == false);

        REQUIRE(wildcard_match(null_view, null_view) == true);
        REQUIRE(wildcard_match("a", null_view) == false);
        REQUIRE(wildcard_match(null_view, "*") == true);
        REQUIRE(wildcard_match(null_view, "?") == false);

        // tests source:
        // http://developforperformance.com/MatchingWildcards_AnImprovedAlgorithmForBigData.html

        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("ab*d")) == false);

        REQUIRE(wildcard_match(mark_string("abcccd"), mark_pattern("*ccd")) == true);
        REQUIRE(wildcard_match(mark_string("mississipissippi"), mark_pattern("*issip*ss*")) == true);
        REQUIRE(wildcard_match(mark_string("xxxx*zzzzzzzzy*f"), mark_pattern("xxxx*zzy*fffff")) == false);
        REQUIRE(wildcard_match(mark_string("xxxx*zzzzzzzzy*f"), mark_pattern("xxx*zzy*f")) == true);
        REQUIRE(wildcard_match(mark_string("xxxxzzzzzzzzyf"), mark_pattern("xxxx*zzy*fffff")) == false);
        REQUIRE(wildcard_match(mark_string("xxxxzzzzzzzzyf"), mark_pattern("xxxx*zzy*f")) == true);
        REQUIRE(wildcard_match(mark_string("xyxyxyzyxyz"), mark_pattern("xy*z*xyz")) == true);
        REQUIRE(wildcard_match(mark_string("mississippi"), mark_pattern("*sip*")) == true);
        REQUIRE(wildcard_match(mark_string("xyxyxyxyz"), mark_pattern("xy*xyz")) == true);
        REQUIRE(wildcard_match(mark_string("mississippi"), mark_pattern("mi*sip*")) == true);
        REQUIRE(wildcard_match(mark_string("ababac"), mark_pattern("*abac*")) == true);
        REQUIRE(wildcard_match(mark_string("ababac"), mark_pattern("*abac*")) == true);
        REQUIRE(wildcard_match(mark_string("aaazz"), mark_pattern("a*zz*")) == true);
        REQUIRE(wildcard_match(mark_string("a12b12"), mark_pattern("*12*23")) == false);
        REQUIRE(wildcard_match(mark_string("a12b12"), mark_pattern("a12b")) == false);
        REQUIRE(wildcard_match(mark_string("a12b12"), mark_pattern("*12*12*")) == true);

        REQUIRE(wildcard_match(mark_string("caaab"), mark_pattern("*a?b")) == true);

        REQUIRE(wildcard_match(mark_string("*"), mark_pattern("*")) == true);
        REQUIRE(wildcard_match(mark_string("a*abab"), mark_pattern("a*b")) == true);
        REQUIRE(wildcard_match(mark_string("a*r"), mark_pattern("a*")) == true);
        REQUIRE(wildcard_match(mark_string("a*ar"), mark_pattern("a*aar")) == false);

        REQUIRE(wildcard_match(mark_string("XYXYXYZYXYz"), mark_pattern("XY*Z*XYz")) == true);
        REQUIRE(wildcard_match(mark_string("missisSIPpi"), mark_pattern("*SIP*")) == true);
        REQUIRE(wildcard_match(mark_string("mississipPI"), mark_pattern("*issip*PI")) == true);
        REQUIRE(wildcard_match(mark_string("xyxyxyxyz"), mark_pattern("xy*xyz")) == true);
        REQUIRE(wildcard_match(mark_string("miSsissippi"), mark_pattern("mi*sip*")) == true);
        REQUIRE(wildcard_match(mark_string("miSsissippi"), mark_pattern("mi*Sip*")) == false);
        REQUIRE(wildcard_match(mark_string("abAbac"), mark_pattern("*Abac*")) == true);
        REQUIRE(wildcard_match(mark_string("abAbac"), mark_pattern("*Abac*")) == true);
        REQUIRE(wildcard_match(mark_string("aAazz"), mark_pattern("a*zz*")) == true);
        REQUIRE(wildcard_match(mark_string("A12b12"), mark_pattern("*12*23")) == false);
        REQUIRE(wildcard_match(mark_string("a12B12"), mark_pattern("*12*12*")) == true);
        REQUIRE(wildcard_match(mark_string("oWn"), mark_pattern("*oWn*")) == true);

        REQUIRE(wildcard_match(mark_string("bLah"), mark_pattern("bLah")) == true);
        REQUIRE(wildcard_match(mark_string("bLah"), mark_pattern("bLaH")) == false);

        REQUIRE(wildcard_match(mark_string("a"), mark_pattern("*?")) == true);
        REQUIRE(wildcard_match(mark_string("ab"), mark_pattern("*?")) == true);
        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("*?")) == true);

        REQUIRE(wildcard_match(mark_string("a"), mark_pattern("??")) == false);
        REQUIRE(wildcard_match(mark_string("ab"), mark_pattern("?*?")) == true);
        REQUIRE(wildcard_match(mark_string("ab"), mark_pattern("*?*?*")) == true);
        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("?**?*?")) == true);
        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("?**?*&?")) == false);
        REQUIRE(wildcard_match(mark_string("abcd"), mark_pattern("?b*??")) == true);
        REQUIRE(wildcard_match(mark_string("abcd"), mark_pattern("?a*??")) == false);
        REQUIRE(wildcard_match(mark_string("abcd"), mark_pattern("?**?c?")) == true);
        REQUIRE(wildcard_match(mark_string("abcd"), mark_pattern("?**?d?")) == false);
        REQUIRE(wildcard_match(mark_string("abcde"), mark_pattern("?*b*?*d*?")) == true);

        REQUIRE(wildcard_match(mark_string("bLah"), mark_pattern("bL?h")) == true);
        REQUIRE(wildcard_match(mark_string("bLaaa"), mark_pattern("bLa?")) == false);
        REQUIRE(wildcard_match(mark_string("bLah"), mark_pattern("bLa?")) == true);
        REQUIRE(wildcard_match(mark_string("bLaH"), mark_pattern("?Lah")) == false);
        REQUIRE(wildcard_match(mark_string("bLaH"), mark_pattern("?LaH")) == true);

        REQUIRE(wildcard_match(
            mark_string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"),
            mark_pattern("a*a*a*a*a*a*aa*aaa*a*a*b")) == true);
        REQUIRE(wildcard_match(
            mark_string("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"),
            mark_pattern("*a*b*ba*ca*a*aa*aaa*fa*ga*b*")) == true);
        REQUIRE(wildcard_match(
            mark_string("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"),
            mark_pattern("*a*b*ba*ca*a*x*aaa*fa*ga*b*")) == false);
        REQUIRE(wildcard_match(
            mark_string("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"),
            mark_pattern("*a*b*ba*ca*aaaa*fa*ga*gggg*b*")) == false);
        REQUIRE(wildcard_match(
            mark_string("abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"),
            mark_pattern("*a*b*ba*ca*aaaa*fa*ga*ggg*b*")) == true);
        REQUIRE(wildcard_match(
            mark_string("aaabbaabbaab"),
            mark_pattern("*aabbaa*a*")) == true);
        REQUIRE(wildcard_match(
            mark_string("a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*"),
            mark_pattern("a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*")) == true);
        REQUIRE(wildcard_match(
            mark_string("aaaaaaaaaaaaaaaaa"),
            mark_pattern("*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*")) == true);
        REQUIRE(wildcard_match(
            mark_string("aaaaaaaaaaaaaaaa"),
            mark_pattern("*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*")) == false);
        REQUIRE(wildcard_match(
            mark_string("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn"),
            mark_pattern("abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*")) == false);
        REQUIRE(wildcard_match(
            mark_string("abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn"),
            mark_pattern("abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*")) == true);
        REQUIRE(wildcard_match(
            mark_string("abc*abcd*abcd*abc*abcd"),
            mark_pattern("abc*abc*abc*abc*abc")) == false);
        REQUIRE(wildcard_match(
            mark_string("abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd"),
            mark_pattern("abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd")) == true);
        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("********a********b********c********")) == true);
        REQUIRE(wildcard_match(mark_string("********a********b********c********"), mark_pattern("abc")) == false);
        REQUIRE(wildcard_match(mark_string("abc"), mark_pattern("********a********b********b********")) == false);
        REQUIRE(wildcard_match(mark_string("*abc*"), mark_pattern("***a*b*c***")) == true);
    }
    {
        REQUIRE(strings::starts_with("", ""));
        REQUIRE(strings::starts_with("hello_world", ""));
        REQUIRE(strings::starts_with("hello_world", "hello"));
        REQUIRE(strings::starts_with("hello_world", "hello_world"));
        REQUIRE_FALSE(strings::starts_with("", "a"));
        REQUIRE_FALSE(strings::starts_with("hello_world", "world"));
        REQUIRE_FALSE(strings::starts_with("hello_world", "hello_world_42"));
        REQUIRE_FALSE(strings::starts_with("hello_world", "42_hello_world"));

        REQUIRE(strings::ends_with("", ""));
        REQUIRE(strings::ends_with("hello_world", ""));
        REQUIRE(strings::ends_with("hello_world", "world"));
        REQUIRE(strings::ends_with("hello_world", "hello_world"));
        REQUIRE_FALSE(strings::ends_with("", "a"));
        REQUIRE_FALSE(strings::ends_with("hello_world", "hello"));
        REQUIRE_FALSE(strings::ends_with("hello_world", "hello_world_42"));
        REQUIRE_FALSE(strings::ends_with("hello_world", "42_hello_world"));
    }
    {
        char buf[6];
        REQUIRE_THROWS_AS(strings::format(buf, 0, "hello"), strings::bad_format_buffer);
        REQUIRE_THROWS_AS(strings::format(nullptr, sizeof(buf), "hello"), strings::bad_format_buffer);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "helloE"), strings::bad_format_buffer);
        REQUIRE_NOTHROW(strings::format(buf, sizeof(buf), "hello"));
        REQUIRE_NOTHROW(strings::format(nullptr, 0, "hello"));

        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%hell"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "he%ll"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "hell%"), strings::bad_format);

        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%10%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "hell%10%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%10%hell"), strings::bad_format);

        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%x%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "hell%y%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%z%hell"), strings::bad_format);
    }
    {
        REQUIRE_THROWS_AS(strings::rformat("%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("%hell"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("he%ll"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("hell%"), strings::bad_format);

        REQUIRE_THROWS_AS(strings::rformat("%10%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("hell%10%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("%10%hell"), strings::bad_format);

        REQUIRE_THROWS_AS(strings::rformat("%x%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("hell%y%"), strings::bad_format);
        REQUIRE_THROWS_AS(strings::rformat("%z%hell"), strings::bad_format);
    }
    {
        str s;
        REQUIRE(strings::rformat_nothrow(s, "%0", "hello"));
        REQUIRE(s == "hello");
        REQUIRE_FALSE(strings::rformat_nothrow(s, "%"));
        char buf[5] = {0};
        std::size_t length = 0;
        REQUIRE(strings::format_nothrow(buf, std::size(buf), &length, "%0", "hell"));
        REQUIRE(length == 4);
        REQUIRE(str(buf) == str("hell"));
        REQUIRE_FALSE(strings::format_nothrow(buf, std::size(buf), &length, "%0", "hello"));
    }
    {
        REQUIRE(strings::rformat(str_view("%0"), 42) == "42");
        REQUIRE(strings::rformat(str_view("%0%1",2), 42) == "42");

        auto s1 = make_utf8("hello");
        auto s2 = make_wide("hello");
        auto s3 = make_utf16("hello");
        auto s4 = make_utf32("hello");

        REQUIRE(strings::rformat(str_view("%0"), str_view(s1)) == "hello");
        REQUIRE(strings::rformat(str_view("%0"), wstr_view(s2)) == "hello");
        REQUIRE(strings::rformat(str_view("%0"), str16_view(s3)) == "hello");
        REQUIRE(strings::rformat(str_view("%0"), str32_view(s4)) == "hello");
    }
    {
        char buf[1];

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE(strings::format(buf, sizeof(buf), "") == 0);
        REQUIRE(str(buf) == str(""));
    }
    {
        char buf[6];

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE(strings::format(buf, sizeof(buf), "hello") == 5);
        REQUIRE(strings::format(nullptr, 0, "hello") == 5);
        REQUIRE(str(buf) == str("hello"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE(strings::format(buf, sizeof(buf), "hell%%") == 5);
        REQUIRE(strings::format(nullptr, 0, "hell%%") == 5);
        REQUIRE(str(buf) == str("hell%"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE(strings::format(buf, sizeof(buf), "%%hell") == 5);
        REQUIRE(strings::format(nullptr, 0, "%%hell") == 5);
        REQUIRE(str(buf) == str("%hell"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE(strings::format(buf, sizeof(buf), "he%%ll") == 5);
        REQUIRE(strings::format(nullptr, 0, "he%%ll") == 5);
        REQUIRE(str(buf) == str("he%ll"));
    }
    {
        char buf[5];
        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "hello"), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("hell"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "he%"), strings::bad_format);
        REQUIRE(str(buf) == str("he"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "he%99%"), strings::bad_format);
        REQUIRE(str(buf) == str("he"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "he%x%"), strings::bad_format);
        REQUIRE(str(buf) == str("he"));

        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "he%0%", 1234), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("he12"));
    }
    {
        char buf[10];
        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%0%", "hello world"), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("hello wor"));
        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "test%0%", "hello world"), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("testhello"));
        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "%0%test", "hello world"), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("hello wor"));
        std::memset(buf, 0xAB, sizeof(buf));
        REQUIRE_THROWS_AS(strings::format(buf, sizeof(buf), "te%0%st", "hello world"), strings::bad_format_buffer);
        REQUIRE(str(buf) == str("tehello w"));
    }
    {
        REQUIRE(strings::rformat("%0 %1 %2", "hello", "world", 5) == str("hello world 5"));
        REQUIRE(strings::rformat("%1 %0 %2", "hello", "world", 5) == str("world hello 5"));
        REQUIRE(strings::rformat("%2 %1 %0", "hello", "world", 5) == str("5 world hello"));
        REQUIRE(strings::rformat("%0 %0 %1", "hello", "world", 5) == str("hello hello world"));
        REQUIRE(strings::rformat("%2 %1 %1", "hello", "world", 5) == str("5 world world"));

        REQUIRE(
            strings::rformat(
                "%0 %2 %1 %4 %3 %6 %7 %5 %8 %9",
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9) ==
            str("0 2 1 4 3 6 7 5 8 9"));
    }
    {
        REQUIRE(strings::rformat("%0", static_cast<char*>(nullptr)).empty());
        REQUIRE(strings::rformat("%0", static_cast<const char*>(nullptr)).empty());
    }
    {
        REQUIRE(strings::rformat("%0", strings::make_format_arg("ab", u8(4))) == "  ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg("ab", u8(3))) == " ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg("ab", u8(2))) == "ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg("ab", u8(1))) == "ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg("ab", u8(0))) == "ab");

        REQUIRE(strings::rformat("%0", strings::make_format_arg(str("ab"), u8(4))) == "  ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(str("ab"), u8(3))) == " ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(str("ab"), u8(2))) == "ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(str("ab"), u8(1))) == "ab");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(str("ab"), u8(0))) == "ab");

        {
            auto s1 = make_utf8("ab");
            auto s2 = make_wide("ab");
            auto s3 = make_utf16("ab");
            auto s4 = make_utf32("ab");

            REQUIRE(strings::rformat("%0", strings::make_format_arg(s1, u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(s2, u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(s3, u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(s4, u8(4))) == "  ab");

            REQUIRE(strings::rformat("%0", strings::make_format_arg(str_view(s1), u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(wstr_view(s2), u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(str16_view(s3), u8(4))) == "  ab");
            REQUIRE(strings::rformat("%0", strings::make_format_arg(str32_view(s4), u8(4))) == "  ab");
        }
    }
    {
        REQUIRE(strings::rformat("%0", strings::make_format_arg(-5, u8(3))) == " -5");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(-5, u8(4))) == "  -5");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(21, u8(1))) == "21");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(21, u8(2))) == "21");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(42, u8(3))) == " 42");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(42u, u8(3))) == " 42");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(1.23f)) == "1.230000");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(1.23f,u8(0))) == "1.230000");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(1.23f,u8(0),u8(2))) == "1.23");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(1.23f,u8(5),u8(2))) == " 1.23");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(true)) == "true");
        REQUIRE(strings::rformat("%0", strings::make_format_arg(false)) == "false");

        const char* s0 = "hello";
        char s1[64];
        std::strcpy(s1, "world");
        REQUIRE(strings::rformat("%0", s0) == "hello");
        REQUIRE(strings::rformat("%0", s1) == "world");
    }
    {
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<i8>::max()) ==
            std::to_string(std::numeric_limits<i8>::max()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<i8>::min()) ==
            std::to_string(std::numeric_limits<i8>::min()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<i64>::max()) ==
            std::to_string(std::numeric_limits<i64>::max()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<i64>::min()) ==
            std::to_string(std::numeric_limits<i64>::min()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<u64>::max()) ==
            std::to_string(std::numeric_limits<u64>::max()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<f32>::max()) ==
            std::to_string(std::numeric_limits<f32>::max()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<f32>::min()) ==
            std::to_string(std::numeric_limits<f32>::min()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<f64>::max()) ==
            std::to_string(std::numeric_limits<f64>::max()));
        REQUIRE(
            strings::rformat("%0", std::numeric_limits<f64>::min()) ==
            std::to_string(std::numeric_limits<f64>::min()));
    }
    SECTION("regression") {
        {
            // https://github.com/enduro2d/enduro2d/issues/10
            {
                REQUIRE(strings::rformat("%0", "\0") == "");
                REQUIRE(strings::format(nullptr, 0, "%0", "\0") == 0);
            }
            {
                char buf[1];
                strings::format(buf, std::size(buf), "%0", "\0");
                REQUIRE(str(buf) == str(""));
            }
            {
                char buf[2];
                REQUIRE_THROWS_AS(strings::format(buf, std::size(buf), "%0%1", 1, 20), strings::bad_format_buffer);
                REQUIRE(str(buf) == str("1"));
                REQUIRE_THROWS_AS(strings::format(buf, std::size(buf), "%0%1", 20, 1), strings::bad_format_buffer);
                REQUIRE(str(buf) == str("2"));
            }
        }
    }
    SECTION("performance") {
        std::printf("-= strings::performance tests =-\n");
    #if defined(E2D_BUILD_MODE) && E2D_BUILD_MODE == E2D_BUILD_MODE_DEBUG
        const std::size_t task_n = 100'000;
    #else
        const std::size_t task_n = 1'000'000;
    #endif
        {
            std::size_t result = 0;
            e2d_untests::verbose_profiler_ms p("format(int, int)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += strings::format(buffer, sizeof(buffer), "hello %0 world %1 !", 1000, 123);
            }
            p.done(result);
        }
        {
            std::ptrdiff_t result = 0;
            e2d_untests::verbose_profiler_ms p("snprintf(int, int)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += std::snprintf(buffer, sizeof(buffer), "hello %i world %i !", 1000, 123);
            }
            p.done(result);
        }
        {
            std::ptrdiff_t result = 0;
            e2d_untests::verbose_profiler_ms p("format(float, float)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += strings::format(buffer, sizeof(buffer), "hello %0 world %1 !", 1000.f, 123.f);
            }
            p.done(result);
        }
        {
            std::ptrdiff_t result = 0;
            e2d_untests::verbose_profiler_ms p("snprintf(float, float)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += std::snprintf(buffer, sizeof(buffer), "hello %f world %f !", 1000.0, 123.0);
            }
            p.done(result);
        }
        {
            std::ptrdiff_t result = 0;
            e2d_untests::verbose_profiler_ms p("format(const char*)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += strings::format(buffer, sizeof(buffer), "hello %0 world %1 !", "foo", "bar");
            }
            p.done(result);
        }
        {
            std::ptrdiff_t result = 0;
            e2d_untests::verbose_profiler_ms p("snprintf(const char*)");
            for ( std::size_t i = 0; i < task_n; ++i ) {
                char buffer[128];
                result += std::snprintf(buffer, sizeof(buffer), "hello %s world %s !", "foo", "bar");
            }
            p.done(result);
        }
    }
}

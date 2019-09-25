/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_utils.hpp"

#include <3rdparty/rapidjson/schema.h>
#include <3rdparty/rapidjson/document.h>

namespace e2d::json_utils
{
    void add_common_schema_definitions(rapidjson::Document& schema);
}

namespace e2d::json_utils
{
    bool try_parse_value(const rapidjson::Value& root, v2i& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v3i& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v4i& v) noexcept;

    bool try_parse_value(const rapidjson::Value& root, v2u& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v3u& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v4u& v) noexcept;

    bool try_parse_value(const rapidjson::Value& root, v2f& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v3f& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, v4f& v) noexcept;

    bool try_parse_value(const rapidjson::Value& root, m2f& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, m3f& v) noexcept;
    bool try_parse_value(const rapidjson::Value& root, m4f& v) noexcept;

    bool try_parse_value(const rapidjson::Value& root, q4f& v) noexcept;

    bool try_parse_value(const rapidjson::Value& root, b2i& b) noexcept;
    bool try_parse_value(const rapidjson::Value& root, b3i& b) noexcept;

    bool try_parse_value(const rapidjson::Value& root, b2u& b) noexcept;
    bool try_parse_value(const rapidjson::Value& root, b3u& b) noexcept;

    bool try_parse_value(const rapidjson::Value& root, b2f& b) noexcept;
    bool try_parse_value(const rapidjson::Value& root, b3f& b) noexcept;

    bool try_parse_value(const rapidjson::Value& root, color& c) noexcept;
    bool try_parse_value(const rapidjson::Value& root, color32& c) noexcept;

    bool try_parse_value(const rapidjson::Value& root, str& s) noexcept;
    bool try_parse_value(const rapidjson::Value& root, wstr& s) noexcept;
    bool try_parse_value(const rapidjson::Value& root, str16& s) noexcept;
    bool try_parse_value(const rapidjson::Value& root, str32& s) noexcept;
    bool try_parse_value(const rapidjson::Value& root, str_hash& s) noexcept;
}

namespace e2d::json_utils
{
    inline bool try_parse_value(const rapidjson::Value& root, bool& v) noexcept {
        if ( !root.IsBool() ) {
            return false;
        }
        v = root.GetBool();
        return true;
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> &&
        std::is_signed_v<T>, bool>
    try_parse_value(const rapidjson::Value& root, T& v) noexcept {
        if ( !root.IsInt() ) {
            return false;
        }
        const auto iv = root.GetInt();
        if ( iv < std::numeric_limits<T>::min() ) {
            return false;
        }
        if ( iv > std::numeric_limits<T>::max() ) {
            return false;
        }
        v = math::numeric_cast<T>(iv);
        return true;
    }

    template < typename T >
    std::enable_if_t<
        std::is_integral_v<T> &&
        std::is_unsigned_v<T>, bool>
    try_parse_value(const rapidjson::Value& root, T& v) noexcept {
        if ( !root.IsUint() ) {
            return false;
        }
        const auto uv = root.GetUint();
        if ( uv < std::numeric_limits<T>::min() ) {
            return false;
        }
        if ( uv > std::numeric_limits<T>::max() ) {
            return false;
        }
        v = math::numeric_cast<T>(uv);
        return true;
    }

    template < typename T >
    std::enable_if_t<
        std::is_floating_point_v<T>, bool>
    try_parse_value(const rapidjson::Value& root, T& v) noexcept {
        if ( !root.IsNumber() ) {
            return false;
        }
        v = math::numeric_cast<T>(root.GetFloat());
        return true;
    }

    template < typename T, typename Tag >
    std::enable_if_t<
        std::is_arithmetic_v<T>, bool>
    try_parse_value(const rapidjson::Value& root, unit<T, Tag>& v) noexcept {
        return try_parse_value(root, v.value);
    }
}

namespace e2d::json_utils
{
    template < typename T >
    bool try_parse_value(const rapidjson::Value& root, vector<T>& v) {
        if ( !root.IsArray() ) {
            return false;
        }
        vector<T> tv(root.Size());
        for ( rapidjson::SizeType i = 0; i < root.Size(); ++i ) {
            if ( !try_parse_value(root[i], tv[i]) ) {
                return false;
            }
        }
        v = std::move(tv);
        return true;
    }

    template < typename T >
    bool try_parse_value(const rapidjson::Value& root, hash_set<T>& v) {
        vector<T> tv;
        if ( !try_parse_value(root, tv) ) {
            return false;
        }
        v = hash_set<T>(tv.cbegin(), tv.cend());
        return true;
    }

    template < typename T >
    bool try_parse_value(const rapidjson::Value& root, hash_multiset<T>& v) {
        vector<T> tv;
        if ( !try_parse_value(root, tv) ) {
            return false;
        }
        v = hash_multiset<T>(tv.cbegin(), tv.cend());
        return true;
    }

    template < typename T >
    bool try_parse_value(const rapidjson::Value& root, flat_set<T>& v) {
        vector<T> tv;
        if ( !try_parse_value(root, tv) ) {
            return false;
        }
        v = flat_set<T>(tv.cbegin(), tv.cend());
        return true;
    }

    template < typename T >
    bool try_parse_value(const rapidjson::Value& root, flat_multiset<T>& v) {
        vector<T> tv;
        if ( !try_parse_value(root, tv) ) {
            return false;
        }
        v = flat_multiset<T>(tv.cbegin(), tv.cend());
        return true;
    }
}

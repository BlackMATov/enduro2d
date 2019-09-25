/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_base.hpp"

#include <flat.hpp/flat_set.hpp>
#include <flat.hpp/flat_map.hpp>
#include <flat.hpp/flat_multiset.hpp>
#include <flat.hpp/flat_multimap.hpp>

namespace e2d
{
    using f32 = float;
    using f64 = double;

    using i8  = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using u8  = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;
}

namespace e2d
{
    class exception
    : public std::exception {};

    template < typename Value
             , typename Allocator = std::allocator<Value> >
    using vector = std::vector<Value, Allocator>;

    template < typename Key
             , typename Hash = std::hash<Key>
             , typename Pred = std::equal_to<>
             , typename Allocator = std::allocator<Key> >
    using hash_set = std::unordered_set<Key, Hash, Pred, Allocator>;

    template < typename Key
             , typename Hash = std::hash<Key>
             , typename Pred = std::equal_to<>
             , typename Allocator = std::allocator<Key> >
    using hash_multiset = std::unordered_multiset<Key, Hash, Pred, Allocator>;

    template < typename Key
             , typename Value
             , typename Hash = std::hash<Key>
             , typename Pred = std::equal_to<>
             , typename Allocator = std::allocator<std::pair<const Key, Value>> >
    using hash_map = std::unordered_map<Key, Value, Hash, Pred, Allocator>;

    template < typename Key
             , typename Value
             , typename Hash = std::hash<Key>
             , typename Pred = std::equal_to<>
             , typename Allocator = std::allocator<std::pair<const Key, Value>> >
    using hash_multimap = std::unordered_multimap<Key, Value, Hash, Pred, Allocator>;

    template < typename Char
             , typename Traits = std::char_traits<Char>
             , typename Allocator = std::allocator<Char> >
    using basic_string = std::basic_string<Char, Traits, Allocator>;

    template < typename Char
             , typename Traits = std::char_traits<Char> >
    using basic_string_view = std::basic_string_view<Char, Traits>;
}

namespace e2d
{
    template < typename Key
             , typename Compare = std::less<>
             , typename Container = vector<Key> >
    using flat_set = flat_hpp::flat_set<Key, Compare, Container>;

    template < typename Key
             , typename Compare = std::less<>
             , typename Container = vector<Key> >
    using flat_multiset = flat_hpp::flat_multiset<Key, Compare, Container>;

    template < typename Key
             , typename Value
             , typename Compare = std::less<>
             , typename Container = vector<std::pair<Key, Value>> >
    using flat_map = flat_hpp::flat_map<Key, Value, Compare, Container>;

    template < typename Key
             , typename Value
             , typename Compare = std::less<>
             , typename Container = vector<std::pair<Key, Value>> >
    using flat_multimap = flat_hpp::flat_multimap<Key, Value, Compare, Container>;
}

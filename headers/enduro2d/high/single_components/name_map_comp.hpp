/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../components/name_comp.hpp"
#include "../gobject.hpp"

namespace e2d
{
    class name_map_comp final {
    public:
        name_map_comp() = default;

        gobject_iptr find(str_hash name) const noexcept;
        vector<gobject_iptr> find_all(str_hash name) const noexcept;

        void insert(str_hash name, const gobject_iptr& go);
    private:
        flat_multimap<str_hash, gobject_iptr> map_;
    };
}

namespace e2d
{
    inline gobject_iptr name_map_comp::find(str_hash name) const noexcept {
        auto i = map_.find(name);
        return i != map_.end()
            ? i->second
            : nullptr;
    }
        
    inline vector<gobject_iptr> name_map_comp::find_all(str_hash name) const noexcept {
        vector<gobject_iptr> result;
        auto i = map_.find(name);
        for (; i != map_.end() && i->first == name; ++i ) {
            result.push_back(i->second);
        }
        return result;
    }

    inline void name_map_comp::insert(str_hash name, const gobject_iptr& go) {
        map_.insert({name, go});
    }
}

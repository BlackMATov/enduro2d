/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "input_event.hpp"
#include "../factory.hpp"

namespace e2d
{
    class touchable final {
    public:
        class capture final {
        public:
            capture(u32 depth, bool stop, const input_event::data_ptr& ev_data);
        
            bool stop_propagation() const noexcept;
            u32 depth() const noexcept;
            const input_event::data_ptr& data() const noexcept;
        private:
            u32 depth_ = 0;
            bool stop_propagation_ = false;
            input_event::data_ptr ev_data_ = 0;
        };
    public:
        touchable() = default;
        touchable(bool stop);

        touchable& stop_propagation(bool value) noexcept;
        bool stop_propagation() const noexcept;
    private:
        bool stop_propagation_ = false;
    };
    
    template <>
    class factory_loader<touchable> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            touchable& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

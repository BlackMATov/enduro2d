/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "input_event.hpp"

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

        void depth(u32 value) noexcept;

        u32 depth() const noexcept;
        bool stop_propagation() const noexcept;
    private:
        u32 depth_ = 0;
        bool stop_propagation_ = false;
    };
}

namespace e2d
{
    inline touchable::touchable(bool stop)
    : stop_propagation_(stop) {}

    inline void touchable::depth(u32 value) noexcept {
        depth_ = value;
    }

    inline u32 touchable::depth() const noexcept {
        return depth_;
    }

    inline bool touchable::stop_propagation() const noexcept {
        return stop_propagation_;
    }

    inline touchable::capture::capture(u32 depth, bool stop, const input_event::data_ptr& ev_data)
    : depth_(depth)
    , stop_propagation_(stop)
    , ev_data_(ev_data) {}
        
    inline bool touchable::capture::stop_propagation() const noexcept {
        return stop_propagation_;
    }

    inline u32 touchable::capture::depth() const noexcept {
        return depth_;
    }

    inline const input_event::data_ptr& touchable::capture::data() const noexcept {
        return ev_data_;
    }
}

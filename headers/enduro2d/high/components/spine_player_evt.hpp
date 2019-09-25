/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

#include "../factory.hpp"

namespace e2d
{
    class spine_player_evt final {
    public:
        class custom_evt final {
        public:
            custom_evt(str_hash name)
            : name_(name) {}

            custom_evt& int_value(i32 value) noexcept {
                int_value_ = value;
                return *this;
            }

            custom_evt& float_value(f32 value) noexcept {
                float_value_ = value;
                return *this;
            }

            custom_evt& string_value(str value) noexcept {
                string_value_ = std::move(value);
                return *this;
            }

            str_hash name() const noexcept { return name_; }
            i32 int_value() const noexcept { return int_value_; }
            f32 float_value() const noexcept { return float_value_; }
            const str& string_value() const noexcept { return string_value_; }
        private:
            str_hash name_;
            i32 int_value_{0};
            f32 float_value_{0.f};
            str string_value_;
        };

        class end_evt final {
        public:
            end_evt(str message)
            : message_(std::move(message)) {}

            const str& message() const noexcept { return message_; }
        private:
            str message_;
        };

        class complete_evt final {
        public:
            complete_evt(str message)
            : message_(std::move(message)) {}

            const str& message() const noexcept { return message_; }
        private:
            str message_;
        };

        using event = std::variant<
            custom_evt,
            end_evt,
            complete_evt>;
    public:
        spine_player_evt() = default;

        spine_player_evt& add_event(event evt) {
            events_.push_back(std::move(evt));
            return *this;
        }

        spine_player_evt& clear_events() noexcept {
            events_.clear();
            return *this;
        }

        const vector<event>& events() const noexcept {
            return events_;
        }
    private:
        vector<event> events_;
    };

    template <>
    class factory_loader<spine_player_evt> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            spine_player_evt& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

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
    class spine_player_cmd final {
    public:
        class clear_track_cmd final {
        public:
            clear_track_cmd(u32 track)
            : track_(track) {}

            u32 track() const noexcept { return track_; }
        private:
            u32 track_{0u};
        };

        class set_anim_cmd final {
        public:
            set_anim_cmd(u32 track, str name)
            : track_(track)
            , name_(std::move(name)) {}

            set_anim_cmd& loop(bool value) noexcept {
                loop_ = value;
                return *this;
            }

            set_anim_cmd& end_message(str value) noexcept {
                end_message_ = std::move(value);
                return *this;
            }

            set_anim_cmd& complete_message(str value) noexcept {
                complete_message_ = std::move(value);
                return *this;
            }

            u32 track() const noexcept { return track_; }
            const str& name() const noexcept { return name_; }
            bool loop() const noexcept { return loop_; }
            const str& end_message() const noexcept { return complete_message_; }
            const str& complete_message() const noexcept { return complete_message_; }
        private:
            u32 track_{0u};
            str name_;
            bool loop_{false};
            str end_message_;
            str complete_message_;
        };

        class add_anim_cmd final {
        public:
            add_anim_cmd(u32 track, str name)
            : track_(track)
            , name_(std::move(name)) {}

            add_anim_cmd& loop(bool value) noexcept {
                loop_ = value;
                return *this;
            }

            add_anim_cmd& delay(secf value) noexcept {
                delay_ = value;
                return *this;
            }

            add_anim_cmd& end_message(str value) noexcept {
                end_message_ = std::move(value);
                return *this;
            }

            add_anim_cmd& complete_message(str value) noexcept {
                complete_message_ = std::move(value);
                return *this;
            }

            u32 track() const noexcept { return track_; }
            const str& name() const noexcept { return name_; }
            bool loop() const noexcept { return loop_; }
            secf delay() const noexcept { return delay_; }
            const str& end_message() const noexcept { return end_message_; }
            const str& complete_message() const noexcept { return complete_message_; }
        private:
            u32 track_{0u};
            str name_;
            bool loop_{false};
            secf delay_{0.f};
            str end_message_;
            str complete_message_;
        };

        class set_empty_anim_cmd final {
        public:
            set_empty_anim_cmd(u32 track)
            : track_(track) {}

            set_empty_anim_cmd& mix_duration(secf value) noexcept {
                mix_duration_ = value;
                return *this;
            }

            set_empty_anim_cmd& end_message(str value) noexcept {
                end_message_ = std::move(value);
                return *this;
            }

            set_empty_anim_cmd& complete_message(str value) noexcept {
                complete_message_ = std::move(value);
                return *this;
            }

            u32 track() const noexcept { return track_; }
            secf mix_duration() const noexcept { return mix_duration_; }
            const str& end_message() const noexcept { return end_message_; }
            const str& complete_message() const noexcept { return complete_message_; }
        private:
            u32 track_{0u};
            secf mix_duration_{0.f};
            str end_message_;
            str complete_message_;
        };

        class add_empty_anim_cmd final {
        public:
            add_empty_anim_cmd(u32 track)
            : track_(track) {}

            add_empty_anim_cmd& delay(secf value) noexcept {
                delay_ = value;
                return *this;
            }

            add_empty_anim_cmd& mix_duration(secf value) noexcept {
                mix_duration_ = value;
                return *this;
            }

            add_empty_anim_cmd& end_message(str value) noexcept {
                end_message_ = std::move(value);
                return *this;
            }

            add_empty_anim_cmd& complete_message(str value) noexcept {
                complete_message_ = std::move(value);
                return *this;
            }

            u32 track() const noexcept { return track_; }
            secf delay() const noexcept { return delay_; }
            secf mix_duration() const noexcept { return mix_duration_; }
            const str& end_message() const noexcept { return end_message_; }
            const str& complete_message() const noexcept { return complete_message_; }
        private:
            u32 track_{0u};
            secf delay_{0.f};
            secf mix_duration_{0.f};
            str end_message_;
            str complete_message_;
        };

        using command = std::variant<
            clear_track_cmd,
            set_anim_cmd,
            add_anim_cmd,
            set_empty_anim_cmd,
            add_empty_anim_cmd>;
    public:
        spine_player_cmd() = default;

        spine_player_cmd& add_command(command cmd) {
            commands_.push_back(std::move(cmd));
            return *this;
        }

        spine_player_cmd& clear_commands() noexcept {
            commands_.clear();
            return *this;
        }

        const vector<command>& commands() const noexcept {
            return commands_;
        }
    private:
        vector<command> commands_;
    };

    template <>
    class factory_loader<spine_player_cmd> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            spine_player_cmd& component,
            const fill_context& ctx) const;

        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

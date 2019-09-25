/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/spine_systems.hpp>

#include <enduro2d/high/components/spine_player.hpp>
#include <enduro2d/high/components/spine_player_cmd.hpp>
#include <enduro2d/high/components/spine_player_evt.hpp>

#include <spine/spine.h>

namespace
{
    using namespace e2d;

    struct entry_internal_state {
        ecs::entity target;
        str end_message;
        str complete_message;

        entry_internal_state(ecs::entity target, str end_msg, str complete_msg)
        : target(std::move(target))
        , end_message(std::move(end_msg))
        , complete_message(std::move(complete_msg)) {}
    };

    void entry_listener(
        spAnimationState*,
        spEventType type,
        spTrackEntry* entry,
        spEvent* event)
    {
        if ( !entry->userData ) {
            return;
        }

        entry_internal_state& entry_state =
            *static_cast<entry_internal_state*>(entry->userData);

        if ( type == SP_ANIMATION_DISPOSE || !entry_state.target.valid() ) {
            std::unique_ptr<entry_internal_state> entry_internal(&entry_state);
            entry->userData = nullptr;
            return;
        }

        if ( type == SP_ANIMATION_EVENT ) {
            if ( !event || !event->data ) {
                return;
            }
            entry_state.target.ensure_component<spine_player_evt>()
                .add_event(spine_player_evt::custom_evt(event->data->name ? event->data->name : "")
                    .int_value(event->intValue)
                    .float_value(event->floatValue)
                    .string_value(event->stringValue ? event->stringValue : ""));
        } else if ( type == SP_ANIMATION_END ) {
            if ( entry_state.end_message.empty() ) {
                return;
            }
            entry_state.target.ensure_component<spine_player_evt>()
                .add_event(spine_player_evt::end_evt(entry_state.end_message));
        } else if ( type == SP_ANIMATION_COMPLETE ) {
            if ( entry_state.complete_message.empty() ) {
                return;
            }
            entry_state.target.ensure_component<spine_player_evt>()
                .add_event(spine_player_evt::complete_evt(entry_state.complete_message));
        }
    }

    class command_visitor final : private noncopyable {
    public:
        command_visitor(
            ecs::entity target,
            spSkeletonData& skeleton_data,
            spAnimationState& animation_state)
        : target_(std::move(target))
        , skeleton_data_(skeleton_data)
        , animation_state_(animation_state) {}

        void operator()(const spine_player_cmd::clear_track_cmd& cmd) const noexcept {
            spAnimationState_clearTrack(
                &animation_state_,
                math::numeric_cast<int>(cmd.track()));
        }

        void operator()(const spine_player_cmd::set_anim_cmd& cmd) const noexcept {
            spAnimation* animation = spSkeletonData_findAnimation(&skeleton_data_, cmd.name().c_str());
            if ( !animation ) {
                the<debug>().error("SPINE_POST_SYSTEM: animation '%0' is not found", cmd.name());
                return;
            }

            auto entry_state = std::make_unique<entry_internal_state>(
                target_,
                cmd.end_message(),
                cmd.complete_message());

            spTrackEntry* entry = spAnimationState_setAnimation(
                &animation_state_,
                math::numeric_cast<int>(cmd.track()),
                animation,
                cmd.loop() ? 1 : 0);

            entry->listener = &entry_listener;
            entry->userData = entry_state.release();
        }

        void operator()(const spine_player_cmd::add_anim_cmd& cmd) const noexcept {
            spAnimation* animation = spSkeletonData_findAnimation(&skeleton_data_, cmd.name().c_str());
            if ( !animation ) {
                the<debug>().error("SPINE_POST_SYSTEM: animation '%0' is not found", cmd.name());
                return;
            }

            auto entry_state = std::make_unique<entry_internal_state>(
                target_,
                cmd.end_message(),
                cmd.complete_message());

            spTrackEntry* entry = spAnimationState_addAnimation(
                &animation_state_,
                math::numeric_cast<int>(cmd.track()),
                animation,
                cmd.loop() ? 1 : 0,
                cmd.delay().value);

            entry->listener = &entry_listener;
            entry->userData = entry_state.release();
        }

        void operator()(const spine_player_cmd::set_empty_anim_cmd& cmd) const noexcept {
            auto entry_state = std::make_unique<entry_internal_state>(
                target_,
                cmd.end_message(),
                cmd.complete_message());

            spTrackEntry* entry = spAnimationState_setEmptyAnimation(
                &animation_state_,
                math::numeric_cast<int>(cmd.track()),
                cmd.mix_duration().value);

            entry->listener = &entry_listener;
            entry->userData = entry_state.release();
        }

        void operator()(const spine_player_cmd::add_empty_anim_cmd& cmd) const noexcept {
            auto entry_state = std::make_unique<entry_internal_state>(
                target_,
                cmd.end_message(),
                cmd.complete_message());

            spTrackEntry* entry = spAnimationState_addEmptyAnimation(
                &animation_state_,
                math::numeric_cast<int>(cmd.track()),
                cmd.mix_duration().value,
                cmd.delay().value);

            entry->listener = &entry_listener;
            entry->userData = entry_state.release();
        }
    private:
        ecs::entity target_;
        spSkeletonData& skeleton_data_;
        spAnimationState& animation_state_;
    };
}

namespace e2d
{
    //
    // internal_state
    //

    class spine_post_system::internal_state final {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner) {
            process_commands(owner);
            clear_commands(owner);
            clear_events(owner);
        }
    private:
        void process_commands(ecs::registry& owner) {
            owner.for_joined_components<spine_player_cmd, spine_player>([](
                ecs::entity e,
                const spine_player_cmd& pc,
                spine_player& p)
            {
                spSkeleton* skeleton = p.skeleton().get();
                spAnimationState* animation_state = p.animation().get();

                if ( !skeleton || !skeleton->data || !animation_state ) {
                    return;
                }

                command_visitor v(e, *skeleton->data, *animation_state);
                for ( const auto& cmd : pc.commands() ) {
                    std::visit(v, cmd);
                }
            });
        }

        void clear_commands(ecs::registry& owner) {
            owner.for_each_component<spine_player_cmd>([
            ](const ecs::const_entity&, spine_player_cmd& pc) {
                pc.clear_commands();
            });
        }

        void clear_events(ecs::registry& owner) {
            owner.for_each_component<spine_player_evt>([
            ](const ecs::const_entity&, spine_player_evt& pe) {
                pe.clear_events();
            });
        }
    };

    //
    // spine_post_system
    //

    spine_post_system::spine_post_system()
    : state_(new internal_state()) {}

    spine_post_system::~spine_post_system() noexcept {
        spAnimationState_disposeStatics();
    }

    void spine_post_system::process(ecs::registry& owner) {
        state_->process(owner);
    }
}

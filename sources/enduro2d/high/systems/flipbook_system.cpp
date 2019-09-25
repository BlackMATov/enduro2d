/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/systems/flipbook_system.hpp>

#include <enduro2d/high/components/flipbook_player.hpp>
#include <enduro2d/high/components/sprite_renderer.hpp>

namespace
{
    using namespace e2d;

    void update_flipbook_timers(f32 dt, ecs::registry& owner) {
        owner.for_joined_components<flipbook_player>([dt](
            const ecs::const_entity&,
            flipbook_player& fp)
        {
            if ( fp.speed() <= 0.f || fp.stopped() ) {
                return;
            }
            const flipbook_asset::ptr& flipbook_res = fp.flipbook();
            if ( !flipbook_res ) {
                return;
            }
            const flipbook& flipbook = flipbook_res->content();
            const flipbook::sequence* sequence = flipbook.find_sequence(fp.sequence());
            if ( !sequence || sequence->frames.empty() ) {
                return;
            }
            fp.time(fp.time() + dt * fp.speed());
            if ( sequence->fps > 0.f ) {
                const f32 loop_time = sequence->frames.size() / sequence->fps;
                if ( fp.time() >= loop_time ) {
                    if ( fp.looped() ) {
                        fp.time(math::mod(fp.time(), loop_time));
                    } else {
                        fp.stop(loop_time);
                    }
                }
            }
        });
    }

    void update_flipbook_sprites(ecs::registry& owner) {
        owner.for_joined_components<flipbook_player, sprite_renderer>([](
            const ecs::const_entity&,
            const flipbook_player& fp,
            sprite_renderer& sr)
        {
            const flipbook_asset::ptr& flipbook_res = fp.flipbook();
            if ( !flipbook_res ) {
                sr.sprite(nullptr);
                return;
            }
            const flipbook& flipbook = flipbook_res->content();
            const flipbook::sequence* sequence = flipbook.find_sequence(fp.sequence());
            if ( !sequence || sequence->frames.empty() ) {
                sr.sprite(nullptr);
                return;
            }
            const std::size_t frame_index = math::clamp<std::size_t>(
                math::numeric_cast<std::size_t>(fp.time() * sequence->fps),
                0u,
                sequence->frames.size() - 1u);
            const flipbook::frame* frame = flipbook.find_frame(sequence->frames[frame_index]);
            sr.sprite(frame ? frame->sprite : nullptr);
        });
    }
}

namespace e2d
{
    //
    // flipbook_system::internal_state
    //

    class flipbook_system::internal_state final : private noncopyable {
    public:
        internal_state() = default;
        ~internal_state() noexcept = default;

        void process(ecs::registry& owner) {
            update_flipbook_timers(the<engine>().delta_time(), owner);
            update_flipbook_sprites(owner);
        }
    };

    //
    // flipbook_system
    //

    flipbook_system::flipbook_system()
    : state_(new internal_state()) {}
    flipbook_system::~flipbook_system() noexcept = default;

    void flipbook_system::process(ecs::registry& owner) {
        state_->process(owner);
    }
}

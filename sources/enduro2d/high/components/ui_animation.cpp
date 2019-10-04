/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/high/components/ui_animation.hpp>
#include <enduro2d/high/components/actor.hpp>
#include <enduro2d/high/components/ui_layout.hpp>

namespace
{
    using namespace e2d;

    class parallel_anim final : public ui_animation::anim_i {
    public:
        parallel_anim(
            ui_animation::anim_builder& b,
            vector<std::unique_ptr<anim_i>>&& anims)
        : anim_i(b)
        , animations_(std::move(anims)) {}

        bool update_(secf time, ecs::entity& e) override {
            for ( auto i = animations_.begin(); i != animations_.end(); ) {
                if ( (*i)->update(time, e) ) {
                    ++i;
                } else {
                    i = animations_.erase(i);
                }
            }
            return !animations_.empty();
        }
    private:
        vector<std::unique_ptr<anim_i>> animations_;
    };
    
    class sequential_anim final : public ui_animation::anim_i {
    public:
        sequential_anim(
            ui_animation::anim_builder& b,
            vector<std::unique_ptr<anim_i>>&& anims)
        : anim_i(b)
        , animations_(std::move(anims)) {}
        
        bool update_(secf time, ecs::entity& e) override {
            if ( !animations_.empty() ) {
                if ( animations_.front()->update(time, e) ) {
                    animations_.erase(animations_.begin());
                }
                return true;
            }
            return false;
        }
    private:
        vector<std::unique_ptr<anim_i>> animations_;
    };

    template < typename UpdateFn, typename StartFn, typename Data >
    class value_anim : public ui_animation::anim_i {
    public:
        template < typename T >
        value_anim(
            ui_animation::value_anim_builder<T>& b,
            Data&& data,
            UpdateFn&& update_fn,
            StartFn&& start_fn)
        : anim_i(b)
        , data_(data)
        , update_fn_(update_fn)
        , start_fn_(start_fn)
        , duration_(b.duration())
        , easing_(b.easing()) {}
        
        bool update_(secf time, ecs::entity& e) override {
            if ( time < duration_ ) {
                f32 f = 1.0f - (time.value / duration_.value);
                f = easing_(f);
                update_fn_(data_, f, e);
                return true;
            }
            return false;
        }

        bool start_(ecs::entity& e) override {
            return start_fn_(data_, e);
        }
    private:
        Data data_;
        UpdateFn update_fn_;
        StartFn start_fn_;
        const secf duration_;
        const ui_animation::easing_fn easing_;
    };
    
    template < typename T, typename UpdateFn, typename StartFn, typename Data >
    auto make_value_anim(
        ui_animation::value_anim_builder<T>& b,
        Data&& data,
        UpdateFn&& update_fn,
        StartFn&& start_fn)
    {
        using value_anim_t = value_anim<
            std::remove_reference_t<UpdateFn>,
            std::remove_reference_t<StartFn>,
            std::remove_reference_t<Data>>;

        return std::unique_ptr<ui_animation::anim_i>(new value_anim(
            b,
            std::forward<Data>(data),
            std::forward<UpdateFn>(update_fn),
            std::forward<StartFn>(start_fn)));
    }
    
    template < typename D, typename F >
    auto start_fn_adaptor(F&& fn) {
        // TODO
        return std::forward<F>(fn);
    }

    template < typename D, typename F >
    auto update_fn_adaptor(F&& fn) {
        using fi = func_info<F>;
        static_assert(std::tuple_size_v<fi::args> < 3);

        if constexpr( std::tuple_size_v<fi::args> == 0 ) {
            return
                [fn = std::forward<F>(fn)](const D&, f32, ecs::entity&) mutable {
                    fn();
                };
        }
        if constexpr( std::tuple_size_v<fi::args> == 1 ) {
            static_assert(std::is_same_v<typename std::tuple_element<0, fi::args>::type, f32>);
            return
                [fn = std::forward<F>(fn)](const D&, f32 f, ecs::entity&) mutable {
                    fn(f);
                };
        }
        if constexpr( std::tuple_size_v<fi::args> == 2 ) {
            using second_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<1, fi::args>::type>>;
            static_assert(std::is_same_v<typename std::tuple_element<0, fi::args>::type, f32>);

            if constexpr( std::is_same_v<second_t, ecs::entity> ) {
                return std::forward<F>(fn);
            } else {
                // second argument is component type
                return
                    [fn = std::forward<F>(fn)](const D&, f32 f, ecs::entity& e) mutable {
                        auto& comp = e.get_component<second_t>();
                        fn(d, f, comp);
                    };
            }
        }
    }

    template < typename T, typename UpdateFn, typename StartFn, typename Data >
    auto make_value_anim_adaptor(
        ui_animation::value_anim_builder<T>& b,
        Data&& data,
        UpdateFn&& update_fn,
        StartFn&& start_fn)
    {
        using data_t = std::remove_reference_t<Data>;
        return make_value_anim(
            b,
            std::forward<Data>(data),
            update_fn_adaptor<data_t>(std::forward<UpdateFn>(update_fn)),
            start_fn_adaptor<data_t>(std::forward<StartFn>(start_fn)));
    }

    /*
    class scale_anim final : public value_anim {
    public:
        scale_anim(
            ui_animation::anim_builder& b,
            secf duration,
            easing_fn easing,
            std::optional<v3f> from_scale,
            std::optional<v3f> to_scale)
        : value_anim(b)
        , from_scale_(from_scale)
        , to_scale_(to_scale) {}
        
        bool update_(secf time, ecs::entity& e) override {
            auto n = e.get_component<actor>().node();
            n->scale(math::lerp(*from_scale_, *to_scale_, v3f(f)));
        }

        bool start_(ecs::entity& e) override {
            auto* act = e.find_component<actor>();
            if ( act && act->node() ) {
                if ( !from_scale_.has_value() ) {
                    from_scale_ = act->node()->scale();
                }
                if ( !to_scale_.has_value() ) {
                    to_scale_ = act->node()->scale();
                }
                return true;
            }
            return false;
        }
    private:
        std::optional<v3f> from_scale_;
        std::optional<v3f> to_scale_;
    };

    class move_anim final : public ui_animation::anim_i {
    public:
        scale_anim(
            ui_animation::anim_builder& b,
            std::optional<v3f> from_scale,
            std::optional<v3f> to_scale)
        : anim_i(b)
        , from_pos_(from_pos)
        , to_pos_(to_pos) {}
        
        bool update_(secf time, ecs::entity& e) override {
            auto n = e.get_component<actor>().node();
            n->translation(math::lerp(*start_pos_, *end_pos_, v3f(f)));
        }

        bool start_(ecs::entity& e) override {
            auto* act = e.find_component<actor>();
            if ( act && act->node() ) {
                if ( !start_pos_.has_value() ) {
                    start_pos_ = act->node()->translation();
                }
                if ( !end_pos_.has_value() ) {
                    end_pos_ = act->node()->translation();
                }
                return true;
            }
            return false;
        }
    private:
        std::optional<v3f> from_pos_;
        std::optional<v3f> to_pos_;
    };*/
}

namespace e2d
{
    //
    // anim_i
    //

    ui_animation::anim_i::anim_i(anim_builder& b)
    : on_start_(std::move(b.on_start_))
    , on_complete_(std::move(b.on_complete_))
    , on_step_complete_(std::move(b.on_step_complete_))
    , loops_(b.loops_)
    , delay_(b.delay_) {}

    bool ui_animation::anim_i::update(secf t, ecs::entity& e) {
        if ( canceled_ ) {
            return false;
        }
        if ( started_ ) {
            t -= start_time_;
            if ( update_(t, e) ) {
                // on update
            } else {
                if ( --loops_ == 0 ) {
                    complete_(e);
                    safe_call(on_complete_, e);
                    return false;
                } else {
                    end_(t, e);
                    safe_call(on_step_complete_, e);
                }
            }
        } else {
            delay_ -= t;
            if ( delay_ > secf(0.f) ) {
                return true;
            }
            start_time_ = t - delay_;
            if ( !start_(e) ) {
                return false;
            }
            started_ = true;
            safe_call(on_start_, e);
        }
        return true;
    }

    void ui_animation::anim_i::cancel() {
        canceled_ = true;
    }

    //
    // parallel
    //

    std::unique_ptr<ui_animation::anim_i> ui_animation::parallel::build() && {
        return std::make_unique<parallel_anim>(
            *this, std::move(animations_));
    }

    //
    // sequential
    //

    std::unique_ptr<ui_animation::anim_i> ui_animation::sequential::build() && {
        return std::make_unique<sequential_anim>(
            *this, std::move(animations_));
    }

    //
    // ui_animation
    //

    ui_animation::ui_animation(const ui_animation&) {}

    ui_animation::anim_i* ui_animation::animation() const noexcept {
        return anim_.get();
    }

    ui_animation& ui_animation::reset_animation() noexcept {
        anim_.reset(nullptr);
        return *this;
    }

    //
    // scale
    //

    ui_animation::scale&& ui_animation::scale::from(f32 value) && noexcept {
        from_scale_ = v3f(value);
        return std::move(*this);
    }

    ui_animation::scale&& ui_animation::scale::from(const v3f& value) && noexcept {
        from_scale_ = value;
        return std::move(*this);
    }
        
    ui_animation::scale&& ui_animation::scale::to(f32 value) && noexcept {
        to_scale_ = v3f(value);
        return std::move(*this);
    }

    ui_animation::scale&& ui_animation::scale::to(const v3f& value) && noexcept {
        to_scale_ = value;
        return std::move(*this);
    }
    
    std::unique_ptr<ui_animation::anim_i> ui_animation::scale::build() && {
        using data_t = std::pair<std::optional<v3f>, std::optional<v3f>>;
        return make_value_anim(
            *this,
            data_t(from_scale_, to_scale_),
            [](data_t& d, f32 f, actor& act) {
                act.node()->scale(math::lerp(*d.first, *d.second, v3f(f)));
            },
            [](data_t& d, ecs::entity& e) {
                return true;
            });
    }


    //
    // move
    //

    ui_animation::move&& ui_animation::move::from(const v3f& value) && noexcept {
        from_pos_ = value;
        return std::move(*this);
    }

    ui_animation::move&& ui_animation::move::to(const v3f& value) && noexcept {
        to_pos_ = value;
        return std::move(*this);
    }
    
    std::unique_ptr<ui_animation::anim_i> ui_animation::move::build() && {
        using data_t = std::pair<std::optional<v3f>, std::optional<v3f>>;
        return make_value_anim(
            *this,
            data_t(from_pos_, to_pos_),
            [](data_t& d, f32 f, ecs::entity& e) {
            },
            [](data_t& d, ecs::entity& e) {
                return true;
            });
    }
}

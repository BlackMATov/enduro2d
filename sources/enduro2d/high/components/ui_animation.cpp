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

    class parallel_anim final : public ui_animation::abstract_anim {
    public:
        parallel_anim(const parallel_anim& other)
        : abstract_anim(other) {
            animations_.resize(other.animations_.size());
            for ( size_t i = 0; i < animations_.size(); ++i ) {
                animations_[i] = other.animations_[i]->clone();
            }
            complete_animations_.resize(animations_.size(), false);
        }

        parallel_anim(
            ui_animation::anim_builder& b,
            vector<ui_animation::abstract_anim_uptr>&& anims)
        : abstract_anim(b)
        , animations_(std::move(anims)) {
            complete_animations_.resize(animations_.size(), false);
        }

        bool update_(secf time, secf delta, ecs::entity& e) override {
            bool updated = false;
            for ( size_t i = 0; i < animations_.size(); ++i ) {
                if ( complete_animations_[i] ) {
                    continue;
                }
                complete_animations_[i] = !animations_[i]->update(time, delta, e);
                updated = true;
            }
            return updated;
        }

        bool start_(ecs::entity&) {
            for ( size_t i = 0; i < complete_animations_.size(); ++i ) {
                complete_animations_[i] = false;
            }
            return true;
        }

        void end_(secf, ecs::entity&) override {
            E2D_ASSERT(!inversed());
            for ( size_t i = 0; i < complete_animations_.size(); ++i ) {
                complete_animations_[i] = false;
            }
        }

        ui_animation::abstract_anim_uptr clone() const override {
            return std::make_unique<parallel_anim>(*this);
        }
    private:
        vector<ui_animation::abstract_anim_uptr> animations_;
        vector<bool> complete_animations_;
    };
    
    class sequential_anim final : public ui_animation::abstract_anim {
    public:
        sequential_anim() = default;

        sequential_anim(const sequential_anim& other)
        : abstract_anim(other) {
            animations_.resize(other.animations_.size());
            for ( size_t i = 0; i < animations_.size(); ++i ) {
                animations_[i] = other.animations_[i]->clone();
            }
        }

        sequential_anim(
            ui_animation::anim_builder& b,
            vector<ui_animation::abstract_anim_uptr>&& anims)
        : abstract_anim(b)
        , animations_(std::move(anims)) {
            E2D_ASSERT(!animations_.empty());
        }
        
        bool update_(secf time, secf delta, ecs::entity& e) override {
            for (; index_ < animations_.size(); ) {
                if ( animations_[index_]->update(time, delta, e) ) {
                    return true;
                } else {
                    inversed() ? --index_ : ++index_;
                    return true;
                }
            }
            return false;
        }
        
        bool start_(ecs::entity&) {
            index_ = 0;
            return true;
        }

        void end_(secf, ecs::entity&) override {
            index_ = inversed() ? math::max<size_t>(1, animations_.size())-1 : 0;
        }
        
        ui_animation::abstract_anim_uptr clone() const override {
            return std::make_unique<sequential_anim>(*this);
        }

        void append(ui_animation::abstract_anim_uptr anim) {
            animations_.push_back(std::move(anim));
        }
    private:
        vector<ui_animation::abstract_anim_uptr> animations_;
        size_t index_ = 0;
    };

    template < typename UpdateFn, typename StartFn, typename Data >
    class property_anim : public ui_animation::abstract_anim {
    public:
        template < typename T >
        property_anim(
            ui_animation::property_anim_builder<T>& b,
            Data&& data,
            UpdateFn&& update_fn,
            StartFn&& start_fn)
        : abstract_anim(b)
        , data_(data)
        , update_fn_(update_fn)
        , start_fn_(start_fn)
        , duration_(b.duration())
        , easing_(b.easing()) {
            E2D_ASSERT(easing_);
        }
        
        bool update_(secf time, secf, ecs::entity& e) override {
            f32 f = 1.0f;
            bool result = false;
            if ( time < duration_ ) {
                f = (time.value / duration_.value);
                result = true;
            }
            f = easing_(f);
            f = inversed() ? 1.0f - f : f;
            update_fn_(data_, f, e);
            return result;
        }

        bool start_(ecs::entity& e) override {
            return start_fn_(data_, e);
        }

        ui_animation::abstract_anim_uptr clone() const override {
            // TODO: update_fn_ and start_fn_ should be immutable
            return std::make_unique<property_anim>(*this);
        }
    private:
        Data data_;
        UpdateFn update_fn_;
        StartFn start_fn_;
        const secf duration_;
        const ui_animation::easing_fn easing_;
    };
    
    template < typename T, typename UpdateFn, typename StartFn, typename Data >
    [[nodiscard]] auto make_property_anim(
        ui_animation::property_anim_builder<T>& b,
        Data&& data,
        UpdateFn&& update_fn,
        StartFn&& start_fn)
    {
        using value_anim_t = property_anim<
            std::remove_reference_t<UpdateFn>,
            std::remove_reference_t<StartFn>,
            std::remove_reference_t<Data>>;

        return ui_animation::abstract_anim_uptr(new property_anim(
            b,
            std::forward<Data>(data),
            std::forward<UpdateFn>(update_fn),
            std::forward<StartFn>(start_fn)));
    }

    template < typename D, typename F >
    [[nodiscard]] auto start_fn_adaptor(F&& fn) {
        // TODO
        return std::forward<F>(fn);
    }

    template < typename D, typename F >
    [[nodiscard]] auto default_start_fn() {
        using fi = func_info<F>;
        constexpr size_t size = std::tuple_size_v<fi::args>;
        if ( size > 0 ) {
            using last_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<size-1, fi::args>::type>>;
            if constexpr( std::is_same_v<last_t, ecs::entity> ) {
                return [](D& d, ecs::entity&) { return true; };
            } else if constexpr( std::is_same_v<last_t, actor> ) {
                return
                    [](D& d, ecs::entity& e) {
                        auto* act = e.find_component<actor>();
                        return act && act->node();
                    };
            } else {
                return
                    [](D& d, ecs::entity& e) {
                        return e.find_component<last_t>() != nullptr;
                    };
            }
        }
    }

    template < typename T >
    struct tuple_pop_front_ {};

    template < typename A0, typename... An >
    struct tuple_pop_front_<std::tuple<A0, An...>> {
        using type = std::tuple<An...>;
    };

    template < typename T >
    using tuple_pop_front = typename tuple_pop_front_<T>::type;

    template < typename... Ts >
    [[nodiscard]] auto get_components_from_tuple(ecs::entity& e, std::tuple<Ts...>*) {
        return e.get_components<std::remove_reference_t<std::remove_cv_t<Ts>>...>();
    }

    template < typename D, typename F >
    [[nodiscard]] auto update_fn_adaptor(F&& fn) {
        using fi = func_info<F>;

        if constexpr( std::tuple_size_v<fi::args> == 0 ) {
            return
                [fn = std::forward<F>(fn)](D&, f32, ecs::entity&) mutable {
                    fn();
                };
        } else if constexpr( std::tuple_size_v<fi::args> == 2 ) {
            using first_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<0, fi::args>::type>>;
            using second_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<1, fi::args>::type>>;

            if constexpr( std::is_same_v<first_t, D> && std::is_same_v<second_t, f32> ) {
                // data, factor
                return
                    [fn = std::forward<F>(fn)](D& d, f32 f, ecs::entity&) mutable {
                        fn(d, f);
                    };
            } else if constexpr( std::is_same_v<first_t, f32> && std::is_same_v<second_t, ecs::entity> ) {
                // factor, entity
                return
                    [fn = std::forward<F>(fn)](D&, f32 f, ecs::entity& e) mutable {
                        fn(f, e);
                    };
            } else if constexpr( std::is_same_v<first_t, f32> ) {
                // factor, component
                return
                    [fn = std::forward<F>(fn)](D&, f32 f, ecs::entity& e) mutable {
                        auto& c = e.get_component<second_t>();
                        fn(f, c);
                    };
            }
        } else if constexpr( std::tuple_size_v<fi::args> == 3 ) {
            using first_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<0, fi::args>::type>>;
            using second_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<1, fi::args>::type>>;
            using third_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<2, fi::args>::type>>;
            
            if constexpr( std::is_same_v<first_t, D> && std::is_same_v<second_t, f32> && std::is_same_v<third_t, ecs::entity> ) {
                // data, factor, entity
                return std::forward<F>(fn);
            }
            else if constexpr( std::is_same_v<first_t, D> && std::is_same_v<second_t, f32> ) {
                // data, factor, component
                return
                    [fn = std::forward<F>(fn)](D& d, f32 f, ecs::entity& e) mutable {
                        auto& c = e.get_component<third_t>();
                        fn(d, f, c);
                    };
            }
        } else {
            // data, factor, components
            return
                [fn = std::forward<F>(fn)](D& d, f32 f, ecs::entity& e) mutable {
                    using components_t = tuple_pop_front<tuple_pop_front<fi::args>>;
                    std::apply([&fn, &d, f](auto&&... args){
                            fn(d, f, std::forward<decltype(args)>(args)...);
                        },
                        get_components_from_tuple(e, (components_t*)nullptr));
                };
        }
    }

    template < typename T, typename UpdateFn, typename StartFn, typename Data >
    [[nodiscard]] auto make_property_anim_adaptor(
        ui_animation::property_anim_builder<T>& b,
        Data&& data,
        UpdateFn&& update_fn,
        StartFn&& start_fn)
    {
        using data_t = std::remove_reference_t<Data>;
        return make_property_anim(
            b,
            std::forward<Data>(data),
            update_fn_adaptor<data_t>(std::forward<UpdateFn>(update_fn)),
            start_fn_adaptor<data_t>(std::forward<StartFn>(start_fn)));
    }

    template < typename T, typename UpdateFn, typename Data >
    [[nodiscard]] auto make_property_anim_adaptor(
        ui_animation::property_anim_builder<T>& b,
        Data&& data,
        UpdateFn&& update_fn)
    {
        using data_t = std::remove_reference_t<Data>;
        return make_property_anim(
            b,
            std::forward<Data>(data),
            update_fn_adaptor<data_t>(std::forward<UpdateFn>(update_fn)),
            default_start_fn<data_t, UpdateFn>());
    }
}

namespace e2d
{
    //
    // abstract_anim
    //
    
    ui_animation::abstract_anim::abstract_anim()
    : repeat_inversed_(false)
    , const_delay_()
    , const_loops_(0) {}

    ui_animation::abstract_anim::abstract_anim(anim_builder& b)
    : on_start_(std::move(b.on_start_))
    , on_complete_(std::move(b.on_complete_))
    , on_step_complete_(std::move(b.on_step_complete_))
    , loops_(b.loops_)
    , delay_(b.delay_)
    , const_delay_(b.delay_)
    , const_loops_(b.loops_)
    , repeat_inversed_(b.repeat_inversed_) {}

    bool ui_animation::abstract_anim::update(secf t, secf dt, ecs::entity& e) {
        if ( started_ ) {
            secf rel = t - start_time_;
            if ( update_(rel, dt, e) ) {
                // on update
                return true;
            } else {
                constexpr auto infinite_loops = std::numeric_limits<decltype(loops_)>::max();
                if ( loops_ != infinite_loops && --loops_ < 0 ) {
                    complete_(e);
                    if ( on_complete_ ) {
                        on_complete_(e);
                    }
                    // prepare for restarting
                    delay_ = const_delay_;
                    started_ = false;
                    loops_ = 0;
                    return false;
                } else {
                    inversed_ = repeat_inversed_ ? !inversed_ : inversed_;
                    start_time_ = t;
                    end_(rel, e);
                    if ( on_step_complete_ ) {
                        on_step_complete_(e);
                    }
                    return true;
                }
            }
        } else {
            delay_ -= dt;
            if ( delay_ > secf(0.f) ) {
                return true;
            }
            start_time_ = t + delay_;
            if ( !start_(e) ) {
                return false;
            }
            started_ = true;
            if ( on_start_ ) {
                on_start_(e);
            }
            return update(t, dt, e);
        }
    }
    
    bool ui_animation::abstract_anim::inversed() const noexcept {
        return inversed_;
    }

    //
    // parallel
    //

    ui_animation::abstract_anim_uptr ui_animation::parallel::build() && {
        return std::make_unique<parallel_anim>(
            *this, std::move(animations_));
    }

    //
    // sequential
    //

    ui_animation::abstract_anim_uptr ui_animation::sequential::build() && {
        return std::make_unique<sequential_anim>(
            *this, std::move(animations_));
    }

    //
    // ui_animation
    //

    ui_animation::ui_animation(const ui_animation& other) {
        if ( other.anim_ ) {
            anim_ = other.anim_->clone();
        }
    }

    ui_animation::abstract_anim* ui_animation::animation() const noexcept {
        return anim_.get();
    }
    
    ui_animation& ui_animation::add(abstract_anim_uptr value) noexcept {
        if ( !value ) {
            E2D_ASSERT(false);
            return *this;
        }
        if ( !anim_ ) {
            anim_ = std::make_unique<sequential_anim>();
        }
        static_cast<sequential_anim*>(anim_.get())->append(std::move(value));
        return *this;
    }
    
    ui_animation& ui_animation::clear() noexcept {
        anim_.reset();
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
    
    ui_animation::abstract_anim_uptr ui_animation::scale::build() && {
        using data_t = std::pair<std::optional<v3f>, std::optional<v3f>>;
        return make_property_anim_adaptor(
            *this,
            data_t(from_scale_, to_scale_),
            [](data_t& d, f32 f, actor& act) {
                act.node()->scale(math::lerp(*d.first, *d.second, v3f(f)));
            },
            [](data_t& d, ecs::entity& e) {
                auto* act = e.find_component<actor>();
                if ( act && act->node() ) {
                    if ( !d.first.has_value() ) {
                        d.first = act->node()->scale();
                    }
                    if ( !d.second.has_value() ) {
                        d.second = act->node()->scale();
                    }
                    return true;
                }
                return false;
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
    
    ui_animation::abstract_anim_uptr ui_animation::move::build() && {
        using data_t = std::pair<std::optional<v3f>, std::optional<v3f>>;
        return make_property_anim_adaptor(
            *this,
            data_t(from_pos_, to_pos_),
            [](data_t& d, f32 f, actor& act) {
                act.node()->translation(math::lerp(*d.first, *d.second, v3f(f)));
            },
            [](data_t& d, ecs::entity& e) {
                auto* act = e.find_component<actor>();
                if ( act && act->node() ) {
                    if ( !d.first.has_value() ) {
                        d.first = act->node()->translation();
                    }
                    if ( !d.second.has_value() ) {
                        d.second = act->node()->translation();
                    }
                    return true;
                }
                return false;
            });
    }
    
    //
    // size
    //

    ui_animation::size&& ui_animation::size::from(const v2f& value) && noexcept {
        from_width_ = value.x;
        from_height_ = value.y;
        return std::move(*this);
    }

    ui_animation::size&& ui_animation::size::to(const v2f& value) && noexcept {
        to_width_ = value.x;
        to_height_ = value.y;
        return std::move(*this);
    }
        
    ui_animation::size&& ui_animation::size::from_width(f32 value) && noexcept {
        from_width_ = value;
        return std::move(*this);
    }

    ui_animation::size&& ui_animation::size::to_width(f32 value) && noexcept {
        to_width_ = value;
        return std::move(*this);
    }

    ui_animation::size&& ui_animation::size::from_height(f32 value) && noexcept {
        from_height_ = value;
        return std::move(*this);
    }

    ui_animation::size&& ui_animation::size::to_height(f32 value) && noexcept {
        to_height_ = value;
        return std::move(*this);
    }
        
    ui_animation::abstract_anim_uptr ui_animation::size::build() && {
        struct data_t {
            std::optional<f32> from_width;
            std::optional<f32> to_width;
            std::optional<f32> from_height;
            std::optional<f32> to_height;
        };
        return make_property_anim_adaptor(
            *this,
            data_t{from_width_, to_width_, from_height_, to_height_},
            [](data_t& d, f32 f, actor& act, fixed_layout& fl) {
                v2f s = fl.size();
                if ( d.from_width.has_value() & d.to_width.has_value() ) {
                    s.x = math::lerp(d.from_width.value(), d.to_width.value(), f);
                }
                if ( d.from_height.has_value() & d.to_height.has_value() ) {
                    s.y = math::lerp(d.from_height.value(), d.to_height.value(), f);
                }
                fl.size(s);
                act.node()->size(s);
            },
            [](data_t& d, ecs::entity& e) {
                auto* act = e.find_component<actor>();
                auto* fl = e.find_component<fixed_layout>();
                if ( act && act->node() && fl ) {
                    const v2f s = fl->size();
                    if ( d.from_width.has_value() || d.to_width.has_value() ) {
                        d.from_width = d.from_width.value_or(s.x);
                        d.to_width = d.to_width.value_or(s.x);
                    }
                    if ( d.from_height.has_value() || d.to_height.has_value() ) {
                        d.from_height = d.from_height.value_or(s.x);
                        d.to_height = d.to_height.value_or(s.x);
                    }
                    return true;
                }
                return false;
            });
    }
}

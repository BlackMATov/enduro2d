/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "../_high.hpp"

namespace e2d
{
    class ui_animation final {
    public:
        class cancel_tag final {};

    public:
        using easing_fn = f32 (*)(f32 t);

        class abstract_anim;
        using abstract_anim_uptr = std::unique_ptr<abstract_anim>;
        class anim_builder;
        template < typename T > class anim_builder_t;
        template < typename T > class property_anim_builder;
        class scale;
        class move;
        class size;
        class parallel;
        class sequential;
        
        template < typename F > static auto custom(F&&);
    private:
        template < typename F > class custom_impl;
        template < typename F > class custom_anim_i;
    public:
        ui_animation() = default;
        ui_animation(const ui_animation&);
        ui_animation(ui_animation&&) noexcept = default;
        ui_animation& operator=(ui_animation&&) noexcept = default;
        
        template < typename A >
        ui_animation(A&& value);

        template < typename A >
        std::enable_if_t<std::is_base_of_v<anim_builder, A>, ui_animation&> set_animation(A&& value) noexcept;
        ui_animation& set_animation(abstract_anim_uptr value) noexcept;
        abstract_anim* animation() const noexcept;
    private:
        abstract_anim_uptr anim_;
    };
}

namespace e2d
{
    //
    // ui_animation::abstract_anim
    //

    class ui_animation::abstract_anim {
    public:
        abstract_anim(anim_builder&);
        bool update(secf t, secf dt, ecs::entity& e);
        void cancel();
        [[nodiscard]] virtual abstract_anim_uptr clone() const = 0;
        bool inversed() const noexcept;
    protected:
        abstract_anim(const abstract_anim&) = default;
        virtual bool update_(secf time, secf delta, ecs::entity& e) = 0;
        virtual bool start_(ecs::entity&) { return true; }
        virtual void end_(secf, ecs::entity&) {}
        virtual void complete_(ecs::entity&) {}
    protected:
        bool inversed_ = false;
    private:
        bool started_ = false;
        bool canceled_ = false;
        const bool repeat_inversed_;
        secf delay_;
        secf start_time_;
        i32 loops_ = 0;
        const secf const_delay_;
        const i32 const_loops_;
        std::function<void(ecs::entity&)> on_start_;
        std::function<void(ecs::entity&)> on_complete_;
        std::function<void(ecs::entity&)> on_step_complete_; // when loop cycle complete
    };
    
    //
    // ui_animation::anim_builder
    //

    class ui_animation::anim_builder {
        friend class ui_animation::abstract_anim;
    protected:
        std::function<void(ecs::entity&)> on_start_;
        std::function<void(ecs::entity&)> on_complete_;
        std::function<void(ecs::entity&)> on_step_complete_; // when loop cycle complete
        i32 loops_ = 0;
        secf delay_ {0.0f};
        bool repeat_inversed_ = false;
    };

    template < typename T >
    class ui_animation::anim_builder_t : public anim_builder {
    public:
        template < typename F >
        T&& on_start(F&& fn) &&;
        
        template < typename F >
        T&& on_complete(F&& fn) &&;
            
        template < typename F >
        T&& on_step_complete(F&& fn) &&;

        T&& repeat(u32 count) && noexcept;
        T&& delay(secf value) && noexcept;
        
        T&& infinite_loops() && noexcept;
        T&& repeat_inversed() && noexcept;
    };

    //
    // ui_animation::property_anim_builder
    //

    template < typename T >
    class ui_animation::property_anim_builder : public anim_builder_t<T> {
    public:
        T&& ease(easing_fn fn) && noexcept;
        T&& duration(secf value) && noexcept;

        easing_fn easing() const noexcept;
        secf duration() const noexcept;
    protected:
        secf duration_;
        easing_fn easing_ = easing::linear;
    };

    //
    // ui_animation::parallel
    //

    class ui_animation::parallel final : public anim_builder_t<ui_animation::parallel> {
    public:
        parallel() = default;
        
        template < typename A >
        std::enable_if_t<std::is_base_of_v<anim_builder, A>, parallel&&> add(A&&) &&;
        
        [[nodiscard]] abstract_anim_uptr build() &&;
    private:
        vector<abstract_anim_uptr> animations_;
    };

    //
    // ui_animation::sequential
    //

    class ui_animation::sequential final : public anim_builder_t<ui_animation::sequential> {
    public:
        sequential() = default;
        
        template < typename A >
        std::enable_if_t<std::is_base_of_v<anim_builder, A>, sequential&&> add(A&&) &&;
        
        [[nodiscard]] abstract_anim_uptr build() &&;
    private:
        vector<abstract_anim_uptr> animations_;
    };
    
    //
    // ui_animation::custom_anim_i
    //
    
    template < typename UpdateFn >
    class ui_animation::custom_anim_i final : public abstract_anim {
    public:
        custom_anim_i(
            anim_builder& b,
            UpdateFn&& update_fn,
            secf duration,
            easing_fn easing)
        : abstract_anim(b)
        , update_fn_(std::forward<UpdateFn>(update_fn))
        , duration_(duration)
        , easing_(easing) {}

        custom_anim_i(const custom_anim_i& other)
        : abstract_anim(other)
        , update_fn_(other.update_fn_)
        , duration_(other.duration_)
        , easing_(other.easing_) {}
        
        bool update_(secf time, secf, ecs::entity& e) override {
            f32 f = 1.0f;
            bool result = false;
            if ( time < duration_ ) {
                f = (time.value / duration_.value);
                result = true;
            }
            f = easing_(f);
            f = inversed_ ? 1.0f - f : f;
            update_fn_(f, e);
            return result;
        }

        ui_animation::abstract_anim_uptr clone() const override {
            return std::make_unique<custom_anim_i<UpdateFn>>(*this);
        }
    private:
        UpdateFn update_fn_;
        const secf duration_ {0.0f};
        const easing_fn easing_;
    };
    
    //
    // ui_animation::custom_impl
    //

    template < typename UpdateFn >
    class ui_animation::custom_impl final : public property_anim_builder<ui_animation::custom_impl<UpdateFn>>
    {
        template < typename T >
        friend auto ui_animation::custom(T&& fn);
    public:
        custom_impl(UpdateFn&& update_fn)
        : update_fn_(std::forward<UpdateFn>(update_fn)) {}

        [[nodiscard]] abstract_anim_uptr build() && {
            return std::make_unique<custom_anim_i<UpdateFn>>(
                *this, std::move(update_fn_), duration_, easing_);
        }
    private:
        UpdateFn update_fn_;
    };

    template < typename F >
    [[nodiscard]] auto ui_animation::custom(F&& fn) {
        auto make_custom_impl = [](auto&& fn) {
            using fn_t = std::remove_reference_t<decltype(fn)>;
            return custom_impl<fn_t>(std::move(fn));
        };

        using fi = func_info<F>;
        static_assert(std::tuple_size_v<fi::args> < 3);

        if constexpr( std::tuple_size_v<fi::args> == 0 ) {
            return make_custom_impl(
                [fn = std::forward<F>(fn)](f32, ecs::entity&) mutable {
                    fn();
                });
        }
        if constexpr( std::tuple_size_v<fi::args> == 1 ) {
            static_assert(std::is_same_v<typename std::tuple_element<0, fi::args>::type, f32>);
            return make_custom_impl(
                [fn = std::forward<F>(fn)](f32 f, ecs::entity&) mutable {
                    fn(f);
                });
        }
        if constexpr( std::tuple_size_v<fi::args> == 2 ) {
            using second_t = std::remove_reference_t<std::remove_cv_t<typename std::tuple_element<1, fi::args>::type>>;
            static_assert(std::is_same_v<typename std::tuple_element<0, fi::args>::type, f32>);

            if constexpr( std::is_same_v<second_t, ecs::entity> ) {
                return make_custom_impl(std::forward<F>(fn));
            } else {
                // second argument is component type
                return make_custom_impl(
                    [fn = std::forward<F>(fn)](f32 f, ecs::entity& e) mutable {
                        auto& comp = e.get_component<second_t>();
                        fn(f, comp);
                    });
            }
        }
    }

    //
    // ui_animation::scale
    //

    class ui_animation::scale final : public property_anim_builder<ui_animation::scale> {
    public:
        scale() = default;

        ui_animation::scale&& from(f32 value) && noexcept;
        ui_animation::scale&& from(const v3f& value) && noexcept;
        
        ui_animation::scale&& to(f32 value) && noexcept;
        ui_animation::scale&& to(const v3f& value) && noexcept;
        
        [[nodiscard]] abstract_anim_uptr build() &&;
    private:
        std::optional<v3f> from_scale_;
        std::optional<v3f> to_scale_;
    };

    //
    // ui_animation::move
    //

    class ui_animation::move final : public property_anim_builder<ui_animation::move> {
    public:
        move() = default;

        ui_animation::move&& from(const v3f& value) && noexcept;
        ui_animation::move&& to(const v3f& value) && noexcept;
        
        [[nodiscard]] abstract_anim_uptr build() &&;
    private:
        std::optional<v3f> from_pos_;
        std::optional<v3f> to_pos_;
    };

    //
    // ui_animation::size
    //

    class ui_animation::size final : public property_anim_builder<ui_animation::size> {
    public:
        size() = default;

        ui_animation::size&& from(const v2f& value) && noexcept;
        ui_animation::size&& to(const v2f& value) && noexcept;
        
        ui_animation::size&& from_width(f32 value) && noexcept;
        ui_animation::size&& to_width(f32 value) && noexcept;
        ui_animation::size&& from_height(f32 value) && noexcept;
        ui_animation::size&& to_height(f32 value) && noexcept;
        
        [[nodiscard]] abstract_anim_uptr build() &&;
    private:
        std::optional<f32> from_width_;
        std::optional<f32> to_width_;
        std::optional<f32> from_height_;
        std::optional<f32> to_height_;
    };
}

namespace e2d
{
    template < typename T >
    template < typename F >
    T&& ui_animation::anim_builder_t<T>::on_start(F&& fn) && {
        on_start_ = std::forward<F>(fn);
        return std::move(static_cast<T&>(*this));
    }
            
    template < typename T >
    template < typename F >
    T&& ui_animation::anim_builder_t<T>::on_complete(F&& fn) && {
        on_complete_ = std::forward<F>(fn);
        return std::move(static_cast<T&>(*this));
    }
            
    template < typename T >
    template < typename F >
    T&& ui_animation::anim_builder_t<T>::on_step_complete(F&& fn) && {
        on_step_complete_ = std::forward<F>(fn);
        return std::move(static_cast<T&>(*this));
    }
    
    template < typename T >
    T&& ui_animation::anim_builder_t<T>::repeat(u32 count) && noexcept {
        loops_ = math::numeric_cast<i32>(count);
        return std::move(static_cast<T&>(*this));
    }
    
    template < typename T >
    T&& ui_animation::anim_builder_t<T>::infinite_loops() && noexcept {
        loops_ = std::numeric_limits<decltype(loops_)>::max();
        return std::move(static_cast<T&>(*this));
    }
    
    template < typename T >
    T&& ui_animation::anim_builder_t<T>::repeat_inversed() && noexcept {
        repeat_inversed_ = true;
        return std::move(static_cast<T&>(*this));
    }

    template < typename T >
    T&& ui_animation::anim_builder_t<T>::delay(secf value) && noexcept {
        delay_ = value;
        return std::move(static_cast<T&>(*this));
    }
}

namespace e2d
{
    template < typename T >
    T&& ui_animation::property_anim_builder<T>::duration(secf value) && noexcept {
        duration_ = value;
        return std::move(static_cast<T&>(*this));
    }

    template < typename T >
    T&& ui_animation::property_anim_builder<T>::ease(easing_fn fn) && noexcept {
        E2D_ASSERT(fn);
        easing_ = fn;
        return std::move(static_cast<T&>(*this));
    }
    
    template < typename T >
    ui_animation::easing_fn ui_animation::property_anim_builder<T>::easing() const noexcept {
        return easing_;
    }
    
    template < typename T >
    secf ui_animation::property_anim_builder<T>::duration() const noexcept {
        return duration_;
    }
}

namespace e2d
{
    template < typename A >
    std::enable_if_t<
        std::is_base_of_v<ui_animation::anim_builder, A>,
        ui_animation::parallel&&>
    ui_animation::parallel::add(A&& value) && {
        animations_.push_back(std::forward<A>(value).build());
        return std::move(*this);
    }
        
    template < typename A >
    std::enable_if_t<
        std::is_base_of_v<ui_animation::anim_builder, A>,
        ui_animation::sequential&&>
    ui_animation::sequential::add(A&& value) && {
        animations_.push_back(std::forward<A>(value).build());
        return std::move(*this);
    }
}

namespace e2d
{
    template < typename A >
    ui_animation::ui_animation(A&& value) {
        set_animation(std::forward<A>(value));
    }

    template < typename A >
    std::enable_if_t<std::is_base_of_v<ui_animation::anim_builder, A>, ui_animation&>
    ui_animation::set_animation(A&& value) noexcept {
        return set_animation(std::forward<A>(value).build());
    }
}

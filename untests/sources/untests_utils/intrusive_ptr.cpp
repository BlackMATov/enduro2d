/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include "_utils.hpp"
using namespace e2d;

namespace
{
    class obj_t : public ref_counter<obj_t> {
    public:
        obj_t() {
            ++ctor_counter;
        }

        obj_t(int ni) : i(ni) {
            ++ctor_counter;
        }

        virtual ~obj_t() noexcept {
            ++dtor_counter;
        }

        int i{42};
    public:
        static int ctor_counter;
        static int dtor_counter;
        static void reset_counters() {
            ctor_counter = 0u;
            dtor_counter = 0u;
        }
    };
    int obj_t::ctor_counter = 0;
    int obj_t::dtor_counter = 0;

    class derived_t final : public obj_t {
    public:
        derived_t() : obj_t(84) {}
        int j{100500};
    };

    class derived2_t final : public obj_t {
    public:
        derived2_t() : obj_t(21) {}
        int j{500100};
    };

    class obj2_t : public ref_counter<obj2_t, ref_counter_thread_unsafe_policy<>> {
    public:
        obj2_t() {
            ++ctor_counter;
        }

        obj2_t(int ni) : i(ni) {
            ++ctor_counter;
        }

        virtual ~obj2_t() noexcept {
            ++dtor_counter;
        }

        int i{42};
    public:
        static int ctor_counter;
        static int dtor_counter;
        static void reset_counters() {
            ctor_counter = 0u;
            dtor_counter = 0u;
        }
    };
    int obj2_t::ctor_counter = 0;
    int obj2_t::dtor_counter = 0;
}

TEST_CASE("intrusive_ptr") {
    {
        intrusive_ptr<obj_t> p;
        REQUIRE_FALSE(p);
        REQUIRE(p.get() == nullptr);
        REQUIRE(p.release() == nullptr);
    }
    {
        obj_t::reset_counters();
        intrusive_ptr<obj_t> p(new obj_t());
        REQUIRE(p);
        REQUIRE(p.get());
        REQUIRE(p.get()->i == 42);
        REQUIRE(p->i == 42);
        REQUIRE((*p).i == 42);
        REQUIRE(p->use_count() == 1u);
        REQUIRE(obj_t::ctor_counter == 1u);
        REQUIRE(obj_t::dtor_counter == 0);
        p.reset();
        REQUIRE_FALSE(p);
        REQUIRE(p.get() == nullptr);
        REQUIRE(p.release() == nullptr);
        REQUIRE(obj_t::dtor_counter == 1u);
    }
    {
        obj_t::reset_counters();
        intrusive_ptr<obj_t> p(new obj_t());
        obj_t* o = p.release();
        REQUIRE_FALSE(p);
        REQUIRE(p.get() == nullptr);
        REQUIRE(obj_t::ctor_counter == 1u);
        REQUIRE(obj_t::dtor_counter == 0u);
        REQUIRE(o->use_count() == 1u);

        {
            intrusive_ptr<obj_t> p2(o, false);
            REQUIRE(p2);
            REQUIRE(p2->use_count() == 1u);

            o = p2.release();
            REQUIRE_FALSE(p2);

            p2.reset(o, false);
            REQUIRE(p2);
            REQUIRE(p2->use_count() == 1u);
            REQUIRE(obj_t::ctor_counter == 1u);
            REQUIRE(obj_t::dtor_counter == 0u);

            p2.reset(new obj_t());
            REQUIRE(p2);
            REQUIRE(obj_t::ctor_counter == 2u);
            REQUIRE(obj_t::dtor_counter == 1u);
        }
        REQUIRE(obj_t::ctor_counter == 2u);
        REQUIRE(obj_t::dtor_counter == 2u);
    }
    {
        obj_t::reset_counters();
        {
            intrusive_ptr<obj_t> p1(new obj_t(10));
            intrusive_ptr<obj_t> p2(new obj_t(20));

            p1.swap(p2);
            REQUIRE(p1->i == 20);
            REQUIRE(p2->i == 10);
        }
        REQUIRE(obj_t::ctor_counter == 2u);
        REQUIRE(obj_t::dtor_counter == 2u);
    }
    {
        intrusive_ptr<obj_t> p1(new obj_t(10));
        intrusive_ptr<obj_t> p2(new obj_t(20));

        {
            intrusive_ptr<obj_t> p3;

            REQUIRE_FALSE(p2 == nullptr);
            REQUIRE_FALSE(nullptr == p2);
            REQUIRE(p2 != nullptr);
            REQUIRE(nullptr != p2);

            REQUIRE(p3 == nullptr);
            REQUIRE(nullptr == p3);
            REQUIRE_FALSE(p3 != nullptr);
            REQUIRE_FALSE(nullptr != p3);
        }

        {
            REQUIRE((p1 == p1) == (p1.get() == p1.get()));
            REQUIRE((p1 != p1) == (p1.get() != p1.get()));

            REQUIRE((p1 == p1.get()) == (p1.get() == p1.get()));
            REQUIRE((p1 != p1.get()) == (p1.get() != p1.get()));

            REQUIRE((p1.get() == p1) == (p1.get() == p1.get()));
            REQUIRE((p1.get() != p1) == (p1.get() != p1.get()));

            REQUIRE((p1 < p1) == p1.get() < p1.get());
            REQUIRE((p1 <= p1) == p1.get() <= p1.get());
            REQUIRE((p1 > p1) == p1.get() > p1.get());
            REQUIRE((p1 >= p1) == p1.get() >= p1.get());
        }

        {
            REQUIRE((p2 == p1) == (p2.get() == p1.get()));
            REQUIRE((p2 != p1) == (p2.get() != p1.get()));

            REQUIRE((p2 == p1.get()) == (p2.get() == p1.get()));
            REQUIRE((p2 != p1.get()) == (p2.get() != p1.get()));

            REQUIRE((p2.get() == p1) == (p2.get() == p1.get()));
            REQUIRE((p2.get() != p1) == (p2.get() != p1.get()));

            REQUIRE((p2 < p1) == p2.get() < p1.get());
            REQUIRE((p2 <= p1) == p2.get() <= p1.get());
            REQUIRE((p2 > p1) == p2.get() > p1.get());
            REQUIRE((p2 >= p1) == p2.get() >= p1.get());
        }
    }
    {
        obj_t::reset_counters();
        {
            intrusive_ptr<obj_t> p1(new derived_t());
            REQUIRE(p1->i == 84);
            intrusive_ptr<obj_t> p2 = p1;
            REQUIRE(p2->i == 84);
            REQUIRE(p2->use_count() == 2u);
        }
        REQUIRE(obj_t::ctor_counter == 1u);
        REQUIRE(obj_t::dtor_counter == 1u);
        {
            intrusive_ptr<obj_t> p1(new derived_t());
            intrusive_ptr<obj_t> p2;
            p2 = p1;
            REQUIRE(p2->i == 84);
        }
        REQUIRE(obj_t::ctor_counter == 2u);
        REQUIRE(obj_t::dtor_counter == 2u);
        {
            intrusive_ptr<obj_t> p1(new derived_t());
            intrusive_ptr<obj_t> p2 = std::move(p1);
            REQUIRE(p2->i == 84);
            REQUIRE_FALSE(p1);
        }
        REQUIRE(obj_t::ctor_counter == 3u);
        REQUIRE(obj_t::dtor_counter == 3u);
        {
            intrusive_ptr<obj_t> p1(new derived_t());
            intrusive_ptr<obj_t> p2;
            p2 = std::move(p1);
            REQUIRE(p2->i == 84);
            REQUIRE_FALSE(p1);
        }
        REQUIRE(obj_t::ctor_counter == 4u);
        REQUIRE(obj_t::dtor_counter == 4u);
    }
    {
        E2D_UNUSED(std::make_unique<obj_t>(10));
        intrusive_ptr<const obj_t> p1(new derived2_t());
        intrusive_ptr<const derived2_t> p2 = static_pointer_cast<const derived2_t>(p1);
        REQUIRE(p2->i == 21);
        REQUIRE(p2->j == 500100);
        intrusive_ptr<const derived2_t> p3 = dynamic_pointer_cast<const derived2_t>(p1);
        REQUIRE(p3->i == 21);
        REQUIRE(p3->j == 500100);
        intrusive_ptr<const derived_t> p4 = dynamic_pointer_cast<const derived_t>(p1);
        REQUIRE_FALSE(p4);
        intrusive_ptr<obj_t> p5 = const_pointer_cast<obj_t>(p1);
        REQUIRE(p5);
        REQUIRE(p5->i == 21);
    }
    {
        auto p1 = make_intrusive<obj_t>(10);
        hash_set<intrusive_ptr<obj_t>> s;
        s.insert(p1);
    }
    {
        auto p = make_intrusive<obj2_t>(10);
        REQUIRE(p->i == 10);
        REQUIRE(p->use_count() == 1u);
        p.reset();
        REQUIRE(obj2_t::ctor_counter == 1u);
        REQUIRE(obj2_t::dtor_counter == 1u);
    }
}

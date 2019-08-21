/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include <enduro2d/high/_high.hpp>

#include <enduro2d/high/node.hpp>
#include <enduro2d/high/components/camera.hpp>

#include "render_system_base.hpp"

namespace e2d::render_system_impl
{
    class bad_drawer_operation final : public exception {
    public:
        const char* what() const noexcept final {
            return "bad drawer operation";
        }
    };

    class drawer : private noncopyable {
    public:
        class context : noncopyable {
        public:
            context(
                const camera& cam,
                const render_technique* rt,
                const const_node_iptr& cam_n,
                engine& engine,
                render& render);
            ~context() noexcept;

            void draw(
                const const_node_iptr& node);

            void draw(
                const const_node_iptr& node,
                const renderer& node_r,
                const model_renderer& mdl_r);

            void draw(
                const const_node_iptr& node,
                const renderer& node_r,
                const sprite_renderer& spr_r);
            
            void draw(
                const const_node_iptr& node,
                const renderer& node_r,
                const spine_renderer& spine_r);

            void flush();
        private:
            render& render_;
        };
    public:
        drawer(engine& e, render& r);

        template < typename F >
        void with(const camera& cam,
            const render_technique* rt,
            const const_node_iptr& cam_n,
            F&& f);
    private:
        engine& engine_;
        render& render_;
    };
}

namespace e2d::render_system_impl
{
    template < typename F >
    void drawer::with(const camera& cam, const render_technique* rt, const const_node_iptr& cam_n, F&& f) {
        context ctx{cam, rt, cam_n, engine_, render_};
        std::forward<F>(f)(ctx);
        ctx.flush();
    }
}

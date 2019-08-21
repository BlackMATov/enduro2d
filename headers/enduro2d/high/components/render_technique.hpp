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
    class render_technique final {
    public:
        struct pass {
            render::renderpass_desc desc;
            cbuffer_template_ptr templ;
            render::property_map properties;
        };
    public:
        render_technique() = default;

        render_technique& add_pass(pass p);
        
        std::vector<pass>& passes() noexcept;
        const std::vector<pass>& passes() const noexcept;
    private:
        std::vector<pass> passes_;
    };

    template <>
    class factory_loader<render_technique> final : factory_loader<> {
    public:
        static const char* schema_source;

        bool operator()(
            render_technique& component,
            const fill_context& ctx) const;
            
        bool operator()(
            asset_dependencies& dependencies,
            const collect_context& ctx) const;
    };
}

namespace e2d
{
    inline render_technique& render_technique::add_pass(pass p) {
        passes_.push_back(std::move(p));
        return *this;
    }
    
    inline std::vector<render_technique::pass>& render_technique::passes() noexcept {
        return passes_;
    }

    inline const std::vector<render_technique::pass>& render_technique::passes() const noexcept {
        return passes_;
    }
}

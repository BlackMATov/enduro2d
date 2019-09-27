/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "_high.hpp"

#include "asset.hpp"

namespace e2d
{
    //
    // library_cancelled_exception
    //

    class library_cancelled_exception : public exception {
        const char* what() const noexcept override {
            return "library cancelled exception";
        }
    };

    //
    // loading_asset
    //

    class loading_asset;
    using loading_asset_iptr = intrusive_ptr<loading_asset>;

    class loading_asset
        : private noncopyable
        , public ref_counter<loading_asset> {
    public:
        loading_asset() = default;
        virtual ~loading_asset() noexcept = default;

        virtual void cancel() noexcept = 0;
        virtual str_hash address() const noexcept = 0;
        virtual void wait(deferrer& deferrer) const noexcept = 0;
    };

    template < typename Asset >
    class typed_loading_asset : public loading_asset {
    public:
        using ptr = intrusive_ptr<typed_loading_asset>;
        using promise_type = typename Asset::load_async_result;
    public:
        typed_loading_asset(str_hash address, promise_type promise);
        ~typed_loading_asset() noexcept override = default;

        void cancel() noexcept override;
        str_hash address() const noexcept override;
        void wait(deferrer& deferrer) const noexcept override;

        const promise_type& promise() const noexcept;
    private:
        str_hash address_;
        promise_type promise_;
    };

    //
    // library
    //

    class library final : public module<library> {
    public:
        library(const url& root, deferrer& deferrer);
        ~library() noexcept final;

        const url& root() const noexcept;
        const asset_cache& cache() const noexcept;

        void add_env_value(str_hash key, bool value);
        bool get_env_value(str_hash key) const noexcept;

        std::size_t unload_unused_assets() noexcept;
        std::size_t loading_asset_count() const noexcept;

        template < typename Asset >
        typename Asset::load_result load_main_asset(str_view address) const;

        template < typename Asset >
        typename Asset::load_async_result load_main_asset_async(str_view address) const;

        template < typename Asset, typename Nested = Asset >
        typename Nested::load_result load_asset(str_view address) const;

        template < typename Asset, typename Nested = Asset >
        typename Nested::load_async_result load_asset_async(str_view address) const;
    private:
        template < typename Asset >
        vector<loading_asset_iptr>::iterator
        find_loading_asset_iter_(str_hash address) const noexcept;

        template < typename Asset >
        typename typed_loading_asset<Asset>::ptr
        find_loading_asset_(str_hash address) const noexcept;

        template < typename Asset >
        void remove_loading_asset_(str_hash address) const noexcept;

        void wait_all_loading_assets_() noexcept;
    private:
        url root_;
        deferrer& deferrer_;
        std::atomic<bool> cancelled_{false};
        flat_map<str_hash, bool> env_values_;
    private:
        mutable asset_cache cache_;
        mutable std::recursive_mutex mutex_;
        mutable vector<loading_asset_iptr> loading_assets_;
    };

    //
    // asset_group
    //

    class asset_group {
    public:
        asset_group() = default;
        ~asset_group() noexcept = default;

        template < typename Iter >
        asset_group& add_assets(Iter first, Iter last);

        template < typename Container >
        asset_group& add_assets(Container&& container);

        asset_group& add_asset(str_view address, const asset_ptr& asset);

        template < typename Asset, typename Nested = Asset >
        typename Nested::load_result find_asset(str_view address) const;
    private:
        flat_multimap<str, asset_ptr> assets_;
    };

    //
    // asset_dependency
    //

    class asset_dependency;
    using asset_dependency_iptr = intrusive_ptr<asset_dependency>;

    class asset_dependency
        : private noncopyable
        , public ref_counter<asset_dependency> {
    public:
        asset_dependency() = default;
        virtual ~asset_dependency() noexcept = default;

        virtual const str& main_address() const noexcept = 0;
        virtual stdex::promise<asset_ptr> load_async(const library& library) = 0;
    };

    template < typename Asset >
    class typed_asset_dependency : public asset_dependency {
    public:
        using asset_type = Asset;
        using load_result = typename Asset::load_result;
    public:
        typed_asset_dependency(str_view address);
        ~typed_asset_dependency() noexcept override;

        const str& main_address() const noexcept override;
        stdex::promise<asset_ptr> load_async(const library& library) override;
    private:
        str main_address_;
    };

    //
    // asset_dependencies
    //

    class asset_dependencies final {
    public:
        asset_dependencies() = default;
        ~asset_dependencies() noexcept = default;

        template < typename Asset, typename Nested = Asset >
        asset_dependencies& add_dependency(str_view address);
        stdex::promise<asset_group> load_async(const library& library) const;
    private:
        flat_multimap<str, asset_dependency_iptr> dependencies_;
    };
}

#include "library.inl"

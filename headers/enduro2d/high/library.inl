/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#pragma once

#include "library.hpp"

namespace e2d
{
    //
    // loading_asset
    //

    template < typename Asset >
    typed_loading_asset<Asset>::typed_loading_asset(str_hash address, promise_type promise)
    : address_(address)
    , promise_(std::move(promise)) {}

    template < typename Asset >
    void typed_loading_asset<Asset>::cancel() noexcept {
        promise_.reject(library_cancelled_exception());
    }

    template < typename Asset >
    str_hash typed_loading_asset<Asset>::address() const noexcept {
        return address_;
    }

    template < typename Asset >
    const typename typed_loading_asset<Asset>::promise_type&
    typed_loading_asset<Asset>::promise() const noexcept {
        return promise_;
    }

    template < typename Asset >
    void typed_loading_asset<Asset>::wait(deferrer& deferrer) const noexcept {
        deferrer.active_safe_wait_promise(promise_);
    }

    //
    // library
    //

    inline library::library(const url& root, deferrer& deferrer)
    : root_(root)
    , deferrer_(deferrer) {}

    inline library::~library() noexcept {
        cancelled_.store(true);
        wait_all_loading_assets_();
    }

    inline const url& library::root() const noexcept {
        return root_;
    }

    inline const asset_cache& library::cache() const noexcept {
        return cache_;
    }
    
    inline void library::add_env_value(str_hash key, bool value) {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        env_values_.insert_or_assign(key, value);
    }

    inline bool library::get_env_value(str_hash key) const noexcept {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        auto iter = env_values_.find(key);
        return iter != env_values_.end()
            ? iter->second
            : false;
    }

    inline std::size_t library::unload_unused_assets() noexcept {
        return cache_.unload_unused_assets();
    }

    inline std::size_t library::loading_asset_count() const noexcept {
        std::lock_guard<std::recursive_mutex> guard(mutex_);
        return loading_assets_.size();
    }

    template < typename Asset >
    typename Asset::load_result library::load_main_asset(str_view address) const {
        auto p = load_main_asset_async<Asset>(address);
        deferrer_.active_safe_wait_promise(p);
        return p.get_or_default(nullptr);
    }

    template < typename Asset >
    typename Asset::load_async_result library::load_main_asset_async(str_view address) const {
        const str main_address = address::parent(address);
        const str_hash main_address_hash = make_hash(main_address);

        std::lock_guard<std::recursive_mutex> guard(mutex_);

        if ( cancelled_ ) {
            return stdex::make_rejected_promise<typename Asset::load_result>(library_cancelled_exception());
        }

        if ( auto cached_asset = cache_.find<Asset>(main_address_hash) ) {
            return stdex::make_resolved_promise(std::move(cached_asset));
        }

        if ( auto asset = find_loading_asset_<Asset>(main_address_hash) )  {
            return asset->promise();
        }

        auto p = Asset::load_async(*this, main_address)
        .then([
            this,
            main_address_hash
        ](const typename Asset::load_result& new_asset){
            std::lock_guard<std::recursive_mutex> guard(mutex_);
            cache_.store<Asset>(main_address_hash, new_asset);
            remove_loading_asset_<Asset>(main_address_hash);
            return new_asset;
        }).except([
            this,
            main_address,
            main_address_hash
        ](std::exception_ptr e) -> typename Asset::load_result {
            {
                std::lock_guard<std::recursive_mutex> guard(mutex_);
                remove_loading_asset_<Asset>(main_address_hash);
            }
            try {
                std::rethrow_exception(e);
            } catch ( const std::exception& ee ) {
                the<debug>().error("LIBRARY: Failed to load asset:\n"
                    "--> Asset: %0\n"
                    "--> Address: %1\n"
                    "--> Exception: %2",
                    Asset::type_name(),
                    main_address,
                    ee.what());
            } catch (...) {
                the<debug>().error("LIBRARY: Failed to load asset:\n"
                    "--> Asset: %0\n"
                    "--> Address: %1\n"
                    "--> Exception: unexpected",
                    Asset::type_name(),
                    main_address);
            }
            std::rethrow_exception(e);
        });

        const auto zero_us = time::to_chrono(make_microseconds(0));
        if ( p.wait_for(zero_us) == stdex::promise_wait_status::timeout ) {
            loading_assets_.push_back(new typed_loading_asset<Asset>(main_address_hash, p));
        }

        return p;
    }

    template < typename Asset, typename Nested >
    typename Nested::load_result library::load_asset(str_view address) const {
        auto p = load_asset_async<Asset, Nested>(address);
        deferrer_.active_safe_wait_promise(p);
        return p.get_or_default(nullptr);
    }

    template < typename Asset, typename Nested >
    typename Nested::load_async_result library::load_asset_async(str_view address) const {
        return load_main_asset_async<Asset>(address)
        .then([
            address = str(address),
            nested_address = address::nested(address)
        ](const typename Asset::load_result& main_asset){
            typename Nested::load_result nested_asset = nested_address.empty()
                ? dynamic_pointer_cast<Nested>(main_asset)
                : main_asset->template find_nested_asset<Nested>(nested_address);
            if ( nested_asset ) {
                return nested_asset;
            }
            the<debug>().error("LIBRARY: Failed to load asset:\n"
                "--> Asset: %0\n"
                "--> Nested: %1\n"
                "--> Address: %2\n",
                Asset::type_name(),
                Nested::type_name(),
                address);
            throw asset_loading_exception();
        });
    }

    template < typename Asset >
    vector<loading_asset_iptr>::iterator
    library::find_loading_asset_iter_(str_hash address) const noexcept {
        return std::find_if(
            loading_assets_.begin(), loading_assets_.end(),
            [address](const loading_asset_iptr& asset) noexcept {
                return asset->address() == address
                    && dynamic_pointer_cast<typed_loading_asset<Asset>>(asset);
            });
    }

    template < typename Asset >
    typename typed_loading_asset<Asset>::ptr
    library::find_loading_asset_(str_hash address) const noexcept {
        auto iter = find_loading_asset_iter_<Asset>(address);
        return iter != loading_assets_.end()
            ? static_pointer_cast<typed_loading_asset<Asset>>(*iter)
            : nullptr;
    }

    template < typename Asset >
    void library::remove_loading_asset_(str_hash address) const noexcept {
        auto iter = find_loading_asset_iter_<Asset>(address);
        if ( iter != loading_assets_.end() ) {
            loading_assets_.erase(iter);
        }
    }

    inline void library::wait_all_loading_assets_() noexcept {
        while ( true ) {
            std::unique_lock<std::recursive_mutex> lock(mutex_);
            if ( loading_assets_.empty() ) {
                break;
            }
            const auto loading_asset_copy = loading_assets_.back();
            lock.unlock();
            loading_asset_copy->wait(deferrer_);
        }
    }

    //
    // asset_group
    //

    template < typename Iter >
    asset_group& asset_group::add_assets(Iter first, Iter last) {
        for ( auto iter = first; iter != last; ++iter ) {
            add_asset(iter->first, iter->second);
        }
        return *this;
    }

    template < typename Container >
    asset_group& asset_group::add_assets(Container&& container) {
        return add_assets(std::begin(container), std::end(container));
    }

    inline asset_group& asset_group::add_asset(str_view address, const asset_ptr& asset) {
        str main_address = address::parent(address);
        assets_.emplace(std::move(main_address), asset);
        return *this;
    }

    template < typename Asset, typename Nested >
    typename Nested::load_result asset_group::find_asset(str_view address) const {
        const str main_address = address::parent(address);
        const str nested_address = address::nested(address);
        const auto [first, last] = assets_.equal_range(main_address);
        for ( auto iter = first; iter != last; ++iter ) {
            asset_ptr main_asset = iter->second;
            typename Asset::load_result typed_main_asset = dynamic_pointer_cast<Asset>(main_asset);
            if ( !typed_main_asset ) {
                continue;
            }
            typename Nested::load_result nested_asset = nested_address.empty()
                ? dynamic_pointer_cast<Nested>(typed_main_asset)
                : typed_main_asset->template find_nested_asset<Nested>(nested_address);
            if ( nested_asset ) {
                return nested_asset;
            }
        }
        return nullptr;
    }

    //
    // asset_dependency
    //

    template < typename Asset >
    typed_asset_dependency<Asset>::typed_asset_dependency(str_view address)
    : main_address_(address::parent(address)) {}

    template < typename Asset >
    typed_asset_dependency<Asset>::~typed_asset_dependency() noexcept = default;

    template < typename Asset >
    const str& typed_asset_dependency<Asset>::main_address() const noexcept {
        return main_address_;
    }

    template < typename Asset >
    stdex::promise<asset_ptr> typed_asset_dependency<Asset>::load_async(const library& library) {
        return library.load_main_asset_async<Asset>(main_address_)
        .then([](const typename Asset::load_result& main_asset){
            return asset_ptr(main_asset);
        });
    }

    //
    // asset_dependencies
    //

    template < typename Asset, typename Nested >
    asset_dependencies& asset_dependencies::add_dependency(str_view address) {
        asset_dependency_iptr dep(new typed_asset_dependency<Asset>(address));
        dependencies_.emplace(dep->main_address(), std::move(dep));
        return *this;
    }

    inline stdex::promise<asset_group> asset_dependencies::load_async(const library& library) const {
        vector<stdex::promise<std::pair<str, asset_ptr>>> promises;
        promises.reserve(dependencies_.size());
        for ( const auto& dp : dependencies_ ) {
            promises.push_back(dp.second->load_async(library).then([
                dep = dp.second
            ](const asset_ptr& asset){
                return std::make_pair(dep->main_address(), asset);
            }));
        }
        return stdex::make_all_promise(std::move(promises))
        .then([](const vector<std::pair<str, asset_ptr>>& results){
            return asset_group().add_assets(results);
        });
    }
}

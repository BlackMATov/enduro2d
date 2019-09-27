/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/utils/buffer.hpp>

namespace
{
    using namespace e2d;

    u8* allocate(std::size_t size) {
        return static_cast<u8*>(::operator new(size));
    }

    void deallocate(u8* ptr) {
        ::operator delete(ptr);
    }
}

namespace e2d
{
    void buffer::deleter::operator()(u8* ptr) const noexcept {
        deallocate(ptr);
    }
    
    buffer::buffer(buffer&& other) noexcept {
        assign(std::move(other));
    }

    buffer& buffer::operator=(buffer&& other) noexcept {
        return assign(std::move(other));
    }

    buffer::buffer(const buffer& other) {
        assign(other);
    }

    buffer& buffer::operator=(const buffer& other) {
        return assign(other);
    }

    buffer::buffer(std::size_t size) {
        resize(size);
    }

    buffer::buffer(const void* src, std::size_t size) {
        assign(src, size);
    }

    buffer::iterator buffer::begin() noexcept {
        return data_.get();
    }

    buffer::const_iterator buffer::begin() const noexcept {
        return data_.get();
    }

    buffer::const_iterator buffer::cbegin() const noexcept {
        return data_.get();
    }

    buffer::iterator buffer::end() noexcept {
        return data_.get() + size_;
    }

    buffer::const_iterator buffer::end() const noexcept {
        return data_.get() + size_;
    }

    buffer::const_iterator buffer::cend() const noexcept {
        return data_.get() + size_;
    }

    buffer::reverse_iterator buffer::rbegin() noexcept {
        return reverse_iterator(end());
    }

    buffer::const_reverse_iterator buffer::rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    buffer::const_reverse_iterator buffer::crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    buffer::reverse_iterator buffer::rend() noexcept {
        return reverse_iterator(begin());
    }

    buffer::const_reverse_iterator buffer::rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    buffer::const_reverse_iterator buffer::crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    buffer& buffer::fill(u8 ch) noexcept {
        if ( data_ && size_) {
            std::memset(data_.get(), ch, size_);
        }
        return *this;
    }

    buffer& buffer::resize(std::size_t nsize) {
        data_t ndata = size_ == nsize
            ? std::move(data_)
            : (nsize
                ? data_t(allocate(nsize))
                : data_t());
        if ( ndata && data_ && size_ ) {
            std::memcpy(ndata.get(), data_.get(), math::min(size_, nsize));
        }
        data_.swap(ndata);
        size_ = nsize;
        return *this;
    }

    buffer& buffer::assign(buffer&& other) noexcept {
        if ( this != &other ) {
            swap(other);
            other.clear();
        }
        return *this;
    }

    buffer& buffer::assign(const buffer& other) {
        return this != &other
            ? assign(other.data(), other.size())
            : *this;
    }

    buffer& buffer::assign(const void* src, std::size_t nsize) {
        data_t ndata = size_ == nsize
            ? std::move(data_)
            : (nsize
                ? data_t(allocate(nsize))
                : data_t());
        if ( ndata && src && nsize ) {
            std::memcpy(ndata.get(), src, nsize);
        }
        data_.swap(ndata);
        size_ = nsize;
        return *this;
    }

    void buffer::swap(buffer& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
    }

    void buffer::clear() noexcept {
        data_.reset();
        size_ = 0;
    }

    bool buffer::empty() const noexcept {
        return !size_;
    }

    u8* buffer::data() noexcept {
        return data_.get();
    }

    const u8* buffer::data() const noexcept {
        return data_.get();
    }

    std::size_t buffer::size() const noexcept {
        return size_;
    }
}

namespace e2d
{
    void swap(buffer& l, buffer& r) noexcept {
        l.swap(r);
    }

    bool operator<(const buffer& l, const buffer& r) noexcept {
        const u8* ld = l.data();
        const u8* rd = r.data();
        const std::size_t ls = l.size();
        const std::size_t rs = r.size();
        return
            (ls < rs) ||
            (ls == rs && ls > 0 && std::memcmp(ld, rd, ls) < 0);
    }

    bool operator==(const buffer& l, const buffer& r) noexcept {
        const u8* ld = l.data();
        const u8* rd = r.data();
        const std::size_t ls = l.size();
        const std::size_t rs = r.size();
        return
            (ls == rs) &&
            (ls == 0 || std::memcmp(ld, rd, ls) == 0);
    }

    bool operator!=(const buffer& l, const buffer& r) noexcept {
        return !(l == r);
    }
}

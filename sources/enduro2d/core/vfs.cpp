/*******************************************************************************
 * This file is part of the "Enduro2D"
 * For conditions of distribution and use, see copyright notice in LICENSE.md
 * Copyright (C) 2018-2019, by Matvey Cherevko (blackmatov@gmail.com)
 ******************************************************************************/

#include <enduro2d/core/vfs.hpp>

#include <3rdparty/miniz/miniz.h>

namespace
{
    using namespace e2d;

    template < typename OwnedState >
    class archive_stream final : public input_stream {
        using iter_state_uptr = std::unique_ptr<
            mz_zip_reader_extract_iter_state,
            void(*)(mz_zip_reader_extract_iter_state*)>;
        std::size_t iter_pos_ = 0;
        OwnedState owned_state_;
        iter_state_uptr iter_state_;
    public:
        archive_stream(const OwnedState& owned_state, mz_zip_archive* archive, const char* filename)
        : owned_state_(owned_state)
        , iter_state_(open_iter_state_(archive, filename))
        {
            if ( !iter_state_ ) {
                throw bad_vfs_operation();
            }
        }

        ~archive_stream() noexcept final {
            // reset iter state before owned state
            iter_state_.reset();
        }

        std::size_t read(void* dst, std::size_t size) final {
            E2D_ASSERT(iter_state_);
            std::size_t read_bytes = mz_zip_reader_extract_iter_read(iter_state_.get(), dst, size);
            iter_pos_ += read_bytes;
            return read_bytes;
        }

        std::size_t seek(std::ptrdiff_t offset, bool relative) final {
            E2D_UNUSED(offset, relative);
            throw bad_vfs_operation();
        }

        std::size_t tell() const final {
            return iter_pos_;
        }

        std::size_t length() const noexcept final {
            return math::numeric_cast<std::size_t>(
                iter_state_->file_stat.m_uncomp_size);
        }
    private:
        static iter_state_uptr open_iter_state_(mz_zip_archive* archive, const char* filename) noexcept {
            mz_zip_reader_extract_iter_state* iter_state = mz_zip_reader_extract_file_iter_new(
                archive, filename, MZ_ZIP_FLAG_CASE_SENSITIVE);
            return iter_state_uptr(iter_state, state_deleter_);
        }

        static void state_deleter_(mz_zip_reader_extract_iter_state* state) noexcept {
            if ( state ) {
                mz_zip_reader_extract_iter_free(state);
            }
        }
    };
}

namespace e2d
{
    //
    // vfs
    //

    class vfs::state final : private e2d::noncopyable {
    public:
        std::mutex mutex;
        stdex::jobber worker{1};
        flat_map<str, url> aliases;
        flat_map<str, file_source_uptr> schemes;
    public:
        url resolve_url(const url& url, u8 level = 0) const {
            if ( level > 32 ) {
                throw bad_vfs_operation();
            }
            const auto alias_iter = aliases.find(url.scheme());
            return alias_iter != aliases.cend()
                ? resolve_url(alias_iter->second / url.path(), level + 1)
                : url;
        }

        template < typename F, typename R >
        R with_file_source(const url& url, F&& f, R&& fallback_result) const {
            const auto resolved_url = resolve_url(url);
            const auto scheme_iter = schemes.find(resolved_url.scheme());
            return (scheme_iter != schemes.cend() && scheme_iter->second)
                ? std::invoke(
                    std::forward<F>(f),
                    scheme_iter->second,
                    resolved_url.path())
                : std::forward<R>(fallback_result);
        }
    };

    vfs::vfs()
    : state_(new state()){}

    vfs::~vfs() noexcept = default;

    bool vfs::register_scheme(str_view scheme, file_source_uptr source) {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return (source && source->valid())
            ? state_->schemes.emplace(scheme, std::move(source)).second
            : false;
    }

    bool vfs::unregister_scheme(str_view scheme) noexcept {
        std::lock_guard<std::mutex> guard(state_->mutex);
        const auto iter = state_->schemes.find(scheme);
        return iter != state_->schemes.end()
            ? (state_->schemes.erase(iter), true)
            : false;
    }

    bool vfs::register_scheme_alias(str_view scheme, url alias) {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->aliases.emplace(scheme, std::move(alias)).second;
    }

    bool vfs::unregister_scheme_alias(str_view scheme) noexcept {
        std::lock_guard<std::mutex> guard(state_->mutex);
        const auto iter = state_->aliases.find(scheme);
        return iter != state_->aliases.end()
            ? (state_->aliases.erase(iter), true)
            : false;
    }

    bool vfs::exists(const url& url) const {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->with_file_source(url,
            [](const file_source_uptr& source, const str& path) {
                return source->exists(path);
            }, false);
    }

    input_stream_uptr vfs::read(const url& url) const {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->with_file_source(url,
            [](const file_source_uptr& source, const str& path) {
                return source->read(path);
            }, input_stream_uptr());
    }

    output_stream_uptr vfs::write(const url& url, bool append) const {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->with_file_source(url,
            [&append](const file_source_uptr& source, const str& path) {
                return source->write(path, append);
            }, output_stream_uptr());
    }

    bool vfs::load(const url& url, buffer& dst) const {
        return load_async(url)
            .then([&dst](auto&& src){
                dst = std::forward<decltype(src)>(src);
                return true;
            }).get_or_default(false);
    }

    stdex::promise<buffer> vfs::load_async(const url& url) const {
        return state_->worker.async([this, url](){
            buffer content;
            const input_stream_uptr stream = read(url);
            if ( !stream || !streams::try_read_tail(content, stream) ) {
                throw vfs_load_async_exception();
            }
            return content;
        });
    }

    bool vfs::load_as_string(const url& url, str& dst) const {
        return load_as_string_async(url)
            .then([&dst](auto&& src){
                dst = std::forward<decltype(src)>(src);
                return true;
            }).get_or_default(false);
    }

    stdex::promise<str> vfs::load_as_string_async(const url& url) const {
        return state_->worker.async([this, url](){
            str content;
            const input_stream_uptr stream = read(url);
            if ( !stream || !streams::try_read_tail(content, stream) ) {
                throw vfs_load_async_exception();
            }
            return content;
        });
    }

    bool vfs::trace(const url& url, filesystem::trace_func func) const {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->with_file_source(url,
            [&func](const file_source_uptr& source, const str& path) {
                return source->trace(path, func);
            }, false);
    }

    url vfs::resolve_scheme_aliases(const url& url) const {
        std::lock_guard<std::mutex> guard(state_->mutex);
        return state_->resolve_url(url);
    }

    //
    // archive_file_source
    //

    class archive_file_source::state final : private e2d::noncopyable {
    public:
        using archive_ptr = std::shared_ptr<mz_zip_archive>;
        using stream_ptr = std::shared_ptr<input_stream>;
        archive_ptr archive;
        stream_ptr stream;
    public:
        state(input_stream_uptr nstream)
        : archive(open_archive_(nstream))
        , stream(std::move(nstream)) {}
        ~state() noexcept = default;
    private:
        static archive_ptr open_archive_(const input_stream_uptr& stream) noexcept {
            if ( stream ) {
                mz_zip_archive* archive = static_cast<mz_zip_archive*>(
                    std::calloc(1, sizeof(mz_zip_archive)));
                if ( archive ) {
                    archive->m_pRead = archive_reader_;
                    archive->m_pIO_opaque = stream.get();
                    if ( mz_zip_reader_init(archive, stream->length(), 0) ) {
                        return archive_ptr(archive, archive_deleter_);
                    }
                    std::free(archive);
                }
            }
            return archive_ptr();
        }

        static void archive_deleter_(mz_zip_archive* archive) noexcept {
            if ( archive ) {
                mz_zip_reader_end(archive);
                std::free(archive);
            }
        }

        static size_t archive_reader_(void* opaque, mz_uint64 pos, void* dst, size_t size) noexcept {
            input_stream* stream = static_cast<input_stream*>(opaque);
            return input_sequence(*stream)
                .seek(math::numeric_cast<std::ptrdiff_t>(pos), false)
                .read(dst, size)
                .success() ? size : 0;
        }
    };

    archive_file_source::archive_file_source(input_stream_uptr stream)
    : state_(new state(std::move(stream))) {}
    archive_file_source::~archive_file_source() noexcept = default;

    bool archive_file_source::valid() const noexcept {
        return !!state_->archive;
    }

    bool archive_file_source::exists(str_view path) const {
        return -1 != mz_zip_reader_locate_file(
            state_->archive.get(),
            make_utf8(path).c_str(),
            nullptr,
            MZ_ZIP_FLAG_CASE_SENSITIVE);
    }

    input_stream_uptr archive_file_source::read(str_view path) const {
        try {
            struct owned_state_t {
                state::archive_ptr archive;
                state::stream_ptr stream;
            } owned_state{state_->archive, state_->stream};
            return std::make_unique<archive_stream<owned_state_t>>(
                std::move(owned_state),
                state_->archive.get(),
                make_utf8(path).c_str());
        } catch (...) {
            return nullptr;
        }
    }

    output_stream_uptr archive_file_source::write(str_view path, bool append) const {
        E2D_UNUSED(path, append);
        return nullptr;
    }

    bool archive_file_source::trace(str_view path, filesystem::trace_func func) const {
        str parent = make_utf8(path);
        if ( !parent.empty() ) {
            if ( parent.back() != '/' ) {
                parent += '/';
            }
            mz_uint32 dir_index = 0;
            if ( !mz_zip_reader_locate_file_v2(
                state_->archive.get(),
                parent.c_str(),
                nullptr,
                MZ_ZIP_FLAG_CASE_SENSITIVE,
                &dir_index) )
            {
                return false;
            }
            mz_zip_archive_file_stat dir_stat;
            if ( !mz_zip_reader_file_stat(
                state_->archive.get(),
                dir_index, &dir_stat) )
            {
                return false;
            }
            if ( !dir_stat.m_is_directory ) {
                return false;
            }
        }
        mz_uint num_files = mz_zip_reader_get_num_files(state_->archive.get());
        for ( mz_uint i = 0; i < num_files; ++i ) {
            mz_zip_archive_file_stat file_stat;
            if ( mz_zip_reader_file_stat(state_->archive.get(), i, &file_stat) ) {
                const str_view filename{file_stat.m_filename};
                if ( filename.size() > parent.size() && strings::starts_with(filename, parent) ) {
                    func(file_stat.m_filename, !!file_stat.m_is_directory);
                }
            }
        }
        return true;
    }

    //
    // filesystem_file_source
    //

    filesystem_file_source::filesystem_file_source() = default;
    filesystem_file_source::~filesystem_file_source() noexcept = default;

    bool filesystem_file_source::valid() const noexcept {
        return true;
    }

    bool filesystem_file_source::exists(str_view path) const {
        return filesystem::file_exists(path);
    }

    input_stream_uptr filesystem_file_source::read(str_view path) const {
        return make_read_file(path);
    }

    output_stream_uptr filesystem_file_source::write(str_view path, bool append) const {
        if ( !filesystem::create_directory_recursive(path::parent_path(path)) ) {
            return nullptr;
        }
        return make_write_file(path, append);
    }

    bool filesystem_file_source::trace(str_view path, filesystem::trace_func func) const {
        return filesystem::trace_directory_recursive(path, func);
    }
}

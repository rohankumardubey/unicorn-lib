#pragma once

#include "unicorn/character.hpp"
#include "unicorn/string.hpp"
#include "unicorn/utf.hpp"
#include "unicorn/utility.hpp"
#include <algorithm>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#ifdef _XOPEN_SOURCE
    #include <sys/stat.h>
#endif

namespace RS::Unicorn {

    // Constants

    struct File {

        static constexpr uint32_t dotdot     = 1ul << 0;  // Include . and ..
        static constexpr uint32_t follow     = 1ul << 1;  // Resolve symlinks
        static constexpr uint32_t fullname   = 1ul << 2;  // Return full file names
        static constexpr uint32_t hidden     = 1ul << 3;  // Include hidden files
        static constexpr uint32_t overwrite  = 1ul << 4;  // Delete existing file if necessary
        static constexpr uint32_t recurse    = 1ul << 5;  // Recursive directory operations
        static constexpr uint32_t unicode    = 1ul << 6;  // Skip files with non-Unicode names

    };

    // Types

    #ifdef _XOPEN_SOURCE

        struct FileId:
        public LessThanComparable<FileId> {
            constexpr FileId() = default;
            constexpr FileId(uint64_t h, uint64_t l) noexcept: hi(h), lo(l) {}
            uint64_t hi = 0, lo = 0;
            size_t hash() const noexcept;
        };

        bool operator==(const FileId& lhs, const FileId& rhs) noexcept;
        bool operator<(const FileId& lhs, const FileId& rhs) noexcept;
        std::istream& operator>>(std::istream& in, FileId& f);
        std::ostream& operator<<(std::ostream& out, const FileId& f);

    #else

        using FileId = uint64_t;

    #endif

    // System dependencies

    #ifdef _XOPEN_SOURCE
        constexpr char file_delimiter = '/';
        constexpr char native_file_delimiter = '/';
    #else
        constexpr char file_delimiter = '\\';
        constexpr wchar_t native_file_delimiter = L'\\';
    #endif

    // File name operations

    namespace UnicornDetail {

        #ifdef _XOPEN_SOURCE

            inline Ustring normalize_path(const Ustring& path) {
                return path;
            }

        #else

            inline Ustring normalize_path(const Ustring& path) {
                auto result = path;
                std::replace(result.begin(), result.end(), '/', '\\');
                return result;
            }

            inline std::wstring normalize_path(const std::wstring& path) {
                auto result = path;
                std::replace(result.begin(), result.end(), L'/', L'\\');
                return result;
            }

        #endif

    }

    bool is_legal_mac_leaf_name(const Ustring& file);
    bool is_legal_mac_path_name(const Ustring& file);
    bool is_legal_posix_leaf_name(const Ustring& file);
    bool is_legal_posix_path_name(const Ustring& file);
    bool is_legal_windows_leaf_name(const std::wstring& file);
    bool is_legal_windows_path_name(const std::wstring& file);
    inline bool is_legal_windows_leaf_name(const Ustring& file) { return is_legal_windows_leaf_name(to_wstring(file)); }
    inline bool is_legal_windows_path_name(const Ustring& file) { return is_legal_windows_path_name(to_wstring(file)); }

    #ifdef _XOPEN_SOURCE

        #ifdef __APPLE__

            inline bool is_legal_leaf_name(const Ustring& file) { return is_legal_mac_leaf_name(file); }
            inline bool is_legal_path_name(const Ustring& file) { return is_legal_mac_path_name(file); }

        #else

            inline bool is_legal_leaf_name(const Ustring& file) { return is_legal_posix_leaf_name(file); }
            inline bool is_legal_path_name(const Ustring& file) { return is_legal_posix_path_name(file); }

        #endif

        bool file_is_absolute(const Ustring& file);
        bool file_is_relative(const Ustring& file);
        bool file_is_root(const Ustring& file);
        inline bool file_is_drive_absolute(const Ustring& /*file*/) { return false; }
        inline bool file_is_drive_relative(const Ustring& /*file*/) { return false; }

    #else

        bool file_is_absolute(const std::wstring& file);
        bool file_is_relative(const std::wstring& file);
        bool file_is_drive_absolute(const std::wstring& file);
        bool file_is_drive_relative(const std::wstring& file);
        bool file_is_root(const std::wstring& file);
        inline bool file_is_absolute(const Ustring& file) { return file_is_absolute(to_wstring(file)); }
        inline bool file_is_relative(const Ustring& file) { return file_is_relative(to_wstring(file)); }
        inline bool file_is_drive_absolute(const Ustring& file) { return file_is_drive_absolute(to_wstring(file)); }
        inline bool file_is_drive_relative(const Ustring& file) { return file_is_drive_relative(to_wstring(file)); }
        inline bool file_is_root(const Ustring& file) { return file_is_root(to_wstring(file)); }
        inline bool is_legal_leaf_name(const std::wstring& file) { return is_legal_windows_leaf_name(file); }
        inline bool is_legal_leaf_name(const Ustring& file) { return is_legal_windows_leaf_name(file); }
        inline bool is_legal_path_name(const std::wstring& file) { return is_legal_windows_path_name(file); }
        inline bool is_legal_path_name(const Ustring& file) { return is_legal_windows_path_name(file); }

    #endif

    inline Ustring file_path(const Ustring& file) { return UnicornDetail::normalize_path(file); }

    template <typename... Args>
    Ustring file_path(const Ustring& file1, const Ustring& file2, Args... args) {
        using namespace UnicornDetail;
        Ustring prefix = normalize_path(file1), suffix = normalize_path(file2);
        if (prefix.empty() || (! suffix.empty() && ! file_is_relative(suffix)))
            return file_path(suffix, args...);
        if (suffix.empty())
            return file_path(prefix, args...);
        if (prefix.back() != file_delimiter)
            prefix += file_delimiter;
        return file_path(prefix + suffix, args...);
    }

    std::pair<Ustring, Ustring> split_path(const Ustring& file, uint32_t flags = 0);
    std::pair<Ustring, Ustring> split_file(const Ustring& file);

    #ifndef _XOPEN_SOURCE

        inline std::wstring file_path(const std::wstring& file) { return UnicornDetail::normalize_path(file); }

        template <typename... Args>
        std::wstring file_path(const std::wstring& file1, const std::wstring& file2, Args... args) {
            using namespace UnicornDetail;
            std::wstring prefix = normalize_path(file1), suffix = normalize_path(file2);
            if (prefix.empty() || (! suffix.empty() && ! file_is_relative(suffix)))
                return file_path(suffix, args...);
            if (suffix.empty())
                return file_path(prefix, args...);
            std::wstring delim;
            if (prefix.back() != native_file_delimiter)
                prefix += native_file_delimiter;
            return file_path(prefix + suffix, args...);
        }

        std::pair<std::wstring, std::wstring> split_path(const std::wstring& file, uint32_t flags = 0);
        std::pair<std::wstring, std::wstring> split_file(const std::wstring& file);

    #endif

    // File system query functions

    #ifdef _XOPEN_SOURCE

        Ustring current_directory();
        bool file_exists(const Ustring& file);
        FileId file_id(const Ustring& file, uint32_t flags = 0);
        bool file_is_directory(const Ustring& file);
        bool file_is_hidden(const Ustring& file);
        bool file_is_symlink(const Ustring& file);
        uint64_t file_size(const Ustring& file, uint32_t flags = 0);
        Ustring resolve_path(const Ustring& file);
        Ustring resolve_symlink(const Ustring& file);
        inline Ustring native_current_directory() { return current_directory(); }

    #else

        std::wstring native_current_directory();
        bool file_exists(const std::wstring& file);
        FileId file_id(const std::wstring& file, uint32_t flags = 0);
        bool file_is_directory(const std::wstring& file);
        bool file_is_hidden(const std::wstring& file);
        bool file_is_symlink(const std::wstring& file);
        uint64_t file_size(const std::wstring& file, uint32_t flags = 0);
        std::wstring resolve_path(const std::wstring& file);
        std::wstring resolve_symlink(const std::wstring& file);
        inline Ustring current_directory() { return to_utf8(native_current_directory()); }
        inline bool file_exists(const Ustring& file) { return file_exists(to_wstring(file)); }
        inline FileId file_id(const Ustring& file, uint32_t flags = 0) { return file_id(to_wstring(file), flags); }
        inline bool file_is_directory(const Ustring& file) { return file_is_directory(to_wstring(file)); }
        inline bool file_is_hidden(const Ustring& file) { return file_is_hidden(to_wstring(file)); }
        inline bool file_is_symlink(const Ustring& file) { return file_is_symlink(to_wstring(file)); }
        inline uint64_t file_size(const Ustring& file, uint32_t flags = 0) { return file_size(to_wstring(file), flags); }
        inline Ustring resolve_path(const Ustring& file) { return to_utf8(resolve_path(to_wstring(file))); }
        inline Ustring resolve_symlink(const Ustring& file) { return to_utf8(resolve_symlink(to_wstring(file))); }

    #endif

    // File system modifying functions

    void copy_file(const NativeString& src, const NativeString& dst, uint32_t flags = 0);
    void make_directory(const NativeString& dir, uint32_t flags = 0);
    void make_symlink(const NativeString& file, const NativeString& link, uint32_t flags = 0);
    void move_file(const NativeString& src, const NativeString& dst, uint32_t flags = 0);
    void remove_file(const NativeString& file, uint32_t flags = 0);

    #ifndef _XOPEN_SOURCE

        inline void copy_file(const Ustring& src, const Ustring& dst, uint32_t flags = 0)
            { return copy_file(to_wstring(src), to_wstring(dst), flags); }
        inline void make_directory(const Ustring& dir, uint32_t flags = 0)
            { return make_directory(to_wstring(dir), flags); }
        inline void make_symlink(const Ustring& file, const Ustring& link, uint32_t flags = 0)
            { return make_symlink(to_wstring(file), to_wstring(link), flags); }
        inline void move_file(const Ustring& src, const Ustring& dst, uint32_t flags = 0)
            { return move_file(to_wstring(src), to_wstring(dst), flags); }
        inline void remove_file(const Ustring& file, uint32_t flags = 0)
            { return remove_file(to_wstring(file), flags); }

    #endif

    // Directory iterators

    class NativeDirectoryIterator:
    public InputIterator<NativeDirectoryIterator, const NativeString> {
    public:
        NativeDirectoryIterator() = default;
        explicit NativeDirectoryIterator(const NativeString& dir, uint32_t flags = 0);
        const NativeString& operator*() const noexcept { return current; }
        NativeDirectoryIterator& operator++();
        bool operator==(const NativeDirectoryIterator& rhs) const noexcept { return impl == rhs.impl; }
    private:
        struct impl_type;
        std::shared_ptr<impl_type> impl;
        NativeString prefix;
        NativeString leaf;
        NativeString current;
        uint32_t fset;
        void do_init(const NativeString& dir);
        void do_next();
    };

    inline Irange<NativeDirectoryIterator> directory(const NativeString& dir, uint32_t flags = 0)
        { return {NativeDirectoryIterator(dir, flags), NativeDirectoryIterator()}; }

    #ifdef _XOPEN_SOURCE

        using DirectoryIterator = NativeDirectoryIterator;

    #else

        class DirectoryIterator:
        public InputIterator<DirectoryIterator, const Ustring> {
        public:
            DirectoryIterator() = default;
            explicit DirectoryIterator(const Ustring& dir, uint32_t flags = 0):
                nat(to_wstring(dir), flags), current(to_utf8(*nat)) {}
            const Ustring& operator*() const noexcept { return current; }
            DirectoryIterator& operator++() { ++nat; current = to_utf8(*nat); return *this; }
            bool operator==(const DirectoryIterator& rhs) const noexcept { return nat == rhs.nat; }
        private:
            NativeDirectoryIterator nat;
            Ustring current;
        };

        inline Irange<DirectoryIterator> directory(const Ustring& dir, uint32_t flags = 0)
            { return {DirectoryIterator(dir, flags), DirectoryIterator()}; }

    #endif

}

#ifdef _XOPEN_SOURCE
    RS_DEFINE_STD_HASH(RS::Unicorn::FileId)
#endif

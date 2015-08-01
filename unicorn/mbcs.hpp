#pragma once

#include "unicorn/core.hpp"
#include "unicorn/character.hpp"
#include "unicorn/utf.hpp"
#include <memory>
#include <stdexcept>
#include <string>

namespace Unicorn {

    // Exceptions

    class UnknownEncoding:
    public std::runtime_error {
    public:
        UnknownEncoding(): std::runtime_error(assemble({}, {})), enc() {}
        explicit UnknownEncoding(const u8string& encoding, const u8string& details = {}):
            std::runtime_error(assemble(encoding, details)), enc(std::make_shared<u8string>(encoding)) {}
        explicit UnknownEncoding(uint32_t encoding, const u8string& details = {}):
            std::runtime_error(assemble(Crow::dec(encoding), details)),
            enc(std::make_shared<u8string>(Crow::dec(encoding))) {}
        const char* encoding() const noexcept { static const char c = 0; return enc ? enc->data() : &c; }
    private:
        std::shared_ptr<u8string> enc;
        static u8string assemble(const u8string& encoding, const u8string& details);
    };

    // Utility functions

    u8string local_encoding(const u8string& default_encoding = "utf-8");
    u8string system_message(int error);

    #if defined(_WIN32)
        u8string windows_message(uint32_t error);
    #endif

    // Conversion functions

    namespace UnicornDetail {

        using EncodingTag =
            #if defined(_XOPEN_SOURCE)
                u8string;
            #else
                uint32_t;
            #endif

        u8string guess_utf(const string& str);
        EncodingTag lookup_encoding(const u8string& name);
        EncodingTag lookup_encoding(uint32_t page);
        void mbcs_flags(Crow::Flagset& flags);
        void native_import(const string& src, NativeString& dst, EncodingTag tag, Crow::Flagset flags);
        void native_export(const NativeString& src, string& dst, EncodingTag tag, Crow::Flagset flags);
        bool utf_import(const string& src, NativeString& dst, EncodingTag tag, Crow::Flagset flags);
        bool utf_export(const NativeString& src, string& dst, EncodingTag tag, Crow::Flagset flags);

        template <typename C, typename E>
        void import_string_helper(const string& src, basic_string<C>& dst, E enc, Crow::Flagset flags) {
            mbcs_flags(flags);
            auto tag = lookup_encoding(enc);
            if (src.empty()) {
                dst.clear();
                return;
            }
            if (tag == EncodingTag()) {
                import_string_helper(src, dst, guess_utf(src), flags);
                return;
            }
            NativeString native_dst;
            if (! utf_import(src, native_dst, tag, flags))
                native_import(src, native_dst, tag, flags);
            recode(native_dst, dst);
        }

        template <typename C, typename E>
        void export_string_helper(const basic_string<C>& src, string& dst, E enc, Crow::Flagset flags) {
            mbcs_flags(flags);
            auto tag = lookup_encoding(enc);
            if (src.empty()) {
                dst.clear();
                return;
            }
            NativeString native_src;
            recode(src, native_src, flags);
            if (! utf_export(native_src, dst, tag, flags))
                native_export(native_src, dst, tag, flags);
        }

    }

    template <typename C>
    void import_string(const string& src, basic_string<C>& dst) {
        UnicornDetail::import_string_helper(src, dst, "char", err_replace);
    }

    template <typename C, typename C2>
    void import_string(const string& src, basic_string<C>& dst, const basic_string<C2>& enc,
            Crow::Flagset flags = {}) {
        UnicornDetail::import_string_helper(src, dst, to_utf8(enc), flags);
    }

    template <typename C, typename C2>
    void import_string(const string& src, basic_string<C>& dst, const C2* enc, Crow::Flagset flags = {}) {
        UnicornDetail::import_string_helper(src, dst, to_utf8(Crow::cstr(enc)), flags);
    }

    template <typename C>
    void import_string(const string& src, basic_string<C>& dst, uint32_t enc, Crow::Flagset flags = {}) {
        UnicornDetail::import_string_helper(src, dst, enc, flags);
    }

    template <typename C>
    void export_string(const basic_string<C>& src, string& dst) {
        UnicornDetail::export_string_helper(src, dst, "char", err_replace);
    }

    template <typename C, typename C2>
    void export_string(const basic_string<C>& src, string& dst, const basic_string<C2>& enc,
            Crow::Flagset flags = {}) {
        UnicornDetail::export_string_helper(src, dst, to_utf8(enc), flags);
    }

    template <typename C, typename C2>
    void export_string(const basic_string<C>& src, string& dst, const C2* enc, Crow::Flagset flags = {}) {
        UnicornDetail::export_string_helper(src, dst, to_utf8(Crow::cstr(enc)), flags);
    }

    template <typename C>
    void export_string(const basic_string<C>& src, string& dst, uint32_t enc, Crow::Flagset flags = {}) {
        UnicornDetail::export_string_helper(src, dst, enc, flags);
    }

}

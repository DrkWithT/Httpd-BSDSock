#include <array>
#include <string_view>
#include "myhttp/types.hpp"

namespace MyHttpd::MyHttp {
    static constexpr std::array<std::string_view, static_cast<std::size_t>(HttpSchema::last) + 1> schemas = {
        "HTTP/1.0",
        "HTTP/1.1",
        "HTTP/x.x"
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(HttpMethod::last) + 1> methods = {
        "GET",
        "POST",
        "NOP"
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(HttpStatus::last) + 1> statuses = {
        "OK",
        "Bad Request",
        "Not Found",
        "Internal Server Error",
        "Not Implemented"
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(MimeType::last) + 1> mimes = {
        "text/plain",
        "text/html",
        "*/*"
    };

    std::string_view stringifyEnum(HttpSchema schema) noexcept {
        const auto schema_n = static_cast<std::size_t>(schema);

        return schemas[schema_n];
    }

    std::string_view stringifyEnum(HttpMethod method) noexcept {
        const auto method_n = static_cast<std::size_t>(method);

        return methods[method_n];
    }

    std::string_view stringifyEnum(HttpStatus status) noexcept {
        const auto status_n = static_cast<std::size_t>(status);

        return statuses[status_n];
    }

    std::string_view stringifyEnum(MimeType mime) noexcept {
        const auto mime_n = static_cast<std::size_t>(mime);

        return mimes[mime_n];
    }

    HttpSchema enumify(std::string_view s, [[maybe_unused]] SchemaOpt opt) noexcept {
        if (s == "HTTP/1.0") {
            return HttpSchema::http_1_0;
        } else if (s == "HTTP/1.1") {
            return HttpSchema::http_1_1;
        }

        return HttpSchema::http_unknown;
    }

    HttpMethod enumify(std::string_view s, [[maybe_unused]] MethodOpt opt) noexcept {
        if (s == "GET") {
            return HttpMethod::h1_get;
        } else if (s == "POST") {
            return HttpMethod::h1_post; 
        }

        return HttpMethod::h1_nop;
    }

    MimeType enumify(std::string_view s, [[maybe_unused]] MimeOpt opt) noexcept {
        if (s == "text/plain") {
            return MimeType::text_plain;
        } else if (s == "text/html") {
            return MimeType::text_html;
        }

        return MimeType::any;
    }
}
#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include "meta/helpers.hpp"
#include "mysock/buffers.hpp"

namespace MyHttpd::MyHttp {
    enum class HttpSchema {
        http_1_0,
        http_1_1,
        http_unknown,
        last = http_unknown
    };

    enum class HttpMethod {
        // h1_head,
        h1_get,
        h1_post,
        h1_nop,
        last = h1_nop
    };

    enum class HttpStatus {
        ok,
        bad_request,
        not_found,
        server_error,
        not_implemented,
        last = not_implemented,
    };

    enum class MimeType {
        text_plain,
        text_html,
        any,
        last = any
    };

    struct SchemaOpt {};
    struct MethodOpt {};
    struct MimeOpt {};

    [[nodiscard]] std::string_view stringifyEnum(HttpSchema schema) noexcept;
    [[nodiscard]] std::string_view stringifyEnum(HttpMethod method) noexcept;
    [[nodiscard]] std::string_view stringifyEnum(HttpStatus status) noexcept;
    [[nodiscard]] std::string_view stringifyEnum(MimeType mime) noexcept;

    [[nodiscard]] HttpSchema enumify(std::string_view s, [[maybe_unused]] SchemaOpt opt) noexcept;
    [[nodiscard]] HttpMethod enumify(std::string_view s, [[maybe_unused]] MethodOpt opt) noexcept;
    [[nodiscard]] MimeType enumify(std::string_view s, [[maybe_unused]] MimeOpt opt) noexcept;

    using HeaderValue = std::variant<int, std::string>;

    struct Request {
        HttpMethod method;
        HttpSchema schema;
        std::string uri;
        MySock::BufferView<Meta::ASCIIOctet> content_vw;
        std::unordered_map<std::string, HeaderValue> headers;
    };

    struct Message {
        HttpStatus status;
        HttpSchema schema;
        std::string_view msg;
        MySock::BufferView<Meta::ASCIIOctet> content_vw;
        std::unordered_map<std::string, HeaderValue> headers;
    };
}
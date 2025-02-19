#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
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
        h1_head,
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

    /// @note Use for translating HTTP/1.x statuses to numerics.
    [[nodiscard]] std::string_view stringifyEnum(HttpStatus status) noexcept;

    /// @note Use for translating HTTP/1.x statuses to messages.
    [[nodiscard]] std::string_view stringifyToMsg(HttpStatus status) noexcept;

    [[nodiscard]] std::string_view stringifyEnum(MimeType mime) noexcept;

    [[nodiscard]] HttpSchema enumify(std::string_view s, [[maybe_unused]] SchemaOpt opt) noexcept;
    [[nodiscard]] HttpMethod enumify(std::string_view s, [[maybe_unused]] MethodOpt opt) noexcept;
    [[nodiscard]] MimeType enumify(std::string_view s, [[maybe_unused]] MimeOpt opt) noexcept;

    using HeaderValue = std::variant<int, std::string>;

    template <typename ItemT> requires (Meta::is_buffer_item_v<ItemT>)
    class DynamicBlob {
    private:
        static constexpr auto growth_n = 2;

        std::unique_ptr<ItemT[]> m_data;
        std::size_t m_size;
        std::size_t m_length;

        [[nodiscard]] bool isFull() const noexcept {
            return m_length >= m_size;
        }

        void pushItem(ItemT item) {
            if (isFull()) {
                expandData(m_size * growth_n);
            }

            m_data[m_length] = item;
            ++m_length;
        }

        void expandData(std::size_t size_next) {
            auto next_block = std::make_unique<ItemT[]>(size_next);
            const ItemT* old_ptr = m_data.get();

            std::copy(old_ptr, old_ptr + m_length, next_block.get());

            m_data = std::move(next_block);
            m_size = size_next;
        }

    public:
        DynamicBlob()
        : m_data {std::make_unique<ItemT[]>(64UL)}, m_size {64UL}, m_length {0UL} {}

        DynamicBlob(std::size_t size0, ItemT filler)
        : m_data {std::make_unique<ItemT[]>(size0)}, m_size {size0}, m_length {0UL} {
            std::fill(m_data.get(), m_data.get() + m_size, filler);
        }

        template <std::same_as<std::string_view> T>
        DynamicBlob(T plain_text)
        : m_data {std::make_unique<ItemT[]>(plain_text.length())}, m_size {plain_text.length()}, m_length {m_size} {
            std::copy(plain_text.begin(), plain_text.end(), m_data.get());
        }

        const ItemT* getReadingPtr() const noexcept {
            return m_data.get();
        }

        [[nodiscard]] std::size_t getLength() const noexcept {
            return m_length;
        }

        void append(const ItemT* raw_p) {
            const ItemT* raw_it = raw_p;
            const auto end_val = ItemT {};
            ItemT temp;

            while ((temp = *raw_it) != end_val) {
                pushItem(temp);
                raw_it++;
            }
        }

        void append(const std::string& s) {
            if (std::is_same_v<ItemT, Meta::ASCIIOctet>) {
                append(s.c_str());
            }
        }
    };

    struct Request {
        HttpMethod method;
        HttpSchema schema;
        std::string uri;
        MySock::BufferView<Meta::ASCIIOctet> content_vw;
        std::unordered_map<std::string, HeaderValue> headers;
    };

    struct Response {
        HttpStatus status;
        HttpSchema schema;
        std::string_view msg;
        DynamicBlob<char> blob;
        std::unordered_map<std::string, HeaderValue> headers;
    };
}
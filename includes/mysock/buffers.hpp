#pragma once

#include <array>
#include <algorithm>
#include <string_view>
#include "meta/helpers.hpp"

namespace MyHttpd::MySock {
    template <typename OctetT> requires (Meta::is_buffer_item_v<OctetT>)
    class BufferView {
    private:
        const OctetT* m_ptr;
        std::size_t m_length;

    public:
        constexpr BufferView() noexcept
        : m_ptr {nullptr}, m_length {0UL} {}

        constexpr BufferView(const OctetT* ptr, std::size_t length) noexcept
        : m_ptr {ptr}, m_length {length} {}

        constexpr std::size_t length() const noexcept {
            return m_length;
        }

        constexpr bool operator==(std::string_view literal) noexcept {
            if constexpr (not std::is_same_v<OctetT, Meta::ASCIIOctet>) {
                return false;
            }

            const auto self_len = length();

            if (m_ptr == nullptr or self_len != literal.length()) {
                return false;
            }

            return std::equal(m_ptr, m_ptr + self_len, literal.data());
        }

        friend constexpr bool operator==(const BufferView& lhs, const BufferView& rhs) noexcept {
            const auto lhs_len = lhs.length();
            const auto rhs_len = rhs.length();

            if (lhs.m_ptr == nullptr and rhs.m_ptr == nullptr) {
                return true;
            } else if (lhs.m_ptr != nullptr or rhs.m_ptr != nullptr) {
                return false;
            }

            return lhs_len == rhs_len and std::equal(lhs.m_ptr, lhs.m_ptr + lhs_len, rhs.m_ptr);
        }
    };
    
    template <typename OctetT, std::size_t Limit> requires (Meta::is_buffer_item_v<OctetT>)
    class FixedBuffer {
    private:
        std::array<OctetT, Limit> m_data;
        std::size_t m_length;

    public:
        constexpr FixedBuffer() noexcept
        : m_data {}, m_length {0UL} {
            std::fill(m_data.begin(), m_data.end(), OctetT {});
        }

        [[nodiscard]] constexpr std::size_t getLimit() const noexcept {
            return Limit;
        }

        constexpr void markLength(std::size_t written_n) {
            m_length = written_n;
        }

        [[nodiscard]] constexpr std::size_t getLength() const noexcept {
            return m_length;
        }

        [[nodiscard]] constexpr OctetT* getPtr() & noexcept {
            return m_data.data();
        }

        [[nodiscard]] constexpr const OctetT* getPtr() const& noexcept {
            return m_data.data();
        }

        constexpr void reset() noexcept {
            std::fill(m_data.begin(), m_data.end(), OctetT {});
            m_length = 0UL;
        }

        [[nodiscard]] friend constexpr BufferView<OctetT> makeView(const FixedBuffer& buffer, std::size_t begin, std::size_t length) noexcept {
            if (begin + length > buffer.getLimit()) {
                return {};
            }

            return {buffer.m_data.data() + begin, length};
        }
    };
}
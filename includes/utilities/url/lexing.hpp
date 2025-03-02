#pragma once

#include <string_view>
#include <type_traits>

namespace MyHttpd::Utilities::Url {
    template <typename Char, typename... Rest> requires (std::is_same_v<Char, char>)
    [[nodiscard]] constexpr bool matchDisjoints(Char target, Char next, Rest... more) noexcept {
        return ((target == next) or ... or (target == more));
    }

    [[nodiscard]] constexpr bool matchLowAlpha(char c) noexcept {
        return c >= 'a' and c <= 'z';
    }

    [[nodiscard]] constexpr bool matchHighAlpha(char c) noexcept {
        return c >= 'A' and c <= 'Z';
    }

    [[nodiscard]] constexpr bool matchDigit(char c) noexcept {
        return c >= '0' and c <= '9';
    }

    [[nodiscard]] constexpr bool matchOthers(char c) noexcept {
        return matchDisjoints(c, ';', ':', '@', '&', '=');
    }

    [[nodiscard]] constexpr bool matchSafe(char c) noexcept {
        /// @note '.' is excluded despite RFC 1738; Sec. 5 requirements since path traversal is bad!
        return matchDisjoints(c, '$', '-', '_', '+');
    }

    [[nodiscard]] constexpr bool matchAlpha(char c) noexcept {
        return matchLowAlpha(c) or matchHighAlpha(c);
    }

    [[nodiscard]] constexpr bool matchNumeric(char c) noexcept {
        return matchDigit(c) or c == '.';
    }

    [[nodiscard]] constexpr bool matchWordy(char c) noexcept {
        return matchAlpha(c) or matchDigit(c) or matchSafe(c) or matchOthers(c);
    }

    enum class TokenTag : unsigned char {
        integral_num, // \d+
        float_num,    // \d*\.\d+
        wordy,        // \w(\w | \d)+
        path_split,   // '/'
        query_start,  // '?'
        query_split,  // '&'
        query_assign, // '='
        unknown,      // *
        eos
    };

    struct Token {
        int begin;
        int length;
        TokenTag tag;

        friend constexpr std::string_view toStringView(const Token& token, const std::string_view source) noexcept {
            if (token.length == 0) {
                return "";
            }

            return source.substr(token.begin, token.length);
        }

        friend std::string toFullString(const Token& token, const std::string_view source);
    };

    class Lexer {
    public:
        Lexer(const std::string& source) noexcept;

        [[nodiscard]] std::string_view viewSource() const noexcept;

        [[nodiscard]] Token lexNext() noexcept;
        [[nodiscard]] Token lexSingle(TokenTag tag) noexcept;
        [[nodiscard]] Token lexNumeric() noexcept;
        [[nodiscard]] Token lexWordy() noexcept;

    private:
        [[nodiscard]] bool atEnd() const noexcept;

        std::string_view m_source;
        int m_pos;
        int m_end;
    };
}
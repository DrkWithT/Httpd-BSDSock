#pragma once

#include <concepts>
#include "utilities/url/model.hpp"
#include "utilities/url/lexing.hpp"

namespace MyHttpd::Utilities::Url {
    enum class TokenChoice {
        previous,
        current
    };

    class Parser {
    private:
        Lexer m_lexer;
        Token m_current;
        Token m_previous;

        bool atEnd() const noexcept;

        template <TokenChoice Choice, typename Tag, typename... Rest> requires (std::same_as<Tag, TokenTag>)
        bool match(Tag next, Rest... rest) noexcept {
            if constexpr (Choice == TokenChoice::current) {
                return ((m_current.tag == next) or ... or (m_current.tag == rest));
            }

            return ((m_previous.tag == next) or ... or (m_previous.tag == rest));
        }

        Token advance() noexcept;

        template <typename... Pack>
        [[maybe_unused]] bool consume() {
            m_previous = m_current;
            m_current = advance();
            return true;
        }

        template <typename Tag, typename... Rest> requires (std::same_as<Tag, TokenTag>)
        [[maybe_unused]] bool consume(Tag tag, Rest... rest) noexcept {
            if (not match<TokenChoice::current>(tag, rest...)) {
                return false;
            }

            m_previous = m_current;
            m_current = advance();

            return true;
        }

        [[nodiscard]] std::string parsePath() noexcept;
        [[nodiscard]] QueryItem parseQueryItem() noexcept;
        [[nodiscard]] std::vector<QueryItem> parseQueryChain() noexcept;

    public:
        Parser(const std::string& source);

        /**
         * @brief Parses a subset of relative URIs tailored for HTTP messages.
         * @example URI `/foo/bar?abc=123`
         * @return URL 
         */
        [[nodiscard]] URL parseAll() noexcept;
    };
}
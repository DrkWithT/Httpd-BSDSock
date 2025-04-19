#include "utilities/url/lexing.hpp"

namespace MyHttpd::Utilities::Url {
    std::string toFullString(const Token& token, const std::string& source) {
        return source.substr(token.begin, token.length);
    }

    Lexer::Lexer(const std::string& source) noexcept
    : m_source {source}, m_pos {0}, m_end (source.size()) {}

    const std::string& Lexer::viewSource() const noexcept {
        return m_source;
    }

    Token Lexer::lexNext() noexcept {
        if (atEnd()) {
            return {
                .begin = m_pos,
                .length = 1,
                .tag = TokenTag::eos
            };
        }

        auto temp_symbol = m_source[m_pos];

        switch(temp_symbol) {
        case '/':
            return lexSingle(TokenTag::path_split);
        case '?':
            return lexSingle(TokenTag::query_start);
        case '&':
            return lexSingle(TokenTag::query_split);
        case '=':
            return lexSingle(TokenTag::query_assign);
        default:
            break;
        }

        if (matchNumeric(temp_symbol)) {
            return lexNumeric();
        } else if (matchWordy(temp_symbol)) {
            return lexWordy();
        }

        return lexSingle(TokenTag::unknown);
    }

    Token Lexer::lexSingle(TokenTag tag) noexcept {
        const auto lexeme_pos = m_pos;
        m_pos++;

        return {
            .begin = lexeme_pos,
            .length = 1,
            .tag = tag
        };
    }

    Token Lexer::lexNumeric() noexcept {
        const auto begin = m_pos;
        auto length = 0;
        auto dots = 0;

        while (not atEnd()) {
            auto temp = m_source[m_pos];

            if (not matchNumeric(temp)) {
                break;
            }

            if (temp == '.') {
                dots++;
            }

            m_pos++;
            length++;
        }

        TokenTag tag = TokenTag::unknown;

        switch(dots) {
        case 0:
            tag = TokenTag::integral_num;
            break;
        case 1:
            tag = TokenTag::float_num;
            break;
        default:
            break;
        };

        return {
            .begin = begin,
            .length = length,
            .tag = tag
        };
    }

    Token Lexer::lexWordy() noexcept {
        const auto begin = m_pos;
        auto length = 0;

        while (not atEnd()) {
            auto temp = m_source[m_pos];

            if (not matchWordy(temp)) {
                break;
            }

            m_pos++;
            length++;
        }

        return {
            .begin = begin,
            .length = length,
            .tag = TokenTag::wordy
        };
    }

    bool Lexer::atEnd() const noexcept {
        return m_pos >= m_end;
    }
}
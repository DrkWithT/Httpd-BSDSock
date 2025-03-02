#include <utility>
#include "utilities/url/parsing.hpp"

namespace MyHttpd::Utilities::Url {
    bool Parser::atEnd() const noexcept {
        return m_current.tag == TokenTag::eos;
    }

    Token Parser::advance() noexcept {
        return m_lexer.lexNext();
    }

    std::string_view Parser::parsePath() noexcept {
        if (not consume(TokenTag::path_split)) {
            return {};
        }

        auto path_start_pos = 0;
        auto path_length = 1;

        if (not match<TokenChoice::current>(TokenTag::wordy)) {
            return m_lexer.viewSource().substr(path_start_pos, path_length);
        }

        path_length += m_current.length;
        consume();

        while (not atEnd()) {
            if (not match<TokenChoice::current>(TokenTag::path_split)) {
                break;
            }

            path_length++;
            consume();

            if (not match<TokenChoice::current>(TokenTag::wordy)) {
                break;
            }

            path_length += m_current.length;
        }

        return m_lexer.viewSource().substr(path_start_pos, path_length);
    }

    QueryItem Parser::parseQueryItem() noexcept {
        if (not consume(TokenTag::wordy)) {
            return {"", Nil {}};
        }

        std::string_view key = toStringView(m_previous, m_lexer.viewSource());

        if (not consume(TokenTag::query_assign)) {
            return {key, Nil {}};
        }

        auto source_view = m_lexer.viewSource();

        if (match<TokenChoice::current>(TokenTag::integral_num)) {
            consume();

            auto lexeme = toFullString(m_previous, source_view);

            return {key, std::stoi(lexeme)};
        } else if (match<TokenChoice::current>(TokenTag::float_num)) {
            consume();

            auto lexeme = toFullString(m_previous, source_view);

            return {key, std::stof(lexeme)};
        } else if (match<TokenChoice::current>(TokenTag::wordy)) {
            consume();

            auto lexeme = toFullString(m_previous, source_view);

            return {key, std::move(lexeme)};
        } else {
            consume();
            return {"", Nil {}};
        }
    }

    std::vector<QueryItem> Parser::parseQueryChain() noexcept {
        if (not consume(TokenTag::query_start)) {
            return {};
        }

        std::vector<QueryItem> items;

        while (not atEnd()) {
            items.emplace_back(parseQueryItem());

            if (match<TokenChoice::current>(TokenTag::query_split)) {
                consume();
            }
        }

        return items;
    }

    URL Parser::parseAll() noexcept {
        return {
            .path = parsePath(),
            .query = parseQueryChain()
        };
    }
}
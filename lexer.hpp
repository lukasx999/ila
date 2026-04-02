#pragma once

#include <string>
#include <variant>
#include <print>
#include <functional>
#include <type_traits>
#include <iostream>
#include <vector>
#include <format>
#include <string_view>
#include <array>
#include <memory>

#include "token.hpp"

class lexer {
public:
    explicit lexer(std::string_view src)
    : m_src(src)
    { }

    [[nodiscard]] std::vector<token::token> tokenize() {

        using namespace std::placeholders;

        // a lex function will attempt to parse a lexeme, and return whether it
        // was actually able to parse the lexeme or not.
        using lex_fn = bool(lexer*);

        auto lex_functions = std::to_array<std::function<lex_fn>>({
            std::bind(&lexer::lex_char_novalue, _1, '\n'),
            std::bind(&lexer::lex_char_novalue, _1, ' '),
            std::bind(&lexer::lex_char<token::plus>, _1, '+'),
            std::bind(&lexer::lex_char<token::minus>, _1, '-'),
            std::bind(&lexer::lex_char<token::lbrace>, _1, '{'),
            std::bind(&lexer::lex_char<token::rbrace>, _1, '}'),
            std::bind(&lexer::lex_char<token::eq>, _1, '='),
            &lexer::lex_string,
            &lexer::lex_integer,
            &lexer::lex_identifier,
        });

        while (true) {

            if (is_at_end()) break;

            // we have to continue, because after the function call, the index might
            // be at the end of the src string, resulting in UB if another function reads
            // the char at that index.

            for (auto& fn : lex_functions) {
                if (fn(this)) break;
            }

        }

        m_idx = 0;
        return m_tokens;
    }

private:
    const std::string_view m_src;
    std::vector<token::token> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] char get_current() const {
        return m_src[m_idx];
    }

    [[nodiscard]] static bool is_identifier(char c) {
        return std::isalpha(c) || c == '_';
    }

    [[nodiscard]] bool is_at_end() const {
        return m_idx >= m_src.size();
    }

    bool lex_identifier() {

        if (is_identifier(get_current())) {

            size_t old_idx = m_idx;
            while ((is_identifier(get_current()) || std::isdigit(get_current())) && !is_at_end()) {
                m_idx++;
            }

            auto value = m_src.substr(old_idx, m_idx - old_idx);

            if (value == "fn") {
                m_tokens.push_back(token::fn());

            } else if (value == "let") {
                m_tokens.push_back(token::let());

            } else {
                m_tokens.push_back(token::identifier(std::string(value)));
            }

            return true;
        }
        return false;
    }

    bool lex_char_novalue(char c) {
        if (get_current() != c) return false;
        m_idx++;
        return true;
    }

    template <typename Token>
    bool lex_char(char c) {
        if (get_current() != c) return false;
        m_tokens.push_back(Token());
        m_idx++;
        return true;
    }

    bool lex_string() {
        if (get_current() != '"') return false;

        size_t old_idx = m_idx;
        m_idx++;
        while (get_current() != '"') {
            if (is_at_end()) {
                std::println(std::cerr, "unterminated string literal");
                std::exit(1);
            }
            m_idx++;
        }
        m_idx++;

        auto value = m_src.substr(old_idx, m_idx);
        m_tokens.push_back(token::string(std::string(value)));
        return true;
    }

    bool lex_integer() {

        if (std::isdigit(get_current())) {

            size_t old_idx = m_idx;
            while (std::isdigit(get_current()) && !is_at_end()) {
                m_idx++;
            }

            int64_t value = 0;
            std::from_chars(m_src.data() + old_idx, m_src.data() + m_idx, value);
            m_tokens.push_back(token::integer(value));

            return true;
        }

        return false;
    }

};

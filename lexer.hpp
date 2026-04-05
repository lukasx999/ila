#pragma once

#include <cassert>
#include <fstream>
#include <filesystem>
#include <string>
#include <print>
#include <functional>
#include <type_traits>
#include <iostream>
#include <vector>
#include <format>

#include "token.hpp"

class lexer_error : public std::runtime_error {
public:
    explicit lexer_error(const char* msg)
    : std::runtime_error(msg)
    { }
};

class lexer {
public:
    explicit lexer(std::string src)
    : m_src(std::move(src))
    { }

    [[nodiscard]] static lexer from_file(const std::filesystem::path& path) {
        std::ifstream file(path);
        return lexer({std::istreambuf_iterator(file), {}});
    }

    [[nodiscard]] auto tokenize() -> std::vector<token::token> {

        using namespace std::placeholders;

        // a lex function will attempt to parse a lexeme, and return whether it
        // was actually able to parse the lexeme or not.
        using lex_fn = std::function<bool(lexer*)>;

        auto lex_functions = std::to_array<lex_fn>({
            std::bind(&lexer::lex_char_ignore, _1, '\n'),
            std::bind(&lexer::lex_char_ignore, _1, ' '),
            std::bind(&lexer::lex_char_double<token::log_and>, _1, '&', '&'),
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
        auto tokens = m_tokens;
        m_tokens.clear();

        return tokens;
    }

private:
    const std::string m_src;
    std::vector<token::token> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] char get_current() const {
        assert(m_idx < m_src.length());
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
            while ((is_identifier(get_current()) || std::isdigit(get_current()))) {
                m_idx++;
                if (is_at_end()) break;
            }

            auto value = m_src.substr(old_idx, m_idx - old_idx);

            if (value == "fn") {
                m_tokens.push_back(token::fn());

            } else if (value == "let") {
                m_tokens.push_back(token::let());

            } else {
                m_tokens.push_back(token::identifier(value));
            }

            return true;
        }
        return false;
    }

    bool lex_char_ignore(char c) {
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

    template <typename Token>
    bool lex_char_double(char c1, char c2) {
        if (get_current() != c1) return false;
        if (m_idx + 1 >= m_src.length()) return false;
        if (m_src.at(m_idx + 1) != c2) return false;
        m_tokens.push_back(Token());
        m_idx += 2;
        return true;
    }

    bool lex_string() {
        // TODO: clean this up
        if (get_current() != '"') return false;

        size_t old_idx = m_idx;
        m_idx++;
        while (get_current() != '"') {
            m_idx++;
            if (is_at_end()) throw lexer_error("unterminated string literal");
        }
        m_idx++;

        auto value = m_src.substr(old_idx, m_idx);
        m_tokens.push_back(token::string(std::string(value)));
        return true;
    }

    bool lex_integer() {

        if (std::isdigit(get_current())) {

            size_t old_idx = m_idx;
            while (std::isdigit(get_current())) {
                m_idx++;
                if (is_at_end()) break;
            }

            int64_t value = 0;
            std::from_chars(m_src.data() + old_idx, m_src.data() + m_idx, value);
            m_tokens.push_back(token::integer(value));

            return true;
        }

        return false;
    }

};

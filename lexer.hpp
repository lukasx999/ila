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

struct literal_integer {
    const int64_t m_value;
};

struct literal_string {
    const std::string_view m_value;
};

struct identifier {
    const std::string_view m_value;
};

struct operator_plus { };
struct operator_minus { };
struct punct_lbrace { };
struct punct_rbrace { };
struct keyword_function { };

using token = std::variant<
    literal_integer,
    literal_string,
    identifier,
    operator_plus,
    operator_minus,
    punct_lbrace,
    punct_rbrace,
    keyword_function
>;

struct token_formatter {
    std::string operator()(const literal_integer& token) const {
        return std::format("int: {}", token.m_value);
    }

    std::string operator()(const literal_string& token) const {
        return std::format("string: {}", token.m_value);
    }

    std::string operator()(const identifier& token) const {
        return std::format("ident: {}", token.m_value);
    }

    std::string operator()(const operator_plus&) const {
        return "plus";
    }

    std::string operator()(const operator_minus&) const {
        return "minus";
    }

    std::string operator()(const punct_lbrace&) const {
        return "lbrace";
    }

    std::string operator()(const punct_rbrace&) const {
        return "rbrace";
    }

    std::string operator()(const keyword_function&) const {
        return "function";
    }
};

template <typename... Ts>
struct overloaded_lambda : Ts... {
    using Ts::operator()...;
};

class lexer {
public:
    explicit lexer(std::string_view src)
    : m_src(src)
    { }

    [[nodiscard]] std::vector<token> tokenize() {

        using namespace std::placeholders;

        // a parse function will attempt to parse a lexeme, and return whether it
        // was actually able to parse the lexeme or not.
        using parse_fn = bool(lexer*);

        auto parse_functions = std::to_array<std::function<parse_fn>>({
            std::bind(&lexer::try_parse_char_novalue, _1, '\n'),
            std::bind(&lexer::try_parse_char_novalue, _1, ' '),
            std::bind(&lexer::try_parse_char<operator_plus>, _1, '+'),
            std::bind(&lexer::try_parse_char<operator_minus>, _1, '-'),
            std::bind(&lexer::try_parse_char<punct_lbrace>, _1, '{'),
            std::bind(&lexer::try_parse_char<punct_rbrace>, _1, '}'),
            &lexer::try_parse_string,
            &lexer::try_parse_integer,
            &lexer::try_parse_identifier,
        });

        while (true) {

            if (is_at_end()) break;

            // we have to continue, because after the function call, the index might
            // be at the end of the src string, resulting in UB if another function reads
            // the char at that index.

            for (auto& fn : parse_functions) {
                if (fn(this)) break;
            }

        }

        m_idx = 0;
        return m_tokens;
    }

private:
    const std::string_view m_src;
    std::vector<token> m_tokens;
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

    bool try_parse_identifier() {

        if (is_identifier(get_current())) {

            size_t old_idx = m_idx;
            while ((is_identifier(get_current()) || std::isdigit(get_current())) && !is_at_end()) {
                m_idx++;
            }

            auto value = m_src.substr(old_idx, m_idx - old_idx);

            if (value == "subroutine") {
                m_tokens.push_back(keyword_function{});
            } else {
                m_tokens.push_back(identifier{value});
            }

            return true;
        }
        return false;
    }

    bool try_parse_char_novalue(char c) {
        if (get_current() != c) return false;
        m_idx++;
        return true;
    }

    template <typename Token>
    bool try_parse_char(char c) {
        if (get_current() != c) return false;
        m_tokens.push_back(Token{});
        m_idx++;
        return true;
    }

    bool try_parse_string() {
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
        m_tokens.push_back(literal_string{value});
        return true;
    }

    bool try_parse_integer() {

        if (std::isdigit(get_current())) {

            size_t old_idx = m_idx;
            while (std::isdigit(get_current()) && !is_at_end()) {
                m_idx++;
            }

            int64_t value = 0;
            std::from_chars(m_src.data() + old_idx, m_src.data() + m_idx, value);
            m_tokens.push_back(literal_integer{value});

            return true;
        }

        return false;
    }

};

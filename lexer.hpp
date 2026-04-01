#pragma once

#include <string>
#include <functional>
#include <type_traits>
#include <iostream>
#include <vector>
#include <format>
#include <string_view>
#include <array>
#include <memory>

class token {
public:
    token() = default;
    virtual ~token() = default;

    [[nodiscard]] virtual std::string fmt() const = 0;

private:
};

class literal_integer : public token {
public:
    explicit literal_integer(int64_t value)
    : m_value(value)
    { }

    [[nodiscard]] std::string fmt() const override {
        return std::format("int: {}", m_value);
    }

private:
    int64_t m_value;
};

class literal_string : public token {
public:
    explicit literal_string(std::string_view value)
    : m_value(value)
    { }

    [[nodiscard]] std::string fmt() const override {
        return std::format("string: {}", m_value);
    }

private:
    std::string_view m_value;
};

class identifier : public token {
public:
    explicit identifier(std::string_view value)
    : m_value(value)
    { }

    [[nodiscard]] std::string fmt() const override {
        return std::format("ident: {}", m_value);
    }

private:
    std::string_view m_value;
};

class operator_plus : public token {
public:
    operator_plus() = default;

    [[nodiscard]] std::string fmt() const override {
        return "plus";
    }

};

class operator_minus : public token {
public:
    operator_minus() = default;

    [[nodiscard]] std::string fmt() const override {
        return "minus";
    }

};

class lexer {
public:
    explicit lexer(std::string_view src)
    : m_src(src)
    { }

    [[nodiscard]] std::vector<std::unique_ptr<token>> tokenize() {

        using namespace std::placeholders;

        // a parse function will attempt to parse a lexeme, and return whether it
        // was actually able to parse the lexeme or not.
        using parse_fn = bool(lexer*);

        auto parse_functions = std::to_array<std::function<parse_fn>>({
            std::bind(&lexer::try_parse_char_novalue, _1, '\n'),
            std::bind(&lexer::try_parse_char_novalue, _1, ' '),
            std::bind(&lexer::try_parse_char<operator_plus>, _1, '+'),
            std::bind(&lexer::try_parse_char<operator_minus>, _1, '-'),
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
        return std::move(m_tokens);
    }

private:
    std::string_view m_src;
    std::vector<std::unique_ptr<token>> m_tokens;
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
            m_tokens.push_back(std::make_unique<identifier>(value));
            return true;
        }
        return false;
    }

    bool try_parse_char_novalue(char c) {
        if (get_current() != c) return false;
        m_idx++;
        return true;
    }

    template <typename Token> requires std::is_base_of_v<token, Token>
    bool try_parse_char(char c) {
        if (get_current() != c) return false;
        m_tokens.push_back(std::make_unique<Token>());
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
        m_tokens.push_back(std::make_unique<literal_string>(value));
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
            m_tokens.push_back(std::make_unique<literal_integer>(value));

            return true;
        }

        return false;
    }

};


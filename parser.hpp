#pragma once

#include <string>
#include <span>

#include "lexer.hpp"

class ast_node {
public:
    ast_node() = default;
    virtual ~ast_node() = default;
};

class literal : public ast_node {
public:
    explicit literal(token token)
    : m_token(std::move(token))
    { }

private:
    token m_token;

};

class binary_operation : public ast_node {
public:
    enum class type {
        plus,
        minus,
    };

    binary_operation(type type, std::unique_ptr<ast_node> lhs, std::unique_ptr<ast_node> rhs)
        : m_type(type)
        , m_lhs(std::move(lhs))
        , m_rhs(std::move(rhs))
    { }

private:
    const type m_type;
    std::unique_ptr<ast_node> m_lhs;
    std::unique_ptr<ast_node> m_rhs;

};

class parser {
public:
    explicit parser(std::span<token> tokens)
    : m_tokens(tokens)
    { }

    [[nodiscard]] std::unique_ptr<ast_node> parse() {
        auto node = parse_binop();
        m_idx = 0;
        return node;
    }

private:
    std::span<token> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] std::unique_ptr<ast_node> parse_expression() {
        return parse_binop();
    }

    [[nodiscard]] std::unique_ptr<ast_node> parse_binop() {

        std::unique_ptr<ast_node> node = parse_atom();

        while (true) {

            binary_operation::type type;

            if (m_idx >= m_tokens.size()) break;

            auto token = m_tokens[m_idx];

            bool should_stop = std::visit(overloaded_lambda {
                [&](const operator_plus& token) {
                    type = binary_operation::type::plus;
                    return false;
                },

                [&](const operator_minus& token) {
                    type = binary_operation::type::minus;
                    return false;
                },

                [](const auto&) {
                    return true;
                },
            }, token);

            if (should_stop) break;

            next_token();

            node = std::make_unique<binary_operation>(type, std::move(node), parse_atom());
        }

        return node;
    }

    [[nodiscard]] std::unique_ptr<ast_node> parse_atom() {
        auto node = std::make_unique<literal>(m_tokens[m_idx]);
        next_token();
        return node;
    }

    void next_token() {
        m_idx++;
    }

};

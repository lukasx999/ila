#pragma once

#include <string>

#include "lexer.hpp"

class ast_node {
public:
    ast_node() = default;
    virtual ~ast_node() = default;
};

class literal : public ast_node {
public:
    explicit literal(std::unique_ptr<token> token)
    : m_token(std::move(token))
    { }

private:
    std::unique_ptr<token> m_token;

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
    explicit parser(std::vector<std::unique_ptr<token>> tokens)
    : m_tokens(std::move(tokens))
    { }

    [[nodiscard]] std::unique_ptr<ast_node> parse() {
        auto node = parse_binop();
        m_idx = 0;
        return node;
    }

private:
    std::vector<std::unique_ptr<token>> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] std::unique_ptr<ast_node> parse_expression() {
        return parse_binop();
    }

    [[nodiscard]] std::unique_ptr<ast_node> parse_binop() {

        std::unique_ptr<ast_node> node = parse_atom();

        while (true) {

            binary_operation::type type;

            if (m_idx >= m_tokens.size()) break;

            if (dynamic_cast<operator_plus*>(m_tokens.at(m_idx).get()) != nullptr) {
                type = binary_operation::type::plus;

            } else if (dynamic_cast<operator_minus*>(m_tokens.at(m_idx).get()) != nullptr) {
                type = binary_operation::type::minus;

            } else {
                break;
            }

            next_token();

            node = std::make_unique<binary_operation>(type, std::move(node), parse_atom());
        }

        return node;
    }

    [[nodiscard]] std::unique_ptr<ast_node> parse_atom() {
        auto node = std::make_unique<literal>(std::move(m_tokens[m_idx]));
        next_token();
        return node;
    }

    void next_token() {
        m_idx++;
    }

};

#pragma once

#include <string>
#include <span>

#include "lexer.hpp"
#include "utils.hpp"
#include "ast.hpp"

class parser {
public:
    explicit parser(std::span<token::token> tokens)
    : m_tokens(tokens)
    { }

    [[nodiscard]] std::unique_ptr<ast::node> parse() {
        auto node = parse_binop();
        m_idx = 0;
        return node;
    }

private:
    const std::span<token::token> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] std::unique_ptr<ast::node> parse_expression() {
        return parse_binop();
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_binop() {

        std::unique_ptr<ast::node> node = parse_atom();

        while (true) {

            ast::bin_op::type type;

            if (m_idx >= m_tokens.size()) break;

            auto token = m_tokens[m_idx];

            bool should_stop = std::visit(overloaded_lambda {
                [&](const token::plus&) {
                    type = ast::bin_op::type::plus;
                    return false;
                },

                [&](const token::minus&) {
                    type = ast::bin_op::type::minus;
                    return false;
                },

                [](const auto&) {
                    return true;
                },
            }, token);

            if (should_stop) break;

            next_token();
            node = std::make_unique<ast::bin_op>(type, std::move(node), parse_atom());
        }

        return node;
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_atom() {
        auto node = std::make_unique<ast::literal>(m_tokens[m_idx]);
        next_token();
        return node;
    }

    void next_token() {
        m_idx++;
    }

};

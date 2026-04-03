#pragma once

#include <cassert>
#include <string>
#include <span>
#include <memory>

#include "lexer.hpp"
#include "utils.hpp"
#include "ast.hpp"

class parser {
public:
    explicit parser(std::span<token::token> tokens)
    : m_tokens(tokens)
    { }

    [[nodiscard]] std::unique_ptr<ast::node> parse() {
        auto node = parse_script();
        m_idx = 0;
        return node;
    }

private:
    const std::span<token::token> m_tokens;
    size_t m_idx = 0;

    [[nodiscard]] std::unique_ptr<ast::node> parse_script() {
        std::vector<std::unique_ptr<ast::node>> children;

        while (!is_at_end()) {

            if (token_isa<token::let>()) {
                children.push_back(parse_var_decl());
            }

            children.push_back(parse_binop());
        }

        return std::make_unique<ast::block>(std::move(children));
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_expression() {
        return parse_binop();
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_binop() {

        std::unique_ptr<ast::node> node = parse_literal();

        while (true) {

            ast::binary_op::type type;

            if (m_idx >= m_tokens.size()) break;

            bool should_stop = std::visit(overloaded_lambda {
                [&](const token::plus&) {
                    type = ast::binary_op::type::plus;
                    return false;
                },

                [&](const token::minus&) {
                    type = ast::binary_op::type::minus;
                    return false;
                },

                [](const auto&) {
                    return true;
                },
            }, get_token());

            if (should_stop) break;

            next_token();
            node = std::make_unique<ast::binary_op>(type, std::move(node), parse_literal());
        }

        return node;
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_literal() {
        auto node = std::make_unique<ast::literal>(get_token());
        next_token();
        return node;
    }

    [[nodiscard]] std::unique_ptr<ast::node> parse_var_decl() {
        assert_token<token::let>();
        next_token();

        assert_token<token::identifier>();
        auto ident = get_token_as<token::identifier>();
        next_token();

        assert_token<token::eq>();
        next_token();

        auto init = parse_expression();

        return std::make_unique<ast::var_decl>(ident, std::move(init));
    }

    template <typename Token>
    [[nodiscard]] bool token_isa() const {
        return std::holds_alternative<Token>(get_token());
    }

    template <typename Token>
    void assert_token() const {
        assert(std::holds_alternative<Token>(get_token()));
    }

    template <typename Token>
    [[nodiscard]] Token get_token_as() {
        return std::get<Token>(get_token());
    }

    [[nodiscard]] token::token& get_token() const {
        return m_tokens[m_idx];
    }

    [[nodiscard]] bool is_at_end() const {
        return m_idx >= m_tokens.size();
    }

    void next_token() {
        m_idx++;
    }

};

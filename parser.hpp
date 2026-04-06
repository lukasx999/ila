#pragma once

#include <cassert>
#include <string>
#include <span>
#include <memory>

#include "lexer.hpp"
#include "utils.hpp"
#include "ast.hpp"

struct parse_error : std::runtime_error {
    explicit parse_error(const char* msg)
    : std::runtime_error(msg)
    { }

    explicit parse_error(std::string msg)
    : std::runtime_error(std::move(msg))
    { }
};

class parser {
public:
    explicit parser(std::vector<token::token> tokens)
    : m_tokens(tokens)
    { }

    [[nodiscard]] std::unique_ptr<ast::node> parse() {
        auto node = parse_script();
        m_idx = 0;
        return node;
    }

private:
    const std::vector<token::token> m_tokens;
    size_t m_idx = 0;

    std::unique_ptr<ast::node> parse_script() {
        std::vector<std::unique_ptr<ast::node>> children;

        while (!is_at_end()) {

            if (auto vardecl = parse_var_decl()) {
                children.push_back(std::move(vardecl));

            } else if (auto function = parse_function()) {
                children.push_back(std::move(function));

            } else {
                children.push_back(parse_expression());
            }

        }

        ast::block block(std::move(children));
        return std::make_unique<ast::node>(std::move(block));
    }

    std::unique_ptr<ast::node> parse_expression() {
        return parse_call();
    }

    std::unique_ptr<ast::node> parse_binop() {

        std::unique_ptr<ast::node> node = parse_literal();

        while (true) {

            ast::binary_op::type type;

            bool should_stop = get_token().match(
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
                }
            );

            if (should_stop) break;

            next_token();
            ast::binary_op binop(type, std::move(node), parse_literal());
            node = std::make_unique<ast::node>(std::move(binop));
        }

        return node;
    }

    std::unique_ptr<ast::node> parse_literal() {
        auto node = std::make_unique<ast::node>(ast::literal(get_token()));
        next_token();
        return node;
    }

    std::unique_ptr<ast::node> parse_function() {
        if (!get_token().isa<token::fn>())
            return nullptr;
        next_token();

        token_must_be<token::identifier>();
        auto ident = get_token().get_as<token::identifier>();
        next_token();

        token_must_be<token::lparen>();
        next_token();

        token_must_be<token::rparen>();
        next_token();

        token_must_be<token::lparen>();
        next_token();

        token_must_be<token::rparen>();
        next_token();

    }

    std::unique_ptr<ast::node> parse_call() {
        auto expr = parse_binop();

        if (get_token().isa<token::lparen>()) {
            next_token();

            std::vector<std::unique_ptr<ast::node>> args;

            while (!get_token().isa<token::rparen>()) {
                args.push_back(parse_expression());

                if (get_token().isa<token::comma>())
                    next_token();
            }

            next_token();
            ast::call call(std::move(expr), std::move(args));
            return std::make_unique<ast::node>(std::move(call));
        }

        return expr;
    }

    std::unique_ptr<ast::node> parse_var_decl() {
        if (!get_token().isa<token::let>()) return nullptr;

        next_token();

        token_must_be<token::identifier>();
        auto ident = get_token().get_as<token::identifier>();
        next_token();

        token_must_be<token::eq>();
        next_token();

        auto init = parse_expression();

        return std::make_unique<ast::node>(ast::var_decl(ident, std::move(init)));
    }

    template <class Token>
    void token_must_be() const {
        if (!get_token().isa<Token>())
            throw parse_error(std::format("expected {}", token::type_to_string<Token>()));
    }

    [[nodiscard]] const token::token& get_token() const {
        return m_tokens.at(m_idx);
    }

    [[nodiscard]] bool is_at_end() const {
        return get_token().isa<token::terminator>();
    }

    void next_token() {
        m_idx++;
    }

};

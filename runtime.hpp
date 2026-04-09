#pragma once

#include <stdexcept>
#include <unordered_map>
#include <stack>
#include <ranges>

#include "ast.hpp"
#include "value.hpp"

struct value_error : std::runtime_error {
    explicit value_error(const char* msg)
    : std::runtime_error(msg)
    { }
};

class runtime_visitor {
public:
    runtime_visitor() = default;

    value::value operator()(const ast::literal& literal) {
        auto token = literal.get_token();

        try {
            auto ident = token.get_as<token::identifier>().m_value;
            try {
                return m_variables.top().at(ident);
            } catch (const std::out_of_range&) {
                throw value_error("variable doesnt exist");
            }

        } catch (const std::bad_variant_access&) {
            return value::from_token(token);
        }

    }

    value::value operator()(const ast::binary_op& binop) {

        auto lhs = std::visit(*this, binop.get_lhs());
        auto rhs = std::visit(*this, binop.get_rhs());

        switch (binop.get_type()) {
            using enum ast::binary_op::type;

            case plus:  return lhs + rhs;
            case minus: return lhs - rhs;
        }

        assert(!"unreachable");
    }

    value::value operator()(const ast::call& call) {

        auto& callee = call.get_callee();

        auto args = call.get_arguments() | std::views::transform([&](const std::unique_ptr<ast::node>& arg) {
            return std::visit(*this, *arg);
        });

        if (callee.isa<ast::literal>()) {

            auto token = callee.get_as<ast::literal>().get_token();

            if (token.isa<token::identifier>()) {
                auto& name = token.get_as<token::identifier>().m_value;

                if (name == "print") {

                    for (auto&& arg : args) {
                        std::print("{}", arg.to_string());
                    }
                    std::println();
                    return value::null();

                }
            }
        }

        auto fn = std::visit(*this, call.get_callee());
        if (!fn.isa<value::function>())
            throw value::type_error("callee is not a function");

        auto& fn_node = fn.get_as<value::function>().get_node();
        (*this)(fn_node);

        return value::null();
    }

    value::value operator()(const ast::var_decl& vardecl) {
        auto init = std::visit(*this, vardecl.get_init());
        m_variables.top().insert({ std::string(vardecl.get_identifier()), init });
        return value::null();
    }

    value::value operator()(const ast::block& block) {
        m_variables.push({});

        for (auto& child : block.get_children()) {
            std::visit(*this, *child);
        }
        m_variables.pop();

        return value::null();
    }

    value::value operator()(const ast::function& function) {
        auto ident = function.get_identifier().m_value;
        value::function fn(function.get_body().get_as<ast::block>());
        m_variables.top().insert({ std::move(ident), fn });
        return value::null();
    }

private:
    std::stack<std::unordered_map<std::string, value::value>> m_variables;

};

class runtime {
public:
    runtime() = default;

    value::value run_tree(const ast::node& root) {
        return std::visit(m_visitor, root);
    }

private:
    runtime_visitor m_visitor;

};

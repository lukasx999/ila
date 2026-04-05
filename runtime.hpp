#pragma once

#include <unordered_map>

#include "ast.hpp"
#include "value.hpp"

class runtime_visitor {
public:
    runtime_visitor() = default;

    value::value operator()(const ast::literal& literal) {
        auto token = literal.get_token();

        try {
            auto ident = token.get_as<token::identifier>().m_value;
            return m_variables.at(ident);

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
    }

    value::value operator()(const ast::var_decl& vardecl) {
        auto init = std::visit(*this, vardecl.get_init());
        m_variables.insert({ vardecl.get_identifier(), init });
        return value::null();
    }

    value::value operator()(const ast::block& block) {

        for (auto& child : block.get_children()) {
            std::visit(*this, *child);
        }

        return value::null();
    }

private:
    std::unordered_map<std::string_view, value::value> m_variables;

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

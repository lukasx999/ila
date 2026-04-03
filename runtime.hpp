#pragma once

#include <unordered_map>

#include "ast.hpp"
#include "value.hpp"

class runtime {
public:
    runtime() = default;

    value::value operator()(const ast::literal& literal) {
        auto token = literal.get_token();

        if (std::holds_alternative<token::identifier>(token)) {
            auto ident = std::get<token::identifier>(token).m_value;
            return m_variables.at(ident);
        };

        return value::from_token(token);
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

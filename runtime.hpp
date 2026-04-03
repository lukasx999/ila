#pragma once

#include "ast.hpp"
#include "value.hpp"

class runtime {
public:
    runtime() = default;

    value::value operator()(const ast::literal& literal) {
        return value::from_token(literal.get_token());
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
        return value::null();
    }

    value::value operator()(const ast::block& block) {
        if (block.get_children().size() == 1)
            return std::visit(*this, *block.get_children().front());


        for (auto& child : block.get_children()) {
            std::visit(*this, *child);
        }

        return value::null();
    }

};

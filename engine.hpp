#pragma once

#include <cassert>

#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

class engine : public ast::node_visitor {
public:
    engine() = default;

    void visit_literal(ast::literal& lit) override {
        auto value = value::from_token(lit.get_token());
        m_values.push_back(value);
    }

    void visit_binary_operation(ast::bin_op& binop) override {
        binop.get_lhs().apply_visitor(*this);
        binop.get_rhs().apply_visitor(*this);

        auto lhs = m_values.back();
        m_values.pop_back();

        auto rhs = m_values.back();
        m_values.pop_back();

        // m_values.push_back(lhs + rhs);
    }

    [[nodiscard]] value::value get_value() const {
        assert(m_values.size() == 1);
        return m_values.back();
    }

private:
    std::vector<value::value> m_values;

};

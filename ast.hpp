#pragma once

#include <memory>
#include <variant>
#include <vector>
#include <print>

#include "token.hpp"

namespace ast {

struct node;

class literal {
public:
    explicit literal(token::token token)
    : m_token(std::move(token))
    { }

    [[nodiscard]] const token::token& get_token() const {
        return m_token;
    }

private:
    token::token m_token;

};

class binary_op {
public:
    enum class type {
        plus,
        minus,
    };

    binary_op(type type, std::unique_ptr<node> lhs, std::unique_ptr<node> rhs)
        : m_type(type)
        , m_lhs(std::move(lhs))
        , m_rhs(std::move(rhs))
    { }

    [[nodiscard]] node& get_lhs() {
        return *m_lhs;
    }

    [[nodiscard]] node& get_rhs() {
        return *m_rhs;
    }

    [[nodiscard]] const node& get_lhs() const {
        return *m_lhs;
    }

    [[nodiscard]] const node& get_rhs() const {
        return *m_rhs;
    }

    [[nodiscard]] type get_type() const {
        return m_type;
    }

private:
    const type m_type;
    std::unique_ptr<node> m_lhs;
    std::unique_ptr<node> m_rhs;

};

class var_decl {
public:
    var_decl(token::identifier ident, std::unique_ptr<node> init)
        : m_ident(ident)
        , m_init(std::move(init))
    { }

    [[nodiscard]] std::string_view get_identifier() const {
        return m_ident.m_value;
    }

    [[nodiscard]] node& get_init() const {
        return *m_init;
    }

private:
    token::identifier m_ident;
    std::unique_ptr<node> m_init;

};

class block {
public:
    explicit block(std::vector<std::unique_ptr<node>> children)
    : m_children(std::move(children))
    { }

    [[nodiscard]] auto get_children() const -> const std::vector<std::unique_ptr<node>>& {
        return m_children;
    }

private:
    std::vector<std::unique_ptr<node>> m_children;

};

using node_variant_type = std::variant<literal, binary_op, var_decl, block>;

struct node : node_variant_type {
    using node_variant_type::variant;
};

struct node_formatter {
    void operator()(literal& lit) {
        print_spacing();
        std::println("lit: {}", token::to_string(lit.get_token()));
    }

    void operator()(binary_op& binop) {
        print_spacing();
        std::println("binop");

        int old_spacing = m_spacing;

        m_spacing++;
        std::visit(*this, binop.get_lhs());

        m_spacing = old_spacing + 1;
        std::visit(*this, binop.get_rhs());
    }

    void operator()(var_decl& decl) {
        print_spacing();
        std::println("vardecl");
        m_spacing++;
        std::visit(*this, decl.get_init());
    }

    void operator()(block& block) {
        print_spacing();
        std::println("block");
        m_spacing++;
        for (auto& child : block.get_children()) {
            std::visit(*this, *child);
        }
    }

private:
    int m_spacing = 0;

    void print_spacing() const {
        for (int i = 0; i < m_spacing; ++i) {
            std::print(" ");
        }
    }

};

} // namespace ast

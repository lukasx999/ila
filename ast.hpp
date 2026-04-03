#pragma once

#include "lexer.hpp"

namespace ast {

class literal;
class binary_op;
class var_decl;

struct node_visitor {
    virtual ~node_visitor() = default;

    virtual void visit_literal(literal&) = 0;
    virtual void visit_binary_op(binary_op&) = 0;
    virtual void visit_var_decl(var_decl&) = 0;
};

class node {
public:
    node() = default;
    virtual ~node() = default;

    virtual void apply_visitor(node_visitor& visitor) = 0;
};

class literal : public node {
public:
    explicit literal(token::token token)
    : m_token(std::move(token))
    { }

    void apply_visitor(node_visitor& visitor) override {
        visitor.visit_literal(*this);
    }

    [[nodiscard]] const token::token& get_token() const {
        return m_token;
    }

private:
    token::token m_token;

};

class binary_op : public node {
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

    void apply_visitor(node_visitor& visitor) override {
        visitor.visit_binary_op(*this);
    }

    [[nodiscard]] node& get_lhs() {
        return *m_lhs;
    }

    [[nodiscard]] node& get_rhs() {
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

class var_decl : public node {
public:
    explicit var_decl(token::identifier ident, std::unique_ptr<node> init)
        : m_ident(ident)
        , m_init(std::move(init))
    { }

    void apply_visitor(node_visitor& visitor) override {
        visitor.visit_var_decl(*this);
    }

    [[nodiscard]] node& get_init() const {
        return *m_init;
    }

private:
    token::identifier m_ident;
    std::unique_ptr<node> m_init;

};

class node_formatter : public node_visitor {
public:
    node_formatter() = default;

    void visit_literal(literal& lit) override {
        print_spacing();
        std::println("lit: {}", std::visit(token::formatter{}, lit.get_token()));
    }

    void visit_binary_op(binary_op& binop) override {
        print_spacing();
        std::println("binop");

        int old_spacing = m_spacing;

        m_spacing++;
        binop.get_lhs().apply_visitor(*this);

        m_spacing = old_spacing + 1;
        binop.get_rhs().apply_visitor(*this);
    }

    void visit_var_decl(var_decl& decl) override {
        print_spacing();
        std::println("vardecl");
        m_spacing++;
        decl.get_init().apply_visitor(*this);
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

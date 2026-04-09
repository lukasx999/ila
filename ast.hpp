#pragma once

#include <memory>
#include <variant>
#include <ranges>
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

    [[nodiscard]] auto get_children() const -> std::span<const std::unique_ptr<node>> {
        return m_children;
    }

private:
    std::vector<std::unique_ptr<node>> m_children;

};

class function {
    public:
    function(token::identifier identifier, std::vector<token::identifier> parameters, std::unique_ptr<node> body)
    : m_identifier(identifier)
    , m_parameters(std::move(parameters))
    , m_body(std::move(body))
    { }

    [[nodiscard]] const token::identifier& get_identifier() const {
        return m_identifier;
    }

    [[nodiscard]] auto get_parameters() const -> std::span<const token::identifier> {
        return m_parameters;
    }

    [[nodiscard]] node& get_body() {
        return *m_body;
    }

    [[nodiscard]] const node& get_body() const {
        return *m_body;
    }

    private:
    token::identifier m_identifier;
    std::vector<token::identifier> m_parameters;
    std::unique_ptr<node> m_body;

};

class call {
public:
    call(std::unique_ptr<node> callee, std::vector<std::unique_ptr<node>> arguments)
        : m_callee(std::move(callee))
        , m_arguments(std::move(arguments))
    { }

    [[nodiscard]] node& get_callee() {
        return *m_callee;
    }

    [[nodiscard]] const node& get_callee() const {
        return *m_callee;
    }

    [[nodiscard]] auto get_arguments() const -> const std::vector<std::unique_ptr<node>>& {
        return m_arguments;
    }

private:
    std::unique_ptr<node> m_callee;
    std::vector<std::unique_ptr<node>> m_arguments;

};

using node_variant_type = variant<literal, binary_op, call, var_decl, function, block>;

struct node : node_variant_type {
    using node_variant_type::variant;
};

struct tree_print_visitor {
    void operator()(const literal& lit) {
        print_spacing();
        std::println("lit: {}", token::to_string(lit.get_token()));
    }

    void operator()(const binary_op& binop) {
        print_spacing();
        std::println("binop");

        int old_spacing = m_spacing;

        m_spacing++;
        std::visit(*this, binop.get_lhs());

        m_spacing = old_spacing + 1;
        std::visit(*this, binop.get_rhs());
    }

    void operator()(const var_decl& decl) {
        print_spacing();
        std::println("vardecl");
        m_spacing++;
        std::visit(*this, decl.get_init());
    }

    void operator()(const block& block) {
        print_spacing();
        std::println("block");
        m_spacing++;
        for (auto& child : block.get_children()) {
            std::visit(*this, *child);
        }
    }

    void operator()(const call& call) {
        print_spacing();
        std::println("call");
        m_spacing++;
        for (auto& child : call.get_arguments()) {
            std::visit(*this, *child);
        }
    }

    void operator()(const function& function) {
        print_spacing();
        auto ident = function.get_identifier().m_value;
        std::print("function {}(", ident);

        auto params = function.get_parameters();
        if (!params.empty()) {

            for (auto& param : params | std::views::take(params.size() - 1)) {
                std::print("{}, ", param.m_value);
            }
            std::print("{}", params.back().m_value);
        }
        std::println(")");

        m_spacing++;
        std::visit(*this, function.get_body());
    }

private:
    int m_spacing = 0;

    void print_spacing() const {
        for (int i = 0; i < m_spacing; ++i) {
            std::print(" ");
        }
    }

};

inline void print_tree(const node& root) {
    std::visit(tree_print_visitor{}, root);
}

} // namespace ast

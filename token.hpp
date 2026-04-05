#pragma once

#include <variant>
#include <cstdint>
#include <string_view>
#include <string>
#include <format>

#include "variant.hpp"

namespace token {

struct integer {
    const int64_t m_value;
};

struct string {
    const std::string m_value;
};

struct identifier {
    const std::string m_value;
};

struct plus { };
struct minus { };
struct lbrace { };
struct rbrace { };
struct eq { };
struct log_and { };
struct fn { };
struct let { };

using token_variant_type = variant<integer, string, identifier,
                            plus, minus, lbrace, rbrace, eq, log_and, fn, let>;

struct token : token_variant_type {
    using token_variant_type::variant;
};

struct format_visitor {
    std::string operator()(const integer& token) const {
        return std::format("int({})", token.m_value);
    }

    std::string operator()(const string& token) const {
        return std::format("string(\"{}\")", token.m_value);
    }

    std::string operator()(const identifier& token) const {
        return std::format("ident({})", token.m_value);
    }

    std::string operator()(const plus&) const {
        return "plus";
    }

    std::string operator()(const minus&) const {
        return "minus";
    }

    std::string operator()(const lbrace&) const {
        return "lbrace";
    }

    std::string operator()(const rbrace&) const {
        return "rbrace";
    }

    std::string operator()(const eq&) const {
        return "equals";
    }

    std::string operator()(const log_and&) const {
        return "log_and";
    }

    std::string operator()(const fn&) const {
        return "fn";
    }

    std::string operator()(const let&) const {
        return "let";
    }
};

[[nodiscard]] inline std::string to_string(const token& token) {
    return std::visit(format_visitor{}, token);
}

template <class Token>
[[nodiscard]] inline std::string_view type_to_string() {
    if constexpr (std::is_same_v<Token, integer>) return "integer";
    else if constexpr (std::is_same_v<Token, string>) return "string";
    else if constexpr (std::is_same_v<Token, identifier>) return "identifier";
    else if constexpr (std::is_same_v<Token, plus>) return "plus";
    else if constexpr (std::is_same_v<Token, minus>) return "minus";
    else if constexpr (std::is_same_v<Token, lbrace>) return "lbrace";
    else if constexpr (std::is_same_v<Token, rbrace>) return "rbrace";
    else if constexpr (std::is_same_v<Token, eq>) return "eq";
    else if constexpr (std::is_same_v<Token, log_and>) return "log_and";
    else if constexpr (std::is_same_v<Token, fn>) return "fn";
    else if constexpr (std::is_same_v<Token, let>) return "let";
}

} // namespace token

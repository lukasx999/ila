#pragma once

#include <variant>
#include <cstdint>
#include <string_view>
#include <string>
#include <format>

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

using token = std::variant<integer, string, identifier,
                           plus, minus, lbrace, rbrace, eq, log_and, fn, let>;

struct formatter {
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

[[nodiscard]] inline std::string format(const token& token) {
    return std::visit(formatter{}, token);
}

} // namespace token

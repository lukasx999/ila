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
struct function { };

using token = std::variant<
integer,
string,
identifier,
plus,
minus,
lbrace,
rbrace,
function
>;

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

    std::string operator()(const function&) const {
        return "function";
    }
};

[[nodiscard]] inline std::string format(const token& token) {
    return std::visit(formatter{}, token);
}

} // namespace token

// TODO: this?
// template <>
// struct std::formatter<token::integer> : std::formatter<std::string> {
//     auto format(const token::integer& integer, std::format_context& ctx) const {
//         auto fmt =  std::format("int({})", integer.m_value);
//         return std::formatter<std::string>::format(fmt, ctx);
//     }
// };

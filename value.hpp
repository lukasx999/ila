#pragma once

#include <cassert>
#include <string>
#include <type_traits>
#include <concepts>
#include <cstring>

#include "lexer.hpp"
#include "utils.hpp"
#include "variant.hpp"

namespace value {

class null {
public:
    null() = default;
};

class integer {
public:
    integer(int64_t value)
    : m_value(value)
    { }

    [[nodiscard]] int64_t get() const {
        return m_value;
    }

    operator int64_t() const {
        return m_value;
    }

private:
    int64_t m_value;

};

class string {
public:
    string(std::string value)
    : m_value(std::move(value))
    { }

    [[nodiscard]] std::string_view get() const {
        return m_value;
    }

    operator std::string_view() const {
        return m_value;
    }

private:
    std::string m_value;

};

class type_error : public std::runtime_error {
public:
    explicit type_error(const char* msg)
    : std::runtime_error(msg)
    { }

};

using variant_type = std::variant<integer, string, null>;

class value : public variant_type {
public:
    value(integer integer) : variant_type(integer) { }
    value(string string)   : variant_type(string)  { }
    value(null null)       : variant_type(null)    { }

    value operator+(const value& other) const {
        return integer(std::get<integer>(*this) + std::get<integer>(other));
    }

    value operator-(const value& other) const {
        return integer(std::get<integer>(*this) - std::get<integer>(other));
    }

    [[nodiscard]] std::string format() const {
        return std::visit(overloaded_lambda {
            [](const integer& integer) {
                return std::format("int({})", integer.get());
            },
            [](const string& string) {
                return std::format("string(\"{}\")", string.get());
            },
            [](const null&) {
                return std::string("null");
            }
        }, *this);
    }

};

struct from_token_visitor {
    value operator()(const token::integer& token) const {
        return integer(token.m_value);
    }

    value operator()(const token::string& token) const {
        return string(token.m_value);
    }

    value operator()(const auto&) const {
        throw std::runtime_error("no match");
    }
};

[[nodiscard]] inline value from_token(const token::token& token) {
    return std::visit(from_token_visitor{}, token);
}

} // namespace value

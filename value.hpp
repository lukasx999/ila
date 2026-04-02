#pragma once

#include <cassert>
#include <string>
#include <type_traits>
#include <concepts>
#include <cstring>

#include "lexer.hpp"
#include "utils.hpp"

namespace value {

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

template <typename Variant, typename T>
struct variant_contains_type;

template <typename... Vs, typename T>
struct variant_contains_type<std::variant<Vs...>, T> {
    static constexpr bool value = std::disjunction_v<std::is_same<T, Vs>...>;
};

template <typename Variant, typename T>
inline constexpr bool variant_contains_type_v = variant_contains_type<Variant, T>::value;

class value {
    using variant_t = std::variant<integer, string>;

public:
    value(integer integer)
    : m_variant(integer)
    { }

    value(string string)
    : m_variant(string)
    { }

    template <typename T>
    [[nodiscard]] T get_as() const {
        static_assert(variant_contains_type_v<variant_t, T>, "value cannot be of the specified type");

        if (!is_a<T>()) throw type_error("value does not contain the specified type");
        return std::get<T>(m_variant);
    }

    template <typename T>
    [[nodiscard]] bool is_a() const {
        static_assert(variant_contains_type_v<variant_t, T>, "value cannot be of the specified type");
        return std::holds_alternative<T>(m_variant);
    }

    value operator+(const value& other) const {
        return integer(get_as<integer>() + other.get_as<integer>());
    }

    value operator-(const value& other) const {
        return integer(get_as<integer>() + other.get_as<integer>());
    }

    [[nodiscard]] std::string format() const {
        return std::visit(overloaded_lambda {
            [](const integer& integer) {
                return std::format("int({})", integer.get());
            },
            [](const string& string) {
                return std::format("string(\"{}\")", string.get());
            },
        }, m_variant);
    }

private:
    variant_t m_variant;

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

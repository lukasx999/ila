#pragma once

#include <variant>

#include "utils.hpp"

template <typename Variant, typename T>
struct variant_contains_type;

template <typename... Vs, typename T>
struct variant_contains_type<std::variant<Vs...>, T> {
    static constexpr bool value = std::disjunction_v<std::is_same<T, Vs>...>;
};

template <typename Variant, typename T>
inline constexpr bool variant_contains_type_v = variant_contains_type<Variant, T>::value;

template <typename... Ts>
class variant {
    using variant_type = std::variant<Ts...>;

public:
    variant(variant_type variant) : m_variant(variant) { }

    template <typename T>
    [[nodiscard]] T get_as() const {
        static_assert(variant_contains_type_v<variant_type, T>, "value cannot be of the specified type");
        return std::get<T>(m_variant);
    }

    template <typename T>
    [[nodiscard]] bool isa() const {
        static_assert(variant_contains_type_v<variant_type, T>, "value cannot be of the specified type");
        return std::holds_alternative<T>(m_variant);
    }

    auto match(auto&&... args) const {
        return std::visit(overloaded_lambda { args... }, m_variant);
    }

private:
    variant_type m_variant;

};

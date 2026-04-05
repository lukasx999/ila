#pragma once

#include <variant>

#include "utils.hpp"

template <typename... Ts>
struct variant : std::variant<Ts...> {
    using std::variant<Ts...>::variant;

    template <typename T>
    [[nodiscard]] const T& get_as() const {
        return std::get<T>(*this);
    }

    template <typename T>
    [[nodiscard]] T& get_as() {
        return std::get<T>(*this);
    }

    template <typename T>
    [[nodiscard]] bool isa() const {
        return std::holds_alternative<T>(*this);
    }

    auto match(auto&&... args) const {
        return std::visit(overloaded_lambda { args... }, *this);
    }
};

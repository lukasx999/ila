#pragma once

template <typename... Ts>
struct overloaded_lambda : Ts... {
    using Ts::operator()...;
};

#include <print>
#include <fstream>

#include "lexer.hpp"

int main() {

    std::ifstream file("main.ila");
    std::istreambuf_iterator<char> it(file);
    std::string src(it, {});

    lexer lexer(src);
    auto tokens = lexer.tokenize();

    for (auto& token : tokens) {
        std::println("{}", token->fmt());
    }

}

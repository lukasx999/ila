#include <print>
#include <fstream>

#include "lexer.hpp"
#include "parser.hpp"

int main() {

    std::ifstream file("main.ila");
    std::istreambuf_iterator<char> it(file);
    std::string src(it, {});

    lexer lexer(src);
    auto tokens = lexer.tokenize();

    for (auto& token : tokens) {
        auto str = std::visit(token_formatter{}, token);
        std::println("* {}", str);
    }

    parser parser(tokens);
    auto root = parser.parse();

}

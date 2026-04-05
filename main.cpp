#include <print>
#include <fstream>
#include <algorithm>
#include <ranges>

#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"
#include "runtime.hpp"

namespace {

void test_value() {
    value::integer a(4);
    value::integer b(5);
    value::value val_a(a);
    assert(val_a.isa<value::integer>());
    assert(val_a.get_as<value::integer>() == 4);
    assert(value::value(a).get_as<value::integer>() == 4);

    bool was_caught = false;

    try {
        auto unused = val_a.get_as<value::string>();
    } catch (const std::bad_variant_access& e) {
        was_caught = true;
    }
    assert(was_caught);

    value::value c(value::string("hello"));
    value::value d(value::string("foo"));
    c = d;
    assert(c.get_as<value::string>().get() == "foo");

    value::value val_int(45);

    assert(val_int.match(
        [](const value::integer&) {
            return true;
        },
        [](const auto&) {
            return false;
        }
    ));
}

void test_lexer() {
    lexer lexer("let x = 1");
    auto tokens = lexer.tokenize();
    assert(tokens.size() == 4);
    assert(tokens.at(0).isa<token::let>());
    assert(tokens.at(1).isa<token::identifier>());
    assert(tokens.at(2).isa<token::eq>());
    assert(tokens.at(3).isa<token::integer>());
}

void run_repl() {

    runtime runtime;

    while (true) {
        std::string line;
        std::print("> ");
        if (!std::getline(std::cin, line)) {
            std::println();
            break;
        }

        auto tokens = lexer(line).tokenize();
        for (auto& token : tokens) {
            std::println("{}", token::to_string(token));
        }

        parser parser(std::move(tokens));
        auto root = parser.parse();


        assert(root->isa<ast::block>());
        auto& block = root->get_as<ast::block>();
        assert(block.get_children().size() == 1);
        auto& expr = *block.get_children().front();

        ast::print_tree(expr);

        auto result = runtime.run_tree(expr);
        std::println("{}", result.to_string());
    }
};

} // namespace

int main() {

    test_value();
    test_lexer();

    run_repl();
    return 0;

    auto tokens = lexer::from_file("main.ila").tokenize();

    for (auto& token : tokens) {
        std::println("{}", token::to_string(token));
    }

    parser parser(std::move(tokens));
    auto root = parser.parse();

    std::visit(ast::tree_print_visitor(), *root);

    runtime_visitor runtime;
    auto result = std::visit(runtime, *root);
    std::println("{}", result.to_string());

}

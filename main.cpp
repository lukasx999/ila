#include <print>
#include <fstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

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
    } catch (const value::type_error& e) {
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
    assert(std::holds_alternative<token::let>(tokens.at(0)));
    assert(std::holds_alternative<token::identifier>(tokens.at(1)));
    assert(std::holds_alternative<token::eq>(tokens.at(2)));
    assert(std::holds_alternative<token::integer>(tokens.at(3)));
}

int main() {

    test_value();
    test_lexer();

    auto tokens = lexer::from_file("main.ila").tokenize();

    for (auto& token : tokens) {
        std::println("* {}", token::format(token));
    }

    parser parser(tokens);
    auto root = parser.parse();
    ast::node_formatter ast_printer;
    root->apply_visitor(ast_printer);

}

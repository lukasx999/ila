#include <print>
#include <fstream>

#include "lexer.hpp"
#include "parser.hpp"
#include "engine.hpp"
#include "value.hpp"

void test_value() {
    value::integer a(4);
    value::integer b(5);
    value::value val_a(a);
    assert(val_a.is_a<value::integer>());
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

}

int main() {

    test_value();

    std::ifstream file("main.ila");
    std::istreambuf_iterator<char> it(file);
    std::string src(it, {});

    lexer lexer(src);
    auto tokens = lexer.tokenize();

    for (auto& token : tokens) {
        std::println("* {}", token::format(token));
    }

    parser parser(tokens);
    auto root = parser.parse();
    ast::printer ast_printer;
    root->apply_visitor(ast_printer);

    engine engine;
    root->apply_visitor(engine);

    auto value = engine.get_value();
    std::println("value: {}", value.format());

}

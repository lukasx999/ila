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
    {
        auto tokens = lexer("1+2").tokenize();
        assert(tokens.size() == 4);
        assert(tokens.at(0).isa<token::integer>());
        assert(tokens.at(0).get_as<token::integer>().m_value == 1);
        assert(tokens.at(1).isa<token::plus>());
        assert(tokens.at(2).isa<token::integer>());
        assert(tokens.at(2).get_as<token::integer>().m_value == 2);
        assert(tokens.at(3).isa<token::terminator>());
    }

    {
        auto tokens = lexer("let x = 1").tokenize();
        assert(tokens.size() == 5);
        assert(tokens.at(0).isa<token::let>());
        assert(tokens.at(1).isa<token::identifier>());
        assert(tokens.at(2).isa<token::eq>());
        assert(tokens.at(3).isa<token::integer>());
        assert(tokens.at(4).isa<token::terminator>());
    }

    {
        auto tokens = lexer("\"foo\"").tokenize();
        assert(tokens.size() == 2);
        assert(tokens.at(0).isa<token::string>());
        assert(tokens.at(0).get_as<token::string>().m_value == "foo");
        assert(tokens.at(1).isa<token::terminator>());
    }
}

void print_tokens(std::span<token::token> tokens) {
    for (auto& token : tokens | std::views::take(tokens.size() - 1)) {
        std::print("{}, ", token::to_string(token));
    }
    std::println("{}", token::to_string(tokens.back()));
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
        print_tokens(tokens);

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

void run_script(const std::filesystem::path& filename) {
    auto tokens = lexer::from_file(filename).tokenize();
    print_tokens(tokens);

    parser parser(std::move(tokens));
    auto root = parser.parse();

    ast::print_tree(*root);

    runtime runtime;
    auto result = runtime.run_tree(*root);
    std::println("{}", result.to_string());
}

} // namespace

int main() {

    test_value();
    test_lexer();

    bool repl = true;

    if (repl) {
        run_repl();
    } else {
        run_script("main.ila");
    }

}

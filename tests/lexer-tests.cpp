#include <catch2/catch_test_macros.hpp>
#include "../lentus/lentus.hpp"

using namespace lentus;
#include <iostream>

TEST_CASE("handle_string_constant handles unterminated string constants", "[lexer]")
{
    std::string source = "\"This is an unterminated string constant;";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    handle_string_constant(data);

    REQUIRE(data.error_found);
    REQUIRE(tokens.empty());
    REQUIRE(data.line_number == 1);
    REQUIRE(data.line_start_position == 0);
}

TEST_CASE("handle_string_constant handles valid string constants", "[lexer]")
{
    std::string source = "\"This is a valid string constant.\";";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    handle_string_constant(data);

    REQUIRE(!data.error_found);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].get_type() == token_type::STRING);
    REQUIRE(*tokens[0].get_string_constant() == "This is a valid string constant.");
    REQUIRE(data.line_number == 1);
    REQUIRE(data.line_start_position == 0);
}

TEST_CASE("handle_string_constant with escaped characters", "[lexer]")
{
    std::string source = "\"This is a string constant with an escaped quote: \\\" and a newline: \\n. Invalid escape sequences should not be treated as escapes: \\x\";";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    handle_string_constant(data);

    REQUIRE(!data.error_found);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].get_type() == token_type::STRING);
    REQUIRE(*tokens[0].get_string_constant() == "This is a string constant with an escaped quote: \" and a newline: \n. Invalid escape sequences should not be treated as escapes: \\x");
    REQUIRE(data.line_number == 1);
    REQUIRE(data.line_start_position == 0);
}

TEST_CASE("handle_identifier_or_keyword handle identifier", "[lexer]")
{
    std::string source = "hi = 42;";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    handle_identifier_or_keyword(data);

    REQUIRE(!data.error_found);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].get_type() == token_type::IDENTIFIER);
    REQUIRE(*tokens[0].get_identifier_name() == "hi");
    REQUIRE(data.line_number == 1);
    REQUIRE(data.line_start_position == 0);
}

TEST_CASE("handle_identifier_or_keyword handle keyword", "[lexer]")
{
    std::string source = "var x = 42;";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    handle_identifier_or_keyword(data);

    REQUIRE(!data.error_found);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].get_type() == token_type::VAR);
    REQUIRE(data.line_number == 1);
    REQUIRE(data.line_start_position == 0);
}

TEST_CASE("handle_number", "[lexer]")
{
    std::string source1 = "42";
    std::vector<token> tokens1;
    lexer_data data1{1, 0, 0, source1, tokens1, std::cout, false};
    handle_number(data1);

    REQUIRE(!data1.error_found);
    REQUIRE(tokens1.size() == 1);
    REQUIRE(tokens1[0].get_type() == token_type::NUMBER);
    REQUIRE(tokens1[0].get_number_constant() == 42);
    REQUIRE(data1.line_number == 1);
    REQUIRE(data1.line_start_position == 0);

    std::string source2 = "3.14";
    std::vector<token> tokens2;
    lexer_data data2{1, 0, 0, source2, tokens2, std::cout, false};
    handle_number(data2);

    REQUIRE(!data2.error_found);
    REQUIRE(tokens2.size() == 1);
    REQUIRE(tokens2[0].get_type() == token_type::NUMBER);
    REQUIRE(tokens2[0].get_number_constant() == 3.14);

    std::string source3 = "3.14e10";
    std::vector<token> tokens3;
    lexer_data data3{1, 0, 0, source3, tokens3, std::cout, false};
    handle_number(data3);

    REQUIRE(!data3.error_found);
    REQUIRE(tokens3.size() == 1);
    REQUIRE(tokens3[0].get_type() == token_type::NUMBER);
    REQUIRE(tokens3[0].get_number_constant() == 3.14e10);
}

TEST_CASE("Lexer handles invalid numbers", "[lexer]")
{
    std::string source1 = "3.14e";
    std::vector<token> tokens1;
    lexer_data data1{1, 0, 0, source1, tokens1, std::cout, false};
    handle_number(data1);

    REQUIRE(data1.error_found);
    REQUIRE(tokens1.empty());

    std::string source2 = "3.14.";
    std::vector<token> tokens2;
    lexer_data data2{1, 0, 0, source2, tokens2, std::cout, false};
    handle_number(data2);

    REQUIRE(data2.error_found);
    REQUIRE(tokens2.empty());

    // don't need to check for starting with "." or "e" because the lexer will never call handle_number in those cases
}

TEST_CASE("handle_symbolic_token multi character token", "[lexer]")
{
    std::string source = "<=";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    bool result = handle_symbolic_token(data);

    REQUIRE(result);
    REQUIRE(!data.error_found);
    REQUIRE(tokens.size() == 1);
    REQUIRE(tokens[0].get_type() == token_type::LESS_EQUAL);
}

TEST_CASE("handle_symbolic_token non matching", "[lexer]")
{
    std::string source = "and";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    bool result = handle_symbolic_token(data);

    REQUIRE(!result);
    REQUIRE(tokens.empty());
    REQUIRE(!data.error_found); // No match at all, but also no partial match, so no error
}

TEST_CASE("tokenize handles unexpected characters", "[lexer]")
{
    std::string source = "var x = 42 @;";

    std::optional<std::vector<token>> result = tokenize(source, std::cout);

    REQUIRE(!result.has_value());
}

TEST_CASE("handle_comment handles single line comments", "[lexer]")
{
    std::string source = "// This is a single line comment\nvar x = 42;";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    bool result = handle_comment(data);

    REQUIRE(result);
    REQUIRE(!data.error_found);
    REQUIRE(tokens.empty());
    REQUIRE(data.line_number == 2);
    REQUIRE(data.line_start_position == (int)std::string("// This is a single line comment\n").length());
    REQUIRE(data.position == (int)std::string("// This is a single line comment\n").length());
}

TEST_CASE("handle_comment handles multi line comments", "[lexer]")
{
    std::string source = "/* This is a \nmulti line comment */ var x = 42;";
    std::vector<token> tokens;
    lexer_data data{1, 0, 0, source, tokens, std::cout, false};

    bool result = handle_comment(data);

    REQUIRE(result);
    REQUIRE(!data.error_found);
    REQUIRE(tokens.empty());
    REQUIRE(data.line_number == 2);
    REQUIRE(data.line_start_position == (int)std::string("/* This is a \n").length());
    REQUIRE(data.position == (int)std::string("/* This is a \nmulti line comment */").length());
}
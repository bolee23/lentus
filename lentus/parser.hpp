#ifndef LENTUS_PARSER_HPP
#define LENTUS_PARSER_HPP

#include "token.hpp"
#include "ast.hpp"

/** Represents a line of source code (for error reporting) */
struct source_line
{
    std::string text;
    int start_position;
};

struct parse_result
{
    expression *ast;
    bool error;
};

struct parser_data
{
    const std::vector<token> &tokens;
    const std::string &source_string;
    const std::vector<source_line> &lines;
    std::ostream &error_stream;
    bool error_found = false;

    int position = 0; /** Current position in the tokens vector */
};

/**
 * Parses a vector of tokens into an abstract syntax tree (AST) represented by an expression pointer. The AST can then be evaluated by the execution context.
 * @param tokens vector of tokens to parse
 * @param source_string the original source code string (for error reporting)
 * @param error_stream stream to write error messages to
 * @return pointer to the root of the AST, or nullptr if a syntax error is found
 */
parse_result parse(const std::vector<token> &tokens, const std::string &source_string, std::ostream &error_stream);

void write_expression(expression *expr);

#endif
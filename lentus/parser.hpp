#ifndef LENTUS_PARSER_HPP
#define LENTUS_PARSER_HPP

#include "token.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "program.hpp"

/** Represents a line of source code (for error reporting) */
struct source_line
{
    std::string text;
    int start_position;
};

template <typename T>
struct parse_result
{
    T *ast;
    bool error;
};

struct parser_data
{
    const std::vector<token> &tokens;
    const std::string &source_string;
    const std::vector<source_line> &lines;
    std::vector<variable_block *> &constant_blocks;
    std::ostream &error_stream;
    bool error_found = false;

    int position = 0; /** Current position in the tokens vector */
};

/**
 * Parses a vector of tokens into an abstract syntax tree (AST).
 * @param tokens vector of tokens to parse
 * @param source_string the original source code string (for error reporting)
 * @param error_stream stream to write error messages to
 * @return Parse result containing the AST and whether an error was found during parsing
 */
parse_result<statement> parse(std::vector<variable_block *> &literal_variables, const std::vector<token> &tokens, const std::string &source_string, std::ostream &error_stream);

void write_ast_node(ast_node *node);

#endif
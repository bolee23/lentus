#include "parser.hpp"
#include <iostream>
#include "function.hpp"

void write_error_message(parser_data &data, const std::string &message)
{

    const token *current;
    int line_number;
    int line_start_position;

    if (data.position >= (int)data.tokens.size())
    {
        current = &data.tokens.back();
        line_number = current->get_line_number();
        line_start_position = data.lines[line_number - 1].start_position;
    }
    else
    {
        current = &data.tokens[data.position];
        line_number = current->get_line_number();
        line_start_position = data.lines[line_number - 1].start_position;
    }

    data.error_stream << "Error at line " << line_number << ": " << message << std::endl;
    data.error_stream << data.lines[line_number - 1].text << std::endl;

    // Position the arrows
    for (int i = 0; i < current->get_start_position() - line_start_position; i++)
    {
        data.error_stream << ' ';
    }

    // One arrow for each character in the token
    for (int i = current->get_start_position(); i < current->get_end_position(); i++)
    {
        data.error_stream << '^';
    }

    data.error_stream << std::endl;
    data.error_found = true;
}

std::string token_type_list_to_string(const std::vector<token_type> &types)
{
    std::string result;
    for (size_t i = 0; i < types.size(); i++)
    {
        result += token_type_to_string(types[i]);
        if (i < types.size() - 1)
        {
            result += " or ";
        }
    }
    return result;
}

void write_expectation_error_message(parser_data &data, std::string expected)
{
    if (data.position >= (int)data.tokens.size())
    {
        write_error_message(data, "Unexpected end of input, expected " + expected);
    }
    else
    {
        const token &current = data.tokens[data.position];
        write_error_message(data, "Unexpected token \"" + current.to_string(data.source_string) + "\", expected " + expected);
    }
}

bool check(parser_data &data, token_type type)
{
    if (data.position >= (int)data.tokens.size())
    {
        return false;
    }
    else
    {
        return data.tokens[data.position].get_type() == type;
    }
}

/**
 * If the current token matches any of the given types, consume it and return true. Otherwise, return false.
 * @param data Parser data
 * @param token_types Token types to check for
 * @return true if token matches
 */
bool match_and_consume(parser_data &data, std::vector<token_type> token_types)
{
    for (token_type type : token_types)
    {
        if (check(data, type))
        {
            data.position++;
            return true;
        }
    }

    return false;
}

/**
 * If the current token matches the given type, consume it and return true. Otherwise, return false.
 * @param data Parser data
 * @param token_type Token type
 * @return true if token matches
 */
bool match_and_consume(parser_data &data, token_type type)
{
    if (check(data, type))
    {
        data.position++;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * Function template for parsing left associative binary operators.
 * @param data Parser data
 * @param function Next highest precedence function
 * @param token_types Token types to check for
 * @return Parsed expression
 */
template <typename F>
parse_result<expression> parse_left_associative(parser_data &data, F function, std::vector<token_type> token_types)
{
    parse_result<expression> result = function(data);
    bool error = result.error;
    expression *left = result.ast;

    while (match_and_consume(data, token_types))
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> right_result = function(data);
        expression *right = right_result.ast;
        error |= right_result.error;
        left = new binary_expression(op, left, right);
    }

    return {left, error};
}

parse_result<expression> parse_expression(parser_data &data);
parse_result<statement> parse_statement(parser_data &data, bool allow_declarations = true);
parse_result<statement> parse_sequence(parser_data &data);

parse_result<expression> parse_primary(parser_data &data)
{
    if (data.position >= (int)data.tokens.size())
    {
        write_error_message(data, "Unexpected end of input");
        return {new error_expression(), true};
    }
    const token &current = data.tokens[data.position];
    int line_number = current.get_line_number();
    int start_position = current.get_start_position();
    int end_position = current.get_end_position();
    switch (current.get_type())
    {

    case token_type::FALSE:
        data.position++;
        return {new literal_expression(variable(false), line_number, start_position, end_position), false};
    case token_type::TRUE:
        data.position++;
        return {new literal_expression(variable(true), line_number, start_position, end_position), false};
    case token_type::NONE:
        data.position++;
        return {new literal_expression(variable(), line_number, start_position, end_position), false};
    case token_type::NUMBER:
        data.position++;
        return {new literal_expression(variable(current.get_number_constant()), line_number, start_position, end_position), false};
    case token_type::STRING:
    {
        data.position++;
        lentus_string *string_constant = new lentus_string(*current.get_string_constant());
        data.constant_blocks.push_back(string_constant);
        return {new literal_expression(variable(string_constant), line_number, start_position, end_position), false};
    }
    case token_type::LEFT_BRACKET:
    {
        data.position++;
        parse_result<expression> other_result = parse_expression(data);
        expression *other_expression = other_result.ast;
        bool error = other_result.error;

        // Expect right bracket to match left bracket
        if (!match_and_consume(data, token_type::RIGHT_BRACKET))
        {
            write_expectation_error_message(data, "'}' here");
            delete other_expression;
            return {new error_expression(), true};
        }
        int right_bracket_end_position = data.tokens[data.position - 1].get_end_position();

        return {new grouping_expression(other_expression, line_number, start_position, right_bracket_end_position), error};
    }
    case token_type::IDENTIFIER:
        data.position++;
        return {new identifier_expression(*current.get_identifier_name(), line_number, start_position, end_position), false};
    default:
    {
        data.position++; // Don't keep parsing the same unexpected token
        // write_error_message(data, std::string("Unexpected token \"") + current.to_string(data.source_string) + "\"");
        return {new error_expression(), true};
        break;
    }
    }
}

/**
 * Parses a function call
 */
parse_result<expression> parse_call(parser_data &data)
{
    parse_result<expression> result = parse_primary(data);
    expression *expr = result.ast;
    bool error = result.error;

    while (match_and_consume(data, token_type::LEFT_BRACKET))
    {
        std::vector<expression *> arguments;

        if (!match_and_consume(data, token_type::RIGHT_BRACKET)) // Call with arguments
        {
            // Add at least one argument, more if commas are observed
            do
            {
                parse_result<expression> arg_result = parse_expression(data);
                error |= arg_result.error;
                arguments.push_back(arg_result.ast);
            } while (match_and_consume(data, token_type::COMMA));

            if (!match_and_consume(data, token_type::RIGHT_BRACKET))
            {
                write_expectation_error_message(data, "')' after function call");
                for (expression *expr : arguments)
                {
                    delete expr;
                }
                data.position++;
                return {new error_expression(), true};
            }
        }

        expr = new call_expression(expr, std::move(arguments), expr->get_line_number(), expr->get_start_position(), data.tokens[data.position - 1].get_end_position());
    }

    return {expr, error};
}

parse_result<expression> parse_not_subtract(parser_data &data)
{
    if (match_and_consume(data, {token_type::NOT, token_type::SUBTRACT}))
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> operand_result = parse_not_subtract(data);
        expression *right = operand_result.ast;

        return {new unary_expression(op, right), operand_result.error};
    }

    return parse_call(data);
}

parse_result<expression> parse_factor(parser_data &data)
{
    return parse_left_associative(data, parse_not_subtract, {token_type::MULTIPLY, token_type::DIVIDE});
}

parse_result<expression> parse_term(parser_data &data)
{
    return parse_left_associative(data, parse_factor, {token_type::ADD, token_type::SUBTRACT});
}

parse_result<expression> parse_comparison(parser_data &data)
{
    return parse_left_associative(data, parse_term, {token_type::GREATER, token_type::GREATER_EQUAL, token_type::LESS, token_type::LESS_EQUAL});
}

parse_result<expression> parse_equality(parser_data &data)
{
    return parse_left_associative(data, parse_comparison, {token_type::NOT_EQUAL, token_type::EQUAL});
}

parse_result<expression> parse_logical_and(parser_data &data)
{
    return parse_left_associative(data, parse_equality, {token_type::AND});
}

parse_result<expression> parse_logical_or(parser_data &data)
{
    return parse_left_associative(data, parse_logical_and, {token_type::OR});
}

parse_result<expression> parse_direct_assignment(parser_data &data)
{
    parse_result<expression> left_result = parse_logical_or(data);
    expression *left = left_result.ast;

    if (match_and_consume(data, token_type::ASSIGN))
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> right_result = parse_direct_assignment(data);
        expression *right = right_result.ast;
        bool error = left_result.error || right_result.error;

        return {new binary_expression(op, left, right), error};
    }

    return left_result;
}

parse_result<expression> parse_compound_assignment(parser_data &data)
{
    parse_result<expression> left_result = parse_direct_assignment(data);
    expression *left = left_result.ast;

    if (match_and_consume(data, {token_type::ADD_ASSIGN, token_type::SUBTRACT_ASSIGN, token_type::MULTIPLY_ASSIGN, token_type::DIVIDE_ASSIGN}))
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> right_result = parse_compound_assignment(data);
        expression *right = right_result.ast;
        bool error = left_result.error || right_result.error;

        return {new binary_expression(op, left, right), error};
    }

    return left_result;
}

parse_result<expression> parse_expression(parser_data &data)
{
    return parse_compound_assignment(data);
}

std::vector<source_line> split_lines(const std::string &source_string)
{
    std::vector<source_line> lines;
    std::string current_line;
    int pos = 0;
    int line_start = 0;
    for (int i = 0; i < (int)source_string.length(); i++)
    {
        char c = source_string[i];
        if (c == '\n')
        {
            lines.push_back({std::move(current_line), line_start});
            current_line.clear();
            pos = i + 1;
            line_start = pos;
        }
        else
        {
            current_line += c;
        }
    }
    if (!current_line.empty())
    {
        lines.push_back({std::move(current_line), line_start});
    }
    return lines;
}

parse_result<statement> parse_declaration(parser_data &data)
{
    if (match_and_consume(data, token_type::IDENTIFIER))
    {
        const token &identifier_token = data.tokens[data.position - 1];
        lentus_string *identifier = new lentus_string(*identifier_token.get_identifier_name());
        data.constant_blocks.push_back(identifier);

        int line_number = identifier_token.get_line_number();
        int start_position = identifier_token.get_start_position();

        if (match_and_consume(data, token_type::ASSIGN))
        {
            parse_result<expression> value_result = parse_expression(data);
            expression *value = value_result.ast;
            bool error = value_result.error;

            if (!match_and_consume(data, token_type::END_STATEMENT))
            {
                write_expectation_error_message(data, "';' here");
                delete value;
                return {new error_statement(), true};
            }

            return {new declaration_statement(identifier, value, line_number, start_position, value->get_end_position()), error};
        }
        else
        {
            if (!match_and_consume(data, token_type::END_STATEMENT))
            {
                write_expectation_error_message(data, "';' here");
                return {new error_statement(), true};
            }

            int end_position = identifier_token.get_end_position();
            return {new declaration_statement(identifier, nullptr, line_number, start_position, end_position), false};
        }
    }
    else
    {
        write_expectation_error_message(data, "identifier after 'var'");
        return {new error_statement(), true};
    }
}

parse_result<statement> parse_expression_statement(parser_data &data)
{
    parse_result<expression> expr_result = parse_expression(data);
    if (match_and_consume(data, token_type::END_STATEMENT))
    {
        return {new expression_statement(expr_result.ast), expr_result.error};
    }
    else
    {
        write_expectation_error_message(data, "';' here");
        return {new error_statement(), true};
    }
}

parse_result<statement> parse_for_statement(parser_data &data)
{
    if (!match_and_consume(data, token_type::LEFT_BRACKET))
    {
        write_expectation_error_message(data, "'(' after 'for'");
        return {new error_statement(), true};
    }
    int line_number = data.tokens[data.position - 1].get_line_number();
    int start_position = data.tokens[data.position - 1].get_start_position();
    int end_position;

    statement *initializer = nullptr;
    bool error = false;

    parse_result<statement> initializer_result = parse_statement(data);
    initializer = initializer_result.ast;
    error |= initializer_result.error;

    parse_result<expression> condition_result = parse_expression(data);
    error |= condition_result.error;

    if (!match_and_consume(data, token_type::END_STATEMENT))
    {
        write_expectation_error_message(data, "';' after loop condition");
        delete initializer;
        delete condition_result.ast;
        return {new error_statement(), true};
    }

    expression *post_iteration = nullptr;

    if (!check(data, token_type::RIGHT_BRACKET)) // Skip post-iteration if there is an immediate right bracket "for(initializer; condition;) { body }"
    {
        parse_result<expression> post_iteration_result = parse_expression(data);
        error |= post_iteration_result.error;
        post_iteration = post_iteration_result.ast;
    }

    // Actually try consume the )
    if (!match_and_consume(data, token_type::RIGHT_BRACKET))
    {
        write_expectation_error_message(data, "')' after for clauses");
        delete initializer;
        delete condition_result.ast;
        delete post_iteration;
        return {new error_statement(), true};
    }

    parse_result<statement> body_result = parse_statement(data);
    error |= body_result.error;

    end_position = body_result.ast->get_end_position();

    return {new for_statement(initializer, condition_result.ast, post_iteration, body_result.ast, line_number, start_position, end_position), error};
}

parse_result<statement> parse_while_statement(parser_data &data)
{
    parse_result<expression> condition_result = parse_expression(data);
    bool error = condition_result.error;
    int line_number = condition_result.ast->get_line_number();
    int start_position = condition_result.ast->get_start_position();

    parse_result<statement> body_result = parse_statement(data);
    error |= body_result.error;
    int end_position = body_result.ast->get_end_position();

    return {new loop_statement(condition_result.ast, body_result.ast, false, line_number, start_position, end_position), error};
}

parse_result<statement> parse_do_statement(parser_data &data)
{
    parse_result<statement> body_result = parse_statement(data);
    bool error = body_result.error;
    int line_number = body_result.ast->get_line_number();
    int start_position = body_result.ast->get_start_position();

    if (!match_and_consume(data, token_type::WHILE))
    {
        write_expectation_error_message(data, "'while' after 'do' statement body");
        return {new error_statement(), true};
    }

    parse_result<expression> condition_result = parse_expression(data);
    error |= condition_result.error;
    int end_position = condition_result.ast->get_end_position();

    if (!match_and_consume(data, token_type::END_STATEMENT))
    {
        write_expectation_error_message(data, "';' after 'while'");
        return {new error_statement(), true};
    }

    return {new loop_statement(condition_result.ast, body_result.ast, true, line_number, start_position, end_position), error};
}

parse_result<statement> parse_if_statement(parser_data &data)
{
    parse_result<expression> condition_result = parse_expression(data);
    bool error = condition_result.error;
    int line_number = condition_result.ast->get_line_number();
    int start_position = condition_result.ast->get_start_position();

    parse_result<statement> then_branch_result = parse_statement(data, false);
    if (then_branch_result.error)
    {
        delete condition_result.ast;
        return {new error_statement(), true};
    }

    int end_position = data.tokens[data.position - 1].get_end_position();

    // Else (optional)
    statement *else_branch = nullptr;
    if (match_and_consume(data, token_type::ELSE))
    {
        parse_result<statement> else_branch_result = parse_statement(data, false);
        error |= else_branch_result.error;
        else_branch = else_branch_result.ast;

        end_position = data.tokens[data.position - 1].get_end_position();
    }

    return {new if_statement(condition_result.ast, then_branch_result.ast, else_branch, line_number, start_position, end_position), error};
}

parse_result<statement> parse_braced_sequence(parser_data &data)
{
    std::vector<statement *> statements;
    bool error = false;

    while (data.position < (int)data.tokens.size())
    {
        if (match_and_consume(data, token_type::RIGHT_BRACE))
        {
            return {new sequence_statement(std::move(statements)), error};
        }
        parse_result<statement> stmt_result = parse_statement(data);
        statements.push_back(stmt_result.ast);
        error |= stmt_result.error;
    }

    // EOF before closing brace
    write_expectation_error_message(data, "'}' here");
    return {new sequence_statement(std::move(statements)), true};
}

parse_result<statement> parse_function_declaration(parser_data &data)
{
    if (!match_and_consume(data, token_type::IDENTIFIER))
    {
        write_expectation_error_message(data, "function name after 'function'");
        return {new error_statement(), true};
    }

    const token &identifier_token = data.tokens[data.position - 1];
    lentus_string *identifier = new lentus_string(*identifier_token.get_identifier_name());
    data.constant_blocks.push_back(identifier);
    int line_number = identifier_token.get_line_number();
    int start_position = identifier_token.get_start_position();

    if (!match_and_consume(data, token_type::LEFT_BRACKET))
    {
        write_expectation_error_message(data, "'(' after function name");
        return {new error_statement(), true};
    }

    std::vector<lentus_string *> parameters;

    while (!match_and_consume(data, token_type::RIGHT_BRACKET))
    {
        // Consume "var" before parameter name but this is optional.
        // function(var x) and function(x) are allowed.
        match_and_consume(data, token_type::VAR);

        if (!match_and_consume(data, token_type::IDENTIFIER))
        {
            write_expectation_error_message(data, "parameter name in function declaration");
            return {new error_statement(), true};
        }

        lentus_string *param = new lentus_string(*data.tokens[data.position - 1].get_identifier_name());
        data.constant_blocks.push_back(param);
        parameters.push_back(param);

        // If there is a comma, consume it and continue to the next parameter. Otherwise, expect a right bracket.
        if (!match_and_consume(data, token_type::COMMA))
        {
            if (!check(data, token_type::RIGHT_BRACKET))
            {
                write_expectation_error_message(data, "')' or ',' after parameter in function declaration");
                return {new error_statement(), true};
            }
        }
    }

    parse_result<statement> body_result = parse_statement(data);
    bool error = body_result.error;
    int end_position = body_result.ast->get_end_position();

    // The function
    function_expression *fn = new function_expression(identifier, body_result.ast, std::move(parameters), line_number, start_position, end_position);

    // The variable declaration statement that declares the function name and assigns the function to it
    declaration_statement *declaration = new declaration_statement(identifier, fn, line_number, start_position, end_position);
    return {declaration, error};
}

parse_result<statement> parse_statement(parser_data &data, bool allow_declarations)
{
    if (data.position >= (int)data.tokens.size())
    {
        write_error_message(data, "Unexpected end of input, expected statement");
        return {new error_statement(), true};
    }

    switch (data.tokens[data.position].get_type())
    {
    case token_type::FUNCTION:
        if (!allow_declarations)
        {
            write_error_message(data, "Function declarations are not allowed here");
            data.position++;
            return {new error_statement(), true};
        }
        data.position++;
        return parse_function_declaration(data);
    case token_type::END_STATEMENT:
        // ';' extra semicolon counts as an empty statement
        // This may be needed ie to skip the initializer statement in a for loop.
        data.position++;
        return {new empty_statement(), false};
    case token_type::FOR:
        data.position++;
        return parse_for_statement(data);
    case token_type::WHILE:
        data.position++;
        return parse_while_statement(data);
    case token_type::DO:
        data.position++;
        return parse_do_statement(data);
    case token_type::LEFT_BRACE:
        data.position++;
        return parse_braced_sequence(data);
    case token_type::IF:
        data.position++;
        return parse_if_statement(data);
    case token_type::VAR:
        if (!allow_declarations)
        {
            write_error_message(data, "Variable declarations are not allowed here");
            data.position++;
            return {new error_statement(), true};
        }
        data.position++;
        return parse_declaration(data);
    default:
        return parse_expression_statement(data);
    }
}

parse_result<statement> parse_sequence(parser_data &data)
{
    std::vector<statement *> statements;
    bool error = false;

    while (data.position < (int)data.tokens.size())
    {
        parse_result<statement> stmt_result = parse_statement(data);
        statements.push_back(stmt_result.ast);
        error |= stmt_result.error;
    }

    return {new sequence_statement(std::move(statements)), error};
}

parse_result<statement> parse_module(parser_data &data)
{
    return parse_sequence(data);
}

parse_result<statement> parse(std::vector<variable_block *> &literal_variables, const std::vector<token> &tokens, const std::string &source_string, std::ostream &error_stream)
{
    // Special case : empty input, single empty sequence statement is returned, and no error is reported
    if (tokens.empty())
    {
        return {new sequence_statement(), false};
    }

    std::vector<source_line> lines = split_lines(source_string);
    parser_data data{tokens, source_string, lines, literal_variables, error_stream, 0};

    parse_result<statement> result = parse_module(data);

    // If there are still tokens left after parsing the expression, that means there is an unexpected token
    if (data.position < (int)data.tokens.size())
    {
        write_error_message(data, std::string("Unexpected token \"") + data.tokens[data.position].to_string(data.source_string) + "\"");
        return {result.ast, true};
    }

    return result;
}

void write_ast_node(ast_node *node)
{
    std::cout << node->to_string() << std::endl;
}
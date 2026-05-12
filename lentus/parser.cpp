#include "parser.hpp"
#include <iostream>

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
    for (int i = 0; i < current->get_start_position() - line_start_position; i++)
    {
        data.error_stream << ' ';
    }
    for (int i = current->get_start_position(); i < current->get_end_position(); i++)
    {
        data.error_stream << '^';
    }
    data.error_stream << std::endl;
    data.error_found = true;
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
 * Function template for parsing left associative binary operators.
 * @param data Parser data
 * @param function Next highest precedence function
 * @param token_types Token types to check for
 * @return Parsed expression
 */
template <typename F>
parse_result parse_left_associative(parser_data &data, F function, std::vector<token_type> token_types)
{
    parse_result result = function(data);
    bool error = result.error;
    ast_node *left = result.ast;

    while (match_and_consume(data, token_types))
    {
        token op = data.tokens[data.position - 1];
        parse_result right_result = function(data);
        ast_node *right = right_result.ast;
        error |= right_result.error;
        left = new binary_expression(op, left, right);
    }

    return {left, error};
}

parse_result parse_expression(parser_data &data);

parse_result parse_primary(parser_data &data)
{
    if (data.position >= (int)data.tokens.size())
    {
        write_error_message(data, "Unexpected end of input");
        return {new error_node(), true};
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
        return {new literal_expression(variable(), line_number, start_position, end_position), true};
    case token_type::NUMBER:
        data.position++;
        return {new literal_expression(variable(current.get_number_constant()), line_number, start_position, end_position), false};
    case token_type::STRING:
        data.position++;
        return {new literal_expression(variable(std::string(*current.get_string_constant())), line_number, start_position, end_position), false};
    case token_type::LEFT_BRACKET:
    {
        data.position++;
        parse_result other_result = parse_expression(data);
        ast_node *other_expression = other_result.ast;
        bool error = other_result.error;

        // Expect right bracket to match left bracket
        if (!match_and_consume(data, {token_type::RIGHT_BRACKET}))
        {
            if (data.position >= (int)data.tokens.size())
            {
                write_error_message(data, "Missing ')'");
            }
            else
            {
                write_error_message(data, "')' expected, got \"" + data.tokens[data.position].to_string(data.source_string) + "\" instead");
            }
            delete other_expression;
            return {new error_node(), true};
        }
        int right_bracket_end_position = data.tokens[data.position - 1].get_end_position();

        return {new grouping_expression(other_expression, line_number, start_position, right_bracket_end_position), error};
    }
    default:
    {
        write_error_message(data, std::string("Unexpected token \"") + current.to_string(data.source_string) + "\"");
        return {new error_node(), true};
        break;
    }
    }
}

parse_result parse_unary(parser_data &data)
{
    if (match_and_consume(data, {token_type::NOT, token_type::SUBTRACT}))
    {
        token op = data.tokens[data.position - 1];
        parse_result operand_result = parse_unary(data);
        ast_node *right = operand_result.ast;

        return {new unary_expression(op, right), operand_result.error};
    }

    return parse_primary(data);
}

parse_result parse_factor(parser_data &data)
{
    return parse_left_associative(data, parse_unary, {token_type::MULTIPLY, token_type::DIVIDE});
}

parse_result parse_term(parser_data &data)
{
    return parse_left_associative(data, parse_factor, {token_type::ADD, token_type::SUBTRACT});
}

parse_result parse_comparison(parser_data &data)
{
    return parse_left_associative(data, parse_term, {token_type::GREATER, token_type::GREATER_EQUAL, token_type::LESS, token_type::LESS_EQUAL});
}

parse_result parse_equality(parser_data &data)
{
    return parse_left_associative(data, parse_comparison, {token_type::NOT_EQUAL, token_type::EQUAL});
}

parse_result parse_expression(parser_data &data)
{
    return parse_equality(data);
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

parse_result parse(const std::vector<token> &tokens, const std::string &source_string, std::ostream &error_stream)
{
    // Special case : empty input, single output with value None
    if (tokens.empty())
    {
        return {new literal_expression(variable(), 1, 0, 0), false};
    }

    std::vector<source_line> lines = split_lines(source_string);
    parser_data data{tokens, source_string, lines, error_stream, 0};

    parse_result result = parse_expression(data);

    // If there are still tokens left after parsing the expression, that means there is an unexpected token
    if (data.position < (int)data.tokens.size())
    {
        write_error_message(data, std::string("Unexpected token \"") + data.tokens[data.position].to_string(data.source_string) + "\"");
        return {result.ast, true};
    }

    return result;
}

void write_expression(ast_node *expr)
{
    std::cout << expr->to_string() << std::endl;
}
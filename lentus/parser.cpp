#include "parser.hpp"
#include <iostream>
#include "function.hpp"

/**
 * LENTUS PARSER
 * This parser uses a recursive descent parsing approach.
 */

// Helper functions for parsing
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
 * @param token_types Token types to check for at this level of precedence
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

// Parser

/**
 * Parses:
 * literals
 * grouping expressions (parentheses)
 * identifiers
 * @param data Parser data
 * @return A primary expression, and error status.
 */
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
        data.position++;
        write_error_message(data, std::string("Unexpected token \"") + current.to_string(data.source_string) + "\"");
        return {new error_expression(), true};
        break;
    }
    }
}

/**
 * Parses function calls of the form expr(expr, expr, ...)
 * @param data Parser data
 * @param expr Expression being called
 * @return Expression with function call applied, and error status.
 */
parse_result<expression> parse_call(parser_data &data, expression *expr)
{
    bool error = false;
    std::vector<expression *> arguments;

    if (!match_and_consume(data, token_type::RIGHT_BRACKET)) // Call with arguments
    {
        do
        {
            if (check(data, token_type::RIGHT_BRACKET)) // Allow trailing comma after last argument ie (a, b,)
            {
                break;
            }
            parse_result<expression> arg_result = parse_expression(data);
            error |= arg_result.error;
            arguments.push_back(arg_result.ast);
        } while (match_and_consume(data, token_type::COMMA));

        if (!match_and_consume(data, token_type::RIGHT_BRACKET))
        {
            write_expectation_error_message(data, "')' after function call");
            for (expression *argument : arguments)
            {
                delete argument;
            }
            data.position++;
            return {new error_expression(), true};
        }
    }

    return {new call_expression(expr, std::move(arguments), expr->get_line_number(), expr->get_start_position(), data.tokens[data.position - 1].get_end_position()), error};
}

/**
 * Parses subscripts of the form expr[expr]
 * @param data Parser data
 * @param expr Expression before the subscript
 * @return Expression with subscript applied, and error status.
 */
parse_result<expression> parse_subscript(parser_data &data, expression *expr)
{
    parse_result<expression> index_result = parse_expression(data);
    bool error = index_result.error;
    expression *index = index_result.ast;

    if (!match_and_consume(data, token_type::RIGHT_INDEX))
    {
        write_expectation_error_message(data, "]' after subscript index");
        delete index;
        data.position++;
        return {new error_expression(), true};
    }

    return {new subscript_expression(expr, index, expr->get_line_number(), expr->get_start_position(), data.tokens[data.position - 1].get_end_position()), error};
}

/**
 * Parses member access using the dot operator
 * @param data Parser data
 * @param expr Expression before the dot operator
 * @return Expression with member access applied, and error status.
 */
parse_result<expression> parse_member_access(parser_data &data, expression *expr)
{
    if (!check(data, token_type::IDENTIFIER))
    {
        write_expectation_error_message(data, "identifier after '.'");
        data.position++;
        return {new error_expression(), true};
    }

    const token &identifier_token = data.tokens[data.position];
    data.position++;

    lentus_string *identifier_string = new lentus_string(*identifier_token.get_identifier_name());
    data.constant_blocks.push_back(identifier_string);
    expression *right = new literal_expression(variable(identifier_string), identifier_token.get_line_number(), identifier_token.get_start_position(), identifier_token.get_end_position());

    return {new subscript_expression(expr, right, expr->get_line_number(), expr->get_start_position(), identifier_token.get_end_position()), false};
}

/**
 * Parses zero or more instances of function calls, subscripts, and member access (dot operator). All left associative.
 * @param data Parser data
 * @param expr Expression to apply accesses to.
 * @return Expression with accesses applied, and error status.
 */
parse_result<expression> parse_accesses(parser_data &data, expression *expr)
{
    // accesses = (call | subscript | member_access)*
    bool error = false;
    while (true)
    {
        if (match_and_consume(data, token_type::LEFT_BRACKET))
        {
            parse_result<expression> call_result = parse_call(data, expr);
            error |= call_result.error;
            expr = call_result.ast;
        }
        else if (match_and_consume(data, token_type::LEFT_INDEX))
        {
            parse_result<expression> subscript_result = parse_subscript(data, expr);
            error |= subscript_result.error;
            expr = subscript_result.ast;
        }
        else if (match_and_consume(data, token_type::DOT))
        {
            parse_result<expression> member_result = parse_member_access(data, expr);
            error |= member_result.error;
            expr = member_result.ast;
        }
        else
        {
            break;
        }
    }
    return {expr, error};
}

/**
 * Parses postfix increment/decrement (var++ and var--), and accesses (function calls, subscripts, member access).
 * Also calls parse_access to parse function calls, subscripts and member access after primary expressions.
 * All left associative.
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_postfix_access(parser_data &data)
{
    // postfix_access = primary ('++' | '--')* accesses ('++' | '--')*
    parse_result<expression> result = parse_primary(data);
    expression *expr = result.ast;
    bool error = result.error;

    // var++ and var--;
    while (match_and_consume(data, {token_type::INCREMENT, token_type::DECREMENT}))
    {
        token op = data.tokens[data.position - 1];
        expr = new unary_expression(op, expr, true);
    }

    parse_result<expression> access_result = parse_accesses(data, expr);
    expr = access_result.ast;
    error |= access_result.error;

    // Handle var[i](j)[k]++ and --;
    while (match_and_consume(data, {token_type::INCREMENT, token_type::DECREMENT}))
    {
        token op = data.tokens[data.position - 1];
        expr = new unary_expression(op, expr, true);
    }

    return {expr, error};
}

/**
 * Parses prefix increment/decrement (++x, --x), which are right associative.
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_prefix(parser_data &data)
{
    // prefix = ('++' | '--')...? postfix_access
    // Increment and decrement are right associative
    if (match_and_consume(data, token_type::INCREMENT)) // Prefix increment
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> operand_result = parse_prefix(data);
        expression *right = operand_result.ast;

        return {new unary_expression(op, right), operand_result.error};
    }
    else if (match_and_consume(data, token_type::DECREMENT)) // Prefix decrement
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> operand_result = parse_prefix(data);
        expression *right = operand_result.ast;

        return {new unary_expression(op, right), operand_result.error};
    }

    return parse_postfix_access(data);
}

/**
 * Parses logical NOT and negation, which are right associative.
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_negations(parser_data &data)
{
    // negations = ('!' | '-') (negations | prefix)
    if (match_and_consume(data, {token_type::NOT, token_type::SUBTRACT}))
    {
        token op = data.tokens[data.position - 1];
        parse_result<expression> operand_result = parse_negations(data);
        expression *right = operand_result.ast;

        return {new unary_expression(op, right), operand_result.error};
    }

    return parse_prefix(data);
}

/**
 * Parses multiplication and division
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_factor(parser_data &data)
{
    // factor = negations (('*' | '/') negations)...?
    return parse_left_associative(data, parse_negations, {token_type::MULTIPLY, token_type::DIVIDE});
}

/**
 * Parses addition and subtraction
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_term(parser_data &data)
{
    // term = factor (('+' | '-') factor)...?
    return parse_left_associative(data, parse_factor, {token_type::ADD, token_type::SUBTRACT});
}

/**
 * Parses comparison operators (>, >=, <, <=)
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_comparison(parser_data &data)
{
    // comparison = term (('>' | '>=' | '<' | '<=') term)...?
    return parse_left_associative(data, parse_term, {token_type::GREATER, token_type::GREATER_EQUAL, token_type::LESS, token_type::LESS_EQUAL});
}

/**
 * Parses equality operators (== and !=)
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_equality(parser_data &data)
{
    // equality = comparison (('==' | '!=') comparison)...?
    return parse_left_associative(data, parse_comparison, {token_type::NOT_EQUAL, token_type::EQUAL});
}

/**
 * Parses logical and expressions
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_logical_and(parser_data &data)
{
    // logical_and = equality ('&&' equality)...?
    return parse_left_associative(data, parse_equality, {token_type::AND});
}

/**
 * Parses logical or expressions
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_logical_or(parser_data &data)
{
    // logical_or = logical_and ('||' logical_and)...?
    return parse_left_associative(data, parse_logical_and, {token_type::OR});
}

/**
 * Parses direct assignment with the "=" operator, which is right associative.
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_direct_assignment(parser_data &data)
{
    // direct_assignment = logical_or (= direct_assignment)?
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

/**
 * Parses compound assignment operators (+=, -=, *=, /=) which are right associative.
 * @param data Parser data
 * @return Expression parsed up to this level of precedence, and error status.
 */
parse_result<expression> parse_compound_assignment(parser_data &data)
{
    // compound_assignment = direct_assignment (('+=' | '-=' | '*=' | '/=') compound_assignment)?
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

/**
 * Parses an expression.
 * @param data Parser data
 * @return Parsed expression, and error status.
 */
parse_result<expression> parse_expression(parser_data &data)
{
    // expression = compound_assignment
    return parse_compound_assignment(data);
}

/**
 * Parses a variable declaration statement
 * @data Parser data, with the "var" token already consumed.
 * @return A declaration statement, and error status.
 */
parse_result<statement> parse_declaration(parser_data &data)
{
    // declaration = 'var' identifier ('=' expression)? ';'
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

/**
 * Parses an expression followed by a semicolon into an expression statement.
 * @param data Parser data
 * @return An expression statement, and error status.
 */
parse_result<statement> parse_expression_statement(parser_data &data)
{
    // expression_statement = expression ';'
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

/**
 * Parses a for statement
 * @data Parser data, with the "for" token already consumed.
 * @return A for statement, and error status.
 */
parse_result<statement> parse_for_statement(parser_data &data)
{
    // for_statement = 'for' '(' (statement | ';') expression? ';' expression? ')' statement
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

/**
 * Parses a while statement, with the "while" token already consumed.
 * @param data Parser data, with the "while" token already consumed.
 * @return A loop statement with is_do_while = false, and error status.
 */
parse_result<statement> parse_while_statement(parser_data &data)
{
    // while_statement = 'while' expression statement
    parse_result<expression> condition_result = parse_expression(data);
    bool error = condition_result.error;
    int line_number = condition_result.ast->get_line_number();
    int start_position = condition_result.ast->get_start_position();

    parse_result<statement> body_result = parse_statement(data);
    error |= body_result.error;
    int end_position = body_result.ast->get_end_position();

    return {new loop_statement(condition_result.ast, body_result.ast, false, line_number, start_position, end_position), error};
}

/**
 * Parses a do-while statement, with the "do" token already consumed.
 * @param data Parser data, with the "do" token already consumed.
 * @return A loop statement with is_do_while = true, and error status.
 */
parse_result<statement> parse_do_statement(parser_data &data)
{
    // do_statement = 'do' statement 'while' expression ';'
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

/**
 * Parses an if statement, with optional else branch.
 * @param data Parser data, with the "if" token already consumed.
 * @return An if statement, and error status.
 */
parse_result<statement> parse_if_statement(parser_data &data)
{
    // if_statement = 'if' expression statement ('else' statement)?
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

/**
 * Parses a sequence of statements enclosed in {} braces.
 * @param data Parser data, with the left brace already consumed.
 * @return A sequence statement containing the parsed statements, and error status.
 */
parse_result<statement> parse_braced_sequence(parser_data &data)
{
    // braced_sequence = '{' statement* '}'
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

/**
 * Parses a function declaration.
 * @param data Parser data, with the "func" token already consumed.
 * @return A declaration statement with the inner expression being a function expression, and error status.
 */
parse_result<statement> parse_function_declaration(parser_data &data)
{
    // var keyword is optional for parameters
    // trailing comma is allowed
    // function_declaration = 'function' identifier '(' ( 'var'? identifier (',' 'var'? identifier)* ','? )? ')' statement
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

/**
 * Parses a return statement.
 * @param data Parser data, with the return token already consumed.
 * @return A return statement, and error status.
 */
parse_result<statement> parse_return_statement(parser_data &data)
{
    // return_statement = 'return' expression? ';'
    if (match_and_consume(data, token_type::END_STATEMENT))
    {
        const token &tok = data.tokens[data.position - 2];
        // return without value
        return {new return_statement(nullptr, tok.get_line_number(), tok.get_start_position(), tok.get_end_position()), false};
    }
    else
    {
        const token &tok = data.tokens[data.position - 1];

        // return with value, consume and expect an expression
        parse_result<expression> return_expr = parse_expression(data);
        bool error = return_expr.error;
        int end = return_expr.ast->get_end_position();

        if (!match_and_consume(data, token_type::END_STATEMENT))
        {
            write_expectation_error_message(data, "';' here");
            delete return_expr.ast;
            return {new error_statement(), true};
        }

        return {new return_statement(return_expr.ast, tok.get_line_number(), tok.get_start_position(), end), error};
    }
}

/**
 * Parses a break statement.
 * @param data Parser data, with the break token already consumed.
 * @return A break statement, and error status.
 */
parse_result<statement> parse_break_statement(parser_data &data)
{
    // break_statement = 'break' ';'
    const token &tok = data.tokens[data.position - 1];
    if (!match_and_consume(data, token_type::END_STATEMENT))
    {
        write_expectation_error_message(data, "';' here");
        return {new error_statement(), true};
    }
    return {new break_statement(tok.get_line_number(), tok.get_start_position(), data.tokens[data.position - 1].get_end_position()), false};
}

/**
 * Parses a continue statement.
 * @param data Parser data, with the continue token already consumed.
 * @return A continue statement, and error status.
 */
parse_result<statement> parse_continue_statement(parser_data &data)
{
    // continue_statement = 'continue' ';'
    const token &tok = data.tokens[data.position - 1];
    if (!match_and_consume(data, token_type::END_STATEMENT))
    {
        write_expectation_error_message(data, "';' here");
        return {new error_statement(), true};
    }
    return {new continue_statement(tok.get_line_number(), tok.get_start_position(), data.tokens[data.position - 1].get_end_position()), false};
}

/**
 * Parses an individual statement.
 * @param allow_declarations Whether to allow variable or function declarations in this statement.
 * @return Parsed statement, and error status.
 */
parse_result<statement> parse_statement(parser_data &data, bool allow_declarations)
{
    // statement = return_statement | ... | expression_statement
    if (data.position >= (int)data.tokens.size())
    {
        write_error_message(data, "Unexpected end of input, expected statement");
        return {new error_statement(), true};
    }

    switch (data.tokens[data.position].get_type())
    {
    case token_type::RETURN:
        data.position++;
        return parse_return_statement(data);
    case token_type::BREAK:
        data.position++;
        return parse_break_statement(data);
    case token_type::CONTINUE:
        data.position++;
        return parse_continue_statement(data);
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

/**
 * Parses a sequence of statements.
 * @return A sequence statement, and error status.
 */
parse_result<statement> parse_sequence(parser_data &data)
{
    // sequence = statement*
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

/**
 * The top level parser function.
 * @return A sequence statement, and error status.
 */
parse_result<statement> parse_module(parser_data &data)
{
    return parse_sequence(data);
}

parse_result<statement> parse(std::vector<variable_block *> &literal_variables, const std::vector<token> &tokens, const std::string &source_string, const std::vector<source_line> &lines, std::ostream &error_stream)
{
    // Special case : empty input, single empty sequence statement is returned, and no error is reported
    if (tokens.empty())
    {
        return {new sequence_statement(), false};
    }

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
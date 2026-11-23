#ifndef LENTUS_AST_HPP
#define LENTUS_AST_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "token.hpp"
#include "data.hpp"
#include "ast-node.hpp"

namespace lentus
{

class user_function;

/**
 * Base class for all expression AST nodes in the Lentus language.
 * Purely to differentiate expressions from statements to avoid bugs
 */
class expression : public ast_node
{
};

/**
 * Whenever a parse error is encountered, this expression is used.
 */
class error_expression : public expression
{
public:
    error_expression()
    {
        // If an error node exists then it cannot be executed anyway.
        this->line_number = 0;
        this->start_position = 0;
        this->end_position = 0;
    }

    error_expression(const error_expression &other) = delete;
    error_expression &operator=(const error_expression &other) = delete;

    reference evaluate(execution_context &execution) const override;
    std::string to_string() const override { return "<error>"; }
    ~error_expression() override = default;
};

class unary_expression : public expression
{
private:
    token op;
    expression *operand;
    bool is_postfix; /** true for var++ and var--, false for ++var and --var */

public:
    unary_expression(const token &op, expression *operand, bool is_postfix = false) : op(op), operand(operand), is_postfix(is_postfix)
    {
        this->line_number = op.get_line_number();
        this->start_position = op.get_start_position();
        this->end_position = operand->get_end_position();
    }

    unary_expression(const unary_expression &other) = delete;
    unary_expression &operator=(const unary_expression &other) = delete;

    unary_expression(unary_expression &&other);
    unary_expression &operator=(unary_expression &&other);

    reference evaluate(execution_context &execution) const override;

    token get_operator() const { return op; }
    expression *get_operand() const { return operand; }

    /**
     * For increment/decrement, returns true if the operator is a postfix operator (var++/var--)
     */
    bool is_postfix_operator() const { return is_postfix; }

    virtual std::string to_string() const override;

    ~unary_expression() override;
};

class binary_expression : public expression
{
private:
    token op;
    expression *left;
    expression *right;

public:
    binary_expression(const token &op, expression *left, expression *right) : op(op), left(left), right(right)
    {
        this->line_number = left->get_line_number();
        this->start_position = left->get_start_position();
        this->end_position = right->get_end_position();
    }

    binary_expression(const binary_expression &other) = delete;
    binary_expression &operator=(const binary_expression &other) = delete;

    binary_expression(binary_expression &&other);
    binary_expression &operator=(binary_expression &&other);

    reference evaluate(execution_context &execution) const override;

    token get_operator() const { return op; }
    expression *get_left() const { return left; }
    expression *get_right() const { return right; }

    virtual std::string to_string() const override;

    ~binary_expression() override;
};

/**
 * Represents an indexing operation, e.g. a[b]
 */
class subscript_expression : public expression
{
private:
    expression *left;
    expression *right;

public:
    subscript_expression(expression *lhs, expression *rhs, int line_number, int start_position, int end_position) : left(lhs), right(rhs)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    subscript_expression(const subscript_expression &other) = delete;
    subscript_expression &operator=(const subscript_expression &other) = delete;

    subscript_expression(subscript_expression &&other);
    subscript_expression &operator=(subscript_expression &&other);

    reference evaluate(execution_context &execution) const override;

    expression *get_left() const { return left; }
    expression *get_right() const { return right; }

    virtual std::string to_string() const override;

    ~subscript_expression() override;
};

/**
 * Represents a literal value as a value
 */
class literal_expression : public expression
{
private:
    value stored_value;

public:
    literal_expression(const value &val, int line_number, int start_position, int end_position) : stored_value(val)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }
    literal_expression(value &&val, int line_number, int start_position, int end_position) : stored_value(std::move(val))
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    /**
     * Create string literal
     */
    literal_expression(string *string, int line_number, int start_position, int end_position);

    /**
     * Create function literal
     */
    literal_expression(user_function *function, int line_number, int start_position, int end_position);

    literal_expression(const literal_expression &other) = delete;
    literal_expression &operator=(const literal_expression &other) = delete;

    literal_expression(literal_expression &&other);
    literal_expression &operator=(literal_expression &&other);

    reference evaluate(execution_context &execution) const override;

    value get_value() const { return stored_value; }

    virtual std::string to_string() const override;

    ~literal_expression() override;
};

/**
 * Expression in parentheses
 */
class grouping_expression : public expression
{
private:
    expression *inner_expression;

public:
    grouping_expression(expression *inner_expression, int line_number, int start_position, int end_position) : inner_expression(inner_expression)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    grouping_expression(const grouping_expression &other) = delete;
    grouping_expression &operator=(const grouping_expression &other) = delete;

    grouping_expression(grouping_expression &&other);
    grouping_expression &operator=(grouping_expression &&other);

    reference evaluate(execution_context &execution) const override;

    expression *get_inner_expression() const { return inner_expression; }

    virtual std::string to_string() const override;

    ~grouping_expression() override;
};

/**
 * Expression containing a name for a value.
 * Evaluating this expression returns a modifiable reference to the associated value.
 */
class identifier_expression : public expression
{
private:
    std::string identifier;

public:
    identifier_expression(const std::string &identifier, int line_number, int start_position, int end_position) : identifier(identifier)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    identifier_expression(std::string &&identifier, int line_number, int start_position, int end_position) : identifier(std::move(identifier))
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    identifier_expression(const identifier_expression &other) = delete;
    identifier_expression &operator=(const identifier_expression &other) = delete;

    identifier_expression(identifier_expression &&other);
    identifier_expression &operator=(identifier_expression &&other);

    reference evaluate(execution_context &execution) const override;
    const std::string &get_identifier() const { return identifier; }

    virtual std::string to_string() const override;
    ~identifier_expression() override = default;
};

/**
 * Represents a function call
 */
class call_expression : public expression
{
private:
    expression *callee;
    std::vector<expression *> arguments;

public:
    call_expression(expression *callee, std::vector<expression *> &&arguments, int line_number, int start_position, int end_position) : callee(callee), arguments(std::move(arguments))
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    call_expression(const call_expression &other) = delete;
    call_expression &operator=(const call_expression &other) = delete;

    call_expression(call_expression &&other);
    call_expression &operator=(call_expression &&other);

    reference evaluate(execution_context &execution) const override;
    const expression *get_callee() const { return callee; }
    const std::vector<expression *> &get_arguments() const { return arguments; }

    virtual std::string to_string() const override;
    ~call_expression() override;
};

/**
 * Represents a function literal.
 * For now, Lentus does not support anonymous functions, so this expression 
 * is only found alongside a value declaration statement.
 * Evaluates to a function value.
 */
class function_expression : public expression
{
private:
    string *name; // optional
    statement *body;
    std::vector<string *> parameter_names;

public:
    function_expression(string *name, statement *body, std::vector<string *> &&parameter_names, int line_number, int start_position, int end_position) : name(name), body(body), parameter_names(std::move(parameter_names))
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    function_expression(const function_expression &other) = delete;
    function_expression &operator=(const function_expression &other) = delete;

    function_expression(function_expression &&other);
    function_expression &operator=(function_expression &&other);

    /**
     * Evaluating a function expression returns a function value that captures the variables in the current scope.
     */
    reference evaluate(execution_context &execution) const override;
    const string *get_name() const { return name; }
    const statement *get_body() const { return body; }
    const std::vector<string *> &get_parameter_names() const { return parameter_names; }

    virtual std::string to_string() const override;
    ~function_expression() override;
};

} // namespace lentus

#endif

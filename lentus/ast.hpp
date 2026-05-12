#ifndef LENTUS_AST_HPP
#define LENTUS_AST_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include "token.hpp"
#include "variable.hpp"

class program;

class ast_node
{
protected:
    int line_number;
    int start_position;
    int end_position;

public:
    virtual variable *evaluate(program &program) const = 0;

    virtual ~ast_node() = default;

    virtual std::string to_string() const = 0;

    int get_line_number() const { return line_number; }
    int get_start_position() const { return start_position; }
    int get_end_position() const { return end_position; }
};

class error_node : public ast_node
{
public:
    error_node()
    {
        // If an error node exists then it cannot be executed anyway.
        this->line_number = 0;
        this->start_position = 0;
        this->end_position = 0;
    }

    error_node(const error_node &other) = delete;
    error_node &operator=(const error_node &other) = delete;

    variable *evaluate(program &program) const override;
    std::string to_string() const override { return "<error>"; }
    ~error_node() override = default;
};

class unary_expression : public ast_node
{
private:
    token op;
    ast_node *operand;

public:
    unary_expression(const token &op, ast_node *operand) : op(op), operand(operand)
    {
        this->line_number = op.get_line_number();
        this->start_position = op.get_start_position();
        this->end_position = operand->get_end_position();
    }

    unary_expression(const unary_expression &other) = delete;
    unary_expression &operator=(const unary_expression &other) = delete;

    unary_expression(unary_expression &&other);
    unary_expression &operator=(unary_expression &&other);

    variable *evaluate(program &program) const override;

    token get_operator() const { return op; }
    ast_node *get_operand() const { return operand; }

    virtual std::string to_string() const override;

    ~unary_expression() override;
};

class binary_expression : public ast_node
{
private:
    token op;
    ast_node *left;
    ast_node *right;

public:
    binary_expression(const token &op, ast_node *left, ast_node *right) : op(op), left(left), right(right)
    {
        this->line_number = left->get_line_number();
        this->start_position = left->get_start_position();
        this->end_position = right->get_end_position();
    }

    binary_expression(const binary_expression &other) = delete;
    binary_expression &operator=(const binary_expression &other) = delete;

    binary_expression(binary_expression &&other);
    binary_expression &operator=(binary_expression &&other);

    variable *evaluate(program &program) const override;

    token get_operator() const { return op; }
    ast_node *get_left() const { return left; }
    ast_node *get_right() const { return right; }

    virtual std::string to_string() const override;

    ~binary_expression() override;
};

class literal_expression : public ast_node
{
private:
    variable value;

public:
    literal_expression(const variable &value, int line_number, int start_position, int end_position) : value(value)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }
    literal_expression(variable &&value, int line_number, int start_position, int end_position) : value(std::move(value))
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    literal_expression(const literal_expression &other) = delete;
    literal_expression &operator=(const literal_expression &other) = delete;

    literal_expression(literal_expression &&other);
    literal_expression &operator=(literal_expression &&other);

    variable *evaluate(program &program) const override;

    variable get_value() const { return value; }

    virtual std::string to_string() const override;

    ~literal_expression() override;
};

class grouping_expression : public ast_node
{
private:
    ast_node *inner_expression;

public:
    grouping_expression(ast_node *inner_expression, int line_number, int start_position, int end_position) : inner_expression(inner_expression)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    grouping_expression(const grouping_expression &other) = delete;
    grouping_expression &operator=(const grouping_expression &other) = delete;

    grouping_expression(grouping_expression &&other);
    grouping_expression &operator=(grouping_expression &&other);

    variable *evaluate(program &program) const override;

    ast_node *get_inner_expression() const { return inner_expression; }

    virtual std::string to_string() const override;

    ~grouping_expression() override;
};

#endif
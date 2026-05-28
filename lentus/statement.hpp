#ifndef LENTUS_STATEMENT_HPP
#define LENTUS_STATEMENT_HPP

#include "ast-node.hpp"
#include "variable.hpp"
#include "expression.hpp"

class statement : public ast_node
{
};

class error_statement : public statement
{
public:
    error_statement()
    {
        // If an error node exists then it cannot be executed anyway.
        this->line_number = 0;
        this->start_position = 0;
        this->end_position = 0;
    }

    error_statement(const error_statement &other) = delete;
    error_statement &operator=(const error_statement &other) = delete;

    reference evaluate(execution_context &execution) const override;
    std::string to_string() const override { return "<error>\n"; }
    ~error_statement() override = default;
};

class declaration_statement : public statement
{
private:
    variable identifier;
    expression *value; /** nullptr if not initialized */

public:
    declaration_statement(lentus_string *identifier, expression *value, int line_number, int start_position, int end_position) : identifier(identifier), value(value)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    declaration_statement(const declaration_statement &other) = delete;
    declaration_statement &operator=(const declaration_statement &other) = delete;

    declaration_statement(declaration_statement &&other);
    declaration_statement &operator=(declaration_statement &&other);

    const lentus_string &get_identifier() const { return *identifier.get_string(); }
    expression *get_value() const { return value; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~declaration_statement() override;
};

/**
 * Statement with one expression
 */
class expression_statement : public statement
{
private:
    expression *expr;

public:
    expression_statement(expression *expr) : expr(expr)
    {
        this->line_number = expr->get_line_number();
        this->start_position = expr->get_start_position();
        this->end_position = expr->get_end_position();
    }

    expression_statement(const expression_statement &other) = delete;
    expression_statement &operator=(const expression_statement &other) = delete;

    expression_statement(expression_statement &&other);
    expression_statement &operator=(expression_statement &&other);

    expression *get_expression() const { return expr; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~expression_statement() override;
};

class if_statement : public statement
{
private:
    expression *condition;
    statement *then_branch;
    statement *else_branch;

public:
    if_statement(expression *condition, statement *then_branch, statement *else_branch, int line_number, int start_position, int end_position) : condition(condition), then_branch(then_branch), else_branch(else_branch)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    if_statement(expression *condition, statement *then_branch, int line_number, int start_position, int end_position) : condition(condition), then_branch(then_branch), else_branch(nullptr)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    if_statement(const if_statement &other) = delete;
    if_statement &operator=(const if_statement &other) = delete;

    if_statement(if_statement &&other);
    if_statement &operator=(if_statement &&other);

    expression *get_condition() const { return condition; }
    statement *get_then_branch() const { return then_branch; }
    statement *get_else_branch() const { return else_branch; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~if_statement() override;
};

class for_statement : public statement
{
private:
    statement *var_initializer;
    expression *condition;
    expression *post_iteration;
    statement *body;

public:
    for_statement(statement *var_initializer, expression *condition, expression *post_iteration, statement *body, int line_number, int start_position, int end_position) : var_initializer(var_initializer), condition(condition), post_iteration(post_iteration), body(body)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    for_statement(const for_statement &other) = delete;
    for_statement &operator=(const for_statement &other) = delete;

    for_statement(for_statement &&other);
    for_statement &operator=(for_statement &&other);

    statement *get_var_initializer() const { return var_initializer; }
    expression *get_condition() const { return condition; }
    expression *get_post_iteration() const { return post_iteration; }
    statement *get_body() const { return body; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~for_statement() override;
};

class loop_statement : public statement
{
private:
    expression *condition;
    statement *body;
    bool is_do_loop;

public:
    loop_statement(expression *condition, statement *body, bool is_do_loop, int line_number, int start_position, int end_position) : condition(condition), body(body), is_do_loop(is_do_loop)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    loop_statement(const loop_statement &other) = delete;
    loop_statement &operator=(const loop_statement &other) = delete;

    loop_statement(loop_statement &&other);
    loop_statement &operator=(loop_statement &&other);

    expression *get_condition() const { return condition; }
    statement *get_body() const { return body; }
    bool get_is_do_loop() const { return is_do_loop; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~loop_statement() override;
};

class sequence_statement : public statement
{
private:
    std::vector<statement *> statements;

public:
    sequence_statement() : statements({}) {}
    sequence_statement(std::vector<statement *> &&statements);

    sequence_statement(const sequence_statement &other) = delete;
    sequence_statement &operator=(const sequence_statement &other) = delete;

    sequence_statement(sequence_statement &&other);
    sequence_statement &operator=(sequence_statement &&other);

    const std::vector<statement *> &get_statements() const { return statements; }

    /**
     * @return The value of the last statement in the sequence, or None if the sequence is empty
     */
    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~sequence_statement() override;
};

class empty_statement : public statement
{
public:
    empty_statement() {}

    empty_statement(const empty_statement &other) = delete;
    empty_statement &operator=(const empty_statement &other) = delete;

    empty_statement(empty_statement &&other) = default;
    empty_statement &operator=(empty_statement &&other) = default;

    /**
     * @return Unmodifiable reference to none
     */
    reference evaluate([[maybe_unused]] execution_context &execution) const override { return reference(); };
    virtual std::string to_string() const override { return "NONE"; };

    ~empty_statement() override = default;
};

class return_statement : public statement
{
private:
    expression *value; /** nullptr if no return value */

public:
    return_statement(expression *value, int line_number, int start_position, int end_position) : value(value)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    return_statement(const return_statement &other) = delete;
    return_statement &operator=(const return_statement &other) = delete;

    return_statement(return_statement &&other);
    return_statement &operator=(return_statement &&other);

    expression *get_value() const { return value; }

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~return_statement() override;
};

class break_statement : public statement
{
public:
    break_statement(int line_number, int start_position, int end_position)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    break_statement(const break_statement &other) = delete;
    break_statement &operator=(const break_statement &other) = delete;

    break_statement(break_statement &&other) = default;
    break_statement &operator=(break_statement &&other) = default;

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~break_statement() override = default;
};

class continue_statement : public statement
{
public:
    continue_statement(int line_number, int start_position, int end_position)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    continue_statement(const continue_statement &other) = delete;
    continue_statement &operator=(const continue_statement &other) = delete;

    continue_statement(continue_statement &&other) = default;
    continue_statement &operator=(continue_statement &&other) = default;

    reference evaluate(execution_context &execution) const override;
    virtual std::string to_string() const override;

    ~continue_statement() override = default;
};
#endif
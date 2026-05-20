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
    std::string identifier;
    expression *value; /** nullptr if not initialized */

public:
    declaration_statement(const std::string &identifier, expression *value, int line_number, int start_position, int end_position) : identifier(identifier), value(value)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    declaration_statement(const std::string &&identifier, expression *value, int line_number, int start_position, int end_position) : identifier(std::move(identifier)), value(value)
    {
        this->line_number = line_number;
        this->start_position = start_position;
        this->end_position = end_position;
    }

    declaration_statement(const declaration_statement &other) = delete;
    declaration_statement &operator=(const declaration_statement &other) = delete;

    declaration_statement(declaration_statement &&other);
    declaration_statement &operator=(declaration_statement &&other);

    const std::string &get_identifier() const { return identifier; }
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

#endif
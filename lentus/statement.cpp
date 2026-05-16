#include "statement.hpp"

declaration_statement::declaration_statement(declaration_statement &&other)
{
    identifier = std::move(other.identifier);
    value = other.value;
    other.value = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

declaration_statement &declaration_statement::operator=(declaration_statement &&other)
{
    if (this != &other)
    {
        delete value;
        identifier = std::move(other.identifier);
        value = other.value;
        other.value = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

declaration_statement::~declaration_statement()
{
    delete value;
}

std::string declaration_statement::to_string() const
{
    if (value)
    {
        return "DECLARE " + identifier + " = " + value->to_string() + "\n";
    }
    else
    {
        return "DECLARE " + identifier + "\n";
    }
}

expression_statement::expression_statement(expression_statement &&other)
{
    expr = other.expr;
    other.expr = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

expression_statement &expression_statement::operator=(expression_statement &&other)
{
    if (this != &other)
    {
        delete expr;
        expr = other.expr;
        other.expr = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

expression_statement::~expression_statement()
{
    delete expr;
}

std::string expression_statement::to_string() const
{
    return "EXPR " + expr->to_string() + "\n";
}

sequence_statement::sequence_statement(std::vector<statement *> &&statements)
{
    this->statements = std::move(statements);
    this->line_number = statements.empty() ? -1 : statements[0]->get_line_number();
    this->start_position = statements.empty() ? -1 : statements[0]->get_start_position();
    this->end_position = statements.empty() ? -1 : statements.back()->get_end_position();
}

sequence_statement::sequence_statement(sequence_statement &&other)
{
    statements = std::move(other.statements);
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

sequence_statement &sequence_statement::operator=(sequence_statement &&other)
{
    if (this != &other)
    {
        for (statement *stmt : statements)
        {
            delete stmt;
        }
        statements = std::move(other.statements);
    }
    return *this;
}

sequence_statement::~sequence_statement()
{
    for (statement *stmt : statements)
    {
        delete stmt;
    }
}

std::string sequence_statement::to_string() const
{
    std::string result;
    for (statement *stmt : statements)
    {
        result += stmt->to_string();
    }
    return result;
}

variable *error_statement::evaluate(program &program) const
{
    return nullptr;
}

variable *declaration_statement::evaluate(program &program) const
{
    return nullptr; // TODO
}

variable *expression_statement::evaluate(program &program) const
{
    return expr->evaluate(program);
}

variable *sequence_statement::evaluate(program &program) const
{
    variable *result = nullptr;
    for (statement *stmt : statements)
    {
        variable *stmt_result = stmt->evaluate(program);
        if (stmt_result)
        {
            result = stmt_result;
        }
    }
    return result;
}
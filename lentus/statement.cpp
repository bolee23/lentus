#include "statement.hpp"
#include "execution.hpp"

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

variable *error_statement::evaluate(execution_context &execution) const
{
    throw lentus_runtime_exception("Cannot evaluate error_node", this);
}

variable *expression_statement::evaluate(execution_context &execution) const
{
    return expr->evaluate(execution);
}

variable *sequence_statement::evaluate(execution_context &execution) const
{
    variable *result = nullptr;
    execution.push();

    for (statement *stmt : statements)
    {
        result = stmt->evaluate(execution);
    }

    execution.pop();

    return result;
}

variable *declaration_statement::evaluate(execution_context &execution) const
{
    stack_frame &frame = execution.peek();
    if (frame.local_variables.contains(identifier))
    { // Already defined in this scope
        throw lentus_runtime_exception("Variable " + identifier + " already defined in this scope", this);
    }
    else
    {
        if (value == nullptr)
        {
            frame.local_variables[identifier] = execution.get_program()->get_none_variable();
        }
        else
        {
            frame.local_variables[identifier] = value->evaluate(execution);
        }
    }
    return frame.local_variables[identifier];
}
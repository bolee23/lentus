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

if_statement::if_statement(if_statement &&other)
{
    condition = other.condition;
    then_branch = other.then_branch;
    else_branch = other.else_branch;
    other.condition = nullptr;
    other.then_branch = nullptr;
    other.else_branch = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

if_statement &if_statement::operator=(if_statement &&other)
{
    if (this != &other)
    {
        delete condition;
        delete then_branch;
        delete else_branch;

        condition = other.condition;
        then_branch = other.then_branch;
        else_branch = other.else_branch;
        other.condition = nullptr;
        other.then_branch = nullptr;
        other.else_branch = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

if_statement::~if_statement()
{
    delete condition;
    delete then_branch;
    delete else_branch;
}

std::string if_statement::to_string() const
{
    std::string result = "IF[" + condition->to_string() + ",THEN{\n" + then_branch->to_string() + "}";
    if (else_branch)
    {
        result += "ELSE{\n" + else_branch->to_string() + "}]\n";
    }
    else
    {
        result += "]\n";
    }
    return result;
}

for_statement::for_statement(for_statement &&other)
{
    var_initializer = other.var_initializer;
    condition = other.condition;
    post_iteration = other.post_iteration;
    body = other.body;
    other.var_initializer = nullptr;
    other.condition = nullptr;
    other.post_iteration = nullptr;
    other.body = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

for_statement &for_statement::operator=(for_statement &&other)
{
    if (this != &other)
    {
        delete var_initializer;
        delete condition;
        delete post_iteration;
        delete body;

        var_initializer = other.var_initializer;
        condition = other.condition;
        post_iteration = other.post_iteration;
        body = other.body;
        other.var_initializer = nullptr;
        other.condition = nullptr;
        other.post_iteration = nullptr;
        other.body = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

for_statement::~for_statement()
{
    delete var_initializer;
    delete condition;
    delete post_iteration;
    delete body;
}

std::string for_statement::to_string() const
{
    return "FOR[" + (var_initializer ? var_initializer->to_string() : "NONE") + ", " + condition->to_string() + ", " + (post_iteration ? post_iteration->to_string() : "NONE") + " DO\n" + body->to_string() + "]\n";
}

loop_statement::loop_statement(loop_statement &&other)
{
    condition = other.condition;
    body = other.body;
    is_do_loop = other.is_do_loop;
    other.condition = nullptr;
    other.body = nullptr;
    other.is_do_loop = false;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

loop_statement &loop_statement::operator=(loop_statement &&other)
{
    if (this != &other)
    {
        delete condition;
        delete body;

        condition = other.condition;
        body = other.body;
        is_do_loop = other.is_do_loop;
        other.condition = nullptr;
        other.body = nullptr;
        other.is_do_loop = false;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

loop_statement::~loop_statement()
{
    delete condition;
    delete body;
}

std::string loop_statement::to_string() const
{
    if (is_do_loop)
    {
        return "DO[" + body->to_string() + " WHILE\n" + condition->to_string() + "]\n";
    }
    else
    {
        return "WHILE[" + condition->to_string() + " DO\n" + body->to_string() + "]\n";
    }
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

reference error_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    throw lentus_runtime_exception("Cannot evaluate error_node", this);
}

reference expression_statement::evaluate(execution_context &execution) const
{
    return expr->evaluate(execution);
}

reference sequence_statement::evaluate(execution_context &execution) const
{
    reference result;
    execution.push();

    for (statement *stmt : statements)
    {
        result = stmt->evaluate(execution);
    }

    execution.pop();

    return result;
}

reference declaration_statement::evaluate(execution_context &execution) const
{
    lentus_string lentus_identifier(this->identifier);

    stack_frame &frame = execution.peek();
    if (frame.local_variables->contains(variable(&lentus_identifier)))
    { // Already defined in this scope
        throw lentus_runtime_exception("Variable " + *lentus_identifier + " already defined in this scope", this);
    }
    else
    {
        // Dynamically allocate new variable and track it.
        variable key = execution.get_program().create_string(new lentus_string(std::move(lentus_identifier)));

        if (value == nullptr)
        {
            frame.local_variables->operator[](key) = variable();
            return variable();
        }
        else
        {
            variable &result = *value->evaluate(execution);
            frame.local_variables->operator[](key) = result;
            return result;
        }
    }
}

reference if_statement::evaluate(execution_context &execution) const
{
    variable condition_result = *condition->evaluate(execution);
    if (!condition_result.is(variable_type::BOOLEAN))
    {
        throw lentus_runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
    }

    if (condition_result.get_boolean())
    {
        return then_branch->evaluate(execution);
    }
    else if (else_branch != nullptr)
    {
        return else_branch->evaluate(execution);
    }
    else
    {
        return variable();
    }
}

reference for_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    execution.push();
    if (var_initializer)
    {
        var_initializer->evaluate(execution);
    }

    variable last_value = variable();

    while (true)
    {
        // Check condition
        variable condition_result = *condition->evaluate(execution);
        if (!condition_result.is(variable_type::BOOLEAN))
        {
            throw lentus_runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
        }
        if (!condition_result.get_boolean())
        {
            break;
        }

        last_value = *body->evaluate(execution);
        if (post_iteration)
        {
            post_iteration->evaluate(execution);
        }
    }

    execution.pop();
    return last_value;
}

reference loop_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    variable last_value = variable();

    if (is_do_loop)
    {
        while (true)
        {
            last_value = *body->evaluate(execution);

            // Check condition
            variable condition_result = *condition->evaluate(execution);
            if (!condition_result.is(variable_type::BOOLEAN))
            {
                throw lentus_runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
            }
            if (!condition_result.get_boolean())
            {
                break;
            }
        }
    }
    else
    {
        while (true)
        {
            // Check condition
            variable condition_result = *condition->evaluate(execution);
            if (!condition_result.is(variable_type::BOOLEAN))
            {
                throw lentus_runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
            }
            if (!condition_result.get_boolean())
            {
                break;
            }

            last_value = *body->evaluate(execution);
        }
    }

    return last_value;
}
#include "statement.hpp"
#include "execution.hpp"

namespace lentus
{

declaration_statement::declaration_statement(declaration_statement &&other)
{
    identifier = std::move(other.identifier);
    initializer = other.initializer;
    other.initializer = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

declaration_statement &declaration_statement::operator=(declaration_statement &&other)
{
    if (this != &other)
    {
        delete initializer;
        identifier = std::move(other.identifier);
        initializer = other.initializer;
        other.initializer = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

declaration_statement::~declaration_statement()
{
    delete initializer;
}

std::string declaration_statement::to_string() const
{
    if (initializer)
    {
        return "DECLARE " + identifier.get_string()->get_value() + " = " + initializer->to_string() + "\n";
    }
    else
    {
        return "DECLARE " + identifier.get_string()->get_value() + "\n";
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

return_statement::return_statement(return_statement &&other)
{
    value = other.value;
    other.value = nullptr;
    line_number = other.line_number;
    start_position = other.start_position;
    end_position = other.end_position;
}

return_statement &return_statement::operator=(return_statement &&other)
{
    if (this != &other)
    {
        delete value;
        value = other.value;
        other.value = nullptr;
        line_number = other.line_number;
        start_position = other.start_position;
        end_position = other.end_position;
    }
    return *this;
}

return_statement::~return_statement()
{
    delete value;
}

std::string return_statement::to_string() const
{
    if (value)
    {
        return "RETURN " + value->to_string() + "\n";
    }
    else
    {
        return "RETURN\n";
    }
}

std::string break_statement::to_string() const
{
    return "BREAK\n";
}

std::string continue_statement::to_string() const
{
    return "CONTINUE\n";
}

reference error_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    throw runtime_exception("Cannot evaluate error_node", this);
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
    if (execution.peek().local_variables->contains(this->identifier))
    { // Already defined in this scope
        throw runtime_exception("Variable " + *this->get_identifier() + " already defined in this scope", this);
    }
    else
    {
        if (initializer == nullptr)
        {
            execution.peek().local_variables->add(this->identifier, value());
            return value();
        }
        else
        {
            reference ref = initializer->evaluate(execution);
            lentus::value result = *ref;
            execution.peek().local_variables->add(this->identifier, result);
            return result;
        }
    }
}

reference if_statement::evaluate(execution_context &execution) const
{
    value condition_result = *condition->evaluate(execution);
    if (!condition_result.is(value_type::BOOLEAN))
    {
        throw runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
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
        return value();
    }
}

reference for_statement::evaluate(execution_context &execution) const
{
    execution.push();
    if (var_initializer)
    {
        var_initializer->evaluate(execution);
    }

    value last_value = value();

    int break_marker = execution.stack_position();
    try // break
    {
        while (true)
        {
            // Check condition
            value condition_result = *condition->evaluate(execution);
            if (!condition_result.is(value_type::BOOLEAN))
            {
                throw runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
            }
            if (!condition_result.get_boolean())
            {
                break;
            }

            int continue_marker = execution.stack_position();
            try // continue
            {
                last_value = *body->evaluate(execution);
            }
            catch (const continue_signal &)
            {
                execution.stack_synchronize(continue_marker);
            }

            // post iteration regardless of continue or not
            if (post_iteration)
            {
                post_iteration->evaluate(execution);
            }
        }

        execution.pop();
        return last_value;
    }
    catch (const break_signal &)
    {
        execution.stack_synchronize(break_marker);
        execution.pop();
        return value();
    }
}

reference loop_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    value last_value = value();

    if (is_do_loop)
    {
        int break_marker = execution.stack_position();
        try // break
        {
            while (true)
            {
                int continue_marker = execution.stack_position();
                try // continue
                {
                    last_value = *body->evaluate(execution);
                }
                catch (const continue_signal &)
                {
                    execution.stack_synchronize(continue_marker);
                }

                // Check condition
                value condition_result = *condition->evaluate(execution);
                if (!condition_result.is(value_type::BOOLEAN))
                {
                    throw runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
                }
                if (!condition_result.get_boolean())
                {
                    break;
                }
            }
            return last_value;
        }
        catch (const break_signal &)
        {
            execution.stack_synchronize(break_marker);
            return value();
        }
    }
    else
    {
        int break_marker = execution.stack_position();
        try // break
        {
            while (true)
            {
                // Check condition
                value condition_result = *condition->evaluate(execution);
                if (!condition_result.is(value_type::BOOLEAN))
                {
                    throw runtime_exception(std::string("Condition expression must evaluate to a boolean type, got ") + condition_result.get_type_name(), condition);
                }
                if (!condition_result.get_boolean())
                {
                    break;
                }

                int continue_marker = execution.stack_position();
                try // continue
                {
                    last_value = *body->evaluate(execution);
                }
                catch (const continue_signal &)
                {
                    execution.stack_synchronize(continue_marker);
                }
            }
            return last_value;
        }
        catch (const break_signal &)
        {
            execution.stack_synchronize(break_marker);
            return value();
        }
    }
}

reference return_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    lentus::value return_value = value ? *value->evaluate(execution) : lentus::value();
    throw return_signal(return_value);
}

reference break_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    throw break_signal();
}

reference continue_statement::evaluate([[maybe_unused]] execution_context &execution) const
{
    throw continue_signal();
}
} // namespace lentus

#include "function.hpp"
#include <iostream>
#include "program.hpp"
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());

void validate_num_arguments(function &func, std::vector<variable> &parameters, size_t expected)
{
    if (parameters.size() != expected)
    {
        throw lentus_runtime_exception("Invalid number of arguments for function " + func.to_string() + ", expected " + std::to_string(expected));
    }
}

variable print_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    for (int i = 0; i < (int)parameters.size(); i++)
    {
        std::cout << parameters[i].to_string();
    }
    return variable();
}

variable print_line_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    for (int i = 0; i < (int)parameters.size(); i++)
    {
        std::cout << parameters[i].to_string();
    }
    std::cout << std::endl;
    return variable();
}

variable number_cast_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1);

    if (parameters[0].is(variable_type::NUMBER))
    {
        return parameters[0];
    }
    else if (parameters[0].is(variable_type::BOOLEAN))
    {
        return variable(parameters[0].get_boolean() ? 1 : 0);
    }
    else if (parameters[0].is(variable_type::STRING))
    {
        const std::string &str = parameters[0].get_string()->get_value();
        try
        {
            size_t idx;
            double number = std::stod(str, &idx);
            if (idx != str.size())
            {
                throw lentus_runtime_exception("Invalid string format for number cast: extra characters after number", executor);
            }
            return variable(number);
        }
        catch (const std::invalid_argument &)
        {
            throw lentus_runtime_exception("Invalid string format for number cast: not a valid number", executor);
        }
        catch (const std::out_of_range &)
        {
            throw lentus_runtime_exception("Invalid string format for number cast: number out of range", executor);
        }
    }
    else
    {
        throw lentus_runtime_exception(std::string("Cannot cast type ") + parameters[0].get_type_name() + " to NUMBER", executor);
    }
}

variable is_number_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1);

    switch (parameters[0].get_type())
    {
    case variable_type::NUMBER:
        return variable(true);
    case variable_type::BOOLEAN:
    case variable_type::LIST:
    case variable_type::TABLE:
    case variable_type::FUNCTION:
    case variable_type::NONE:
        return variable(false);
    case variable_type::STRING:
        try
        {
            const std::string &str = parameters[0].get_string()->get_value();
            size_t idx;
            std::stod(str, &idx);
            return variable(idx == str.size());
        }
        catch (const std::invalid_argument &)
        {
            return variable(false);
        }
        catch (const std::out_of_range &)
        {
            return variable(false);
        }
    default:
        throw lentus_runtime_exception(std::string("Unknown type ") + parameters[0].get_type_name(), executor);
    }
}

variable is_integer_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1);

    switch (parameters[0].get_type())
    {
    case variable_type::NUMBER:
        return std::floor(parameters[0].get_number()) == parameters[0].get_number();
    case variable_type::BOOLEAN:
    case variable_type::LIST:
    case variable_type::TABLE:
    case variable_type::FUNCTION:
    case variable_type::NONE:
        return variable(false);
    case variable_type::STRING:
        try
        {
            const std::string &str = parameters[0].get_string()->get_value();
            size_t idx;
            double number = std::stod(str, &idx);
            return variable(idx == str.size() && number == std::floor(number));
        }
        catch (const std::invalid_argument &)
        {
            return variable(false);
        }
        catch (const std::out_of_range &)
        {
            return variable(false);
        }
    default:
        throw lentus_runtime_exception(std::string("Unknown type ") + parameters[0].get_type_name(), executor);
    }
}

variable string_cast_function::evaluate(execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1);
    return execution.get_program().create_string(parameters[0].to_string());
}

variable lentus_function::evaluate(execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    execution.push_new_function(this);

    // add parameters to the new stack frame
    for (size_t i = 0; i < parameter_names->size(); i++)
    {
        variable param_value = variable((*parameter_names)[i]);
        execution.peek().local_variables->add(param_value, parameters[i]);
    }

    int stack_pos = execution.stack_position();
    try
    {
        // execute the function body
        body->evaluate(execution).get();
        execution.pop();
        return variable(); // Functions return none by default, not the last value in the sequence.
    }
    catch (const return_signal &signal) // Catch return
    {
        execution.stack_synchronize(stack_pos);
        execution.pop();
        return signal.get_return_value();
    }
}

lentus_function &lentus_function::operator=(lentus_function &&other)
{
    if (this != &other)
    {
        body = other.body;
        other.body = nullptr;
        name = other.name;
        parameter_names = other.parameter_names;
        captured_variable_tables = other.captured_variable_tables;
    }
    return *this;
}

lentus_function::lentus_function(lentus_function &&other)
{
    body = other.body;
    other.body = nullptr;
    name = other.name;
    parameter_names = other.parameter_names;
    captured_variable_tables = other.captured_variable_tables;
}

variable input_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    std::string input;
    std::cin >> input;
    return execution.get_program().create_string(input);
}

variable rand_int_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 2);

    if (!parameters[0].is(variable_type::NUMBER) || !parameters[1].is(variable_type::NUMBER))
    {
        throw lentus_runtime_exception("Both arguments to random function must be numbers", executor);
    }

    if (parameters[0].get_number() < std::numeric_limits<long long>::min() || parameters[0].get_number() > std::numeric_limits<long long>::max() ||
        parameters[1].get_number() < std::numeric_limits<long long>::min() || parameters[1].get_number() > std::numeric_limits<long long>::max())
    {
        throw lentus_runtime_exception("Arguments to random function must be within 64-bit integer range", executor);
    }

    long long min = parameters[0].get_number();
    long long max = parameters[1].get_number();

    if (min > max)
    {
        throw lentus_runtime_exception("First argument to random function must be less than or equal to second argument", executor);
    }

    std::uniform_int_distribution<> dis(min, max);
    long long result = dis(gen);
    return variable((double)result);
}

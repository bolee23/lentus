#include "function.hpp"
#include <iostream>
#include "program.hpp"

void validate_arguments(function &func, std::vector<variable> &parameters, size_t expected)
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
    validate_arguments(*this, parameters, 1);

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

variable string_cast_function::evaluate(execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_arguments(*this, parameters, 1);
    return execution.get_program().create_string(parameters[0].to_string());
}

variable lentus_function::evaluate(execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    stack_frame &frame = execution.push_new_function(this);

    // add parameters to the new stack frame
    for (size_t i = 0; i < parameter_names->size(); i++)
    {
        variable param_value = variable((*parameter_names)[i]);
        frame.local_variables->operator[](param_value) = parameters[i];
    }

    // execute the function body
    variable result = body->evaluate(execution).get();
    execution.pop();

    return result;
}

lentus_function &lentus_function::operator=(lentus_function &&other)
{
    if (this != &other)
    {
        delete body;
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
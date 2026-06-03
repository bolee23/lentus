#include "function.hpp"
#include <iostream>
#include "program.hpp"
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());

void validate_num_arguments(function &func, std::vector<variable> &parameters, size_t expected, const ast_node *executor)
{
    if (parameters.size() != expected)
    {
        throw lentus_runtime_exception("Invalid number of arguments for function " + func.to_string() + ", expected " + std::to_string(expected), executor);
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
    validate_num_arguments(*this, parameters, 1, executor);

    if (parameters[0].is(variable_type::NUMBER))
    {
        return parameters[0];
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

variable is_number_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);

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

variable is_integer_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);

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

variable string_cast_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);
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

variable input_function::evaluate([[maybe_unused]] execution_context &execution, [[maybe_unused]] std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    std::string input;
    std::getline(std::cin, input);
    return execution.get_program().create_string(std::move(input));
}

variable rand_int_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, [[maybe_unused]] const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 2, executor);

    if (!parameters[0].is(variable_type::NUMBER) || !parameters[1].is(variable_type::NUMBER))
    {
        throw lentus_runtime_exception("Both arguments to random function must be numbers", executor);
    }

    if (parameters[0].get_number() < std::numeric_limits<long long>::min() || parameters[0].get_number() > std::numeric_limits<long long>::max() ||
        parameters[1].get_number() < std::numeric_limits<long long>::min() || parameters[1].get_number() > std::numeric_limits<long long>::max())
    {
        throw lentus_runtime_exception("Arguments to random function must be within 64-bit integer range", executor);
    }

    double min = parameters[0].get_number();
    double max = parameters[1].get_number();

    if (std::ceil(min) > std::floor(max))
    {
        throw lentus_runtime_exception("First argument to random function must be less than or equal to second argument", executor);
    }
    

    std::uniform_int_distribution<> dis(std::ceil(min), std::floor(max));
    long long result = dis(gen);
    return variable((double)result);
}

variable length_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);

    if (parameters[0].is(variable_type::LIST))
    {
        return variable((double)parameters[0].get_list()->size());
    }
    else if (parameters[0].is(variable_type::STRING))
    {
        return variable((double)parameters[0].get_string()->get_value().size());
    }
    else if (parameters[0].is(variable_type::TABLE))
    {
        return variable((double)parameters[0].get_table()->size());
    }
    else
    {
        throw lentus_runtime_exception("Argument to length function must be a list, table or string", executor);
    }
}

variable table_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    if (parameters.size() % 2 != 0)
    {
        throw lentus_runtime_exception("create_table function requires an even number of arguments for key value pairs", executor);
    }
    else
    {
        variable table = execution.get_program().create_empty_table();
        for (size_t i = 0; i < parameters.size(); i += 2)
        {
            if (parameters[i].is(variable_type::TABLE) || parameters[i].is(variable_type::FUNCTION) || parameters[i].is(variable_type::LIST))
            {
                throw lentus_runtime_exception("Invalid table key type " + parameters[i].get_type_name(), executor);
            }
            table.get_table()->add(parameters[i], parameters[i + 1]);
        }
        return table;
    }
}

variable list_add_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 2, executor);
    if (!parameters[0].is(variable_type::LIST))
    {
        throw lentus_runtime_exception("First argument to list_add must be a list", executor);
    }
    parameters[0].get_list()->push_back(parameters[1]);
    return variable();
}

variable list_remove_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 2, executor);
    if (!parameters[0].is(variable_type::LIST))
    {
        throw lentus_runtime_exception("First argument to list_remove must be a list", executor);
    }
    if (!parameters[1].is(variable_type::NUMBER))
    {
        throw lentus_runtime_exception("Second argument to list_remove must be a number", executor);
    }
    size_t index = parameters[1].get_number();
    if((double)index != parameters[1].get_number()) {
        throw lentus_runtime_exception("Second argument to list_remove must be an integer", executor);
    }
    if ((size_t)index >= parameters[0].get_list()->size()) // unsigned so don't need to check < 0
    {
        throw lentus_runtime_exception("Index out of bounds for list_remove", executor);
    }
    parameters[0].get_list()->remove(index);
    return variable();
}

variable contains_function::evaluate([[maybe_unused]] execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 2, executor);
    if (!parameters[0].is(variable_type::STRING))
    {
        throw lentus_runtime_exception("First argument to contains must be a string", executor);
    }
    if (!parameters[1].is(variable_type::STRING))
    {
        throw lentus_runtime_exception("Second argument to contains must be a string", executor);
    }

    return parameters[0].get_string()->get_value().contains(parameters[1].get_string()->get_value());
}

variable to_lowercase_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);
    if (!parameters[0].is(variable_type::STRING))
    {
        throw lentus_runtime_exception("First argument to to_lowercase must be a string", executor);
    }

    std::string str = parameters[0].get_string()->get_value();
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    return execution.get_program().create_string(str);
}

variable to_uppercase_function::evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor)
{
    validate_num_arguments(*this, parameters, 1, executor);
    if (!parameters[0].is(variable_type::STRING))
    {
        throw lentus_runtime_exception("First argument to to_uppercase must be a string", executor);
    }

    std::string str = parameters[0].get_string()->get_value();
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });

    return execution.get_program().create_string(str);
}

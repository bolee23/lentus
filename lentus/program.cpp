#include "program.hpp"

/**
 * Internal function for adding a variable to the variables array
 */
variable &program::add_variable(variable *var_ptr)
{
    this->variables.push_back(var_ptr);
    return *var_ptr;
}

variable &program::add_variable(const variable &var)
{
    return add_variable(new variable(var));
}

variable &program::add_variable(variable &&var)
{
    return add_variable(new variable(var));
}

variable &program::create_variable(bool value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(double value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(const std::string &value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(std::string &&value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(const std::vector<variable *> &value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(std::vector<variable *> &&value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value)
{
    return add_variable(new variable(value));
}

variable &program::create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &&value)
{
    return add_variable(new variable(value));
}

variable &program::create_function(const function *value)
{
    return add_variable(new variable(value));
}

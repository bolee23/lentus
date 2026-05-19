#include "program.hpp"
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "execution.hpp"

/**
 * Internal function for adding a variable to the variables array
 */
variable *program::add_variable(variable *var_ptr)
{
    this->variables.push_back(var_ptr);
    return var_ptr;
}

variable *program::add_variable(const variable &var)
{
    return add_variable(new variable(var));
}

variable *program::add_variable(variable &&var)
{
    return add_variable(new variable(var));
}

variable *program::create_variable(bool value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(double value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(const std::string &value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(std::string &&value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(const std::vector<variable *> &value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(std::vector<variable *> &&value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value)
{
    return add_variable(new variable(value));
}

variable *program::create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &&value)
{
    return add_variable(new variable(value));
}

variable *program::create_function(const function *value)
{
    return add_variable(new variable(value));
}

variable *program::create_empty_list()
{
    return add_variable(new variable(std::vector<variable *>()));
}

variable *program::create_empty_table()
{
    return add_variable(new variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal>()));
}

std::optional<program_module *> program::load_module(const std::filesystem::path &path, std::ostream &error_stream)
{
    // Check if already loaded, return it
    if (modules.contains(path))
    {
        return std::make_optional(modules[path]);
    }

    if (!std::filesystem::is_regular_file(path))
    {
        error_stream << "Error: " << path << " is not a file" << std::endl;
        return std::nullopt;
    }

    // Create module
    std::optional<program_module *> opt = create_module(path, error_stream, this->create_empty_table());
    if (!opt)
    {
        return std::nullopt;
    }

    program_module *created_module = opt.value();
    modules[path] = created_module;

    // Execute module
    execution_context execution(this);

    std::optional<variable *> result = execution.execute_module(created_module);

    if (!result)
    {
        return std::nullopt;
    }

    return std::make_optional(created_module);
}

program::program() : anon_module(create_empty_table())
{
    none_variable = add_variable(new variable());
    true_variable = create_variable(true);
    false_variable = create_variable(false);
}

program::program(program &&prog) : anon_module(std::move(prog.anon_module))
{
    this->modules = std::move(prog.modules);
    this->variables = std::move(prog.variables);
}

program &program::operator=(program &&prog)
{
    if (this != &prog)
    {
        this->modules = std::move(prog.modules);
        this->variables = std::move(prog.variables);
        this->anon_module = prog.anon_module;
    }
    return *this;
}

program::~program()
{
    for (auto entry : modules)
    {
        program_module *mod = entry.second;
        delete mod;
    }

    for (variable *var : variables)
    {
        delete var;
    }
}
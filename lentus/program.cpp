#include "program.hpp"
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "execution.hpp"
#include "function.hpp"

variable program::track_variable(variable var)
{
    if (var.get_type() == variable_type::STRING)
    {
        variable_blocks.push_back(var.get_string());
    }
    else if (var.get_type() == variable_type::LIST)
    {
        variable_blocks.push_back(var.get_list());
    }
    else if (var.get_type() == variable_type::TABLE)
    {
        variable_blocks.push_back(var.get_table());
    }

    return var;
}

variable program::create_function(lentus_function *fn)
{
    variable_blocks.push_back(fn);
    return variable(fn);
}

function* program::track_native_function(function *fn)
{
    constant_blocks.push_back(fn);
    return fn;
}

variable program::create_string(lentus_string *ptr)
{
    variable_blocks.push_back(ptr);
    return variable(ptr);
}

variable program::create_string_literal(lentus_string *ptr)
{
    constant_blocks.push_back(ptr);
    return variable(ptr);
}

variable program::create_list(lentus_list *ptr)
{
    variable_blocks.push_back(ptr);
    return variable(ptr);
}

variable program::create_table(lentus_table *ptr)
{
    variable_blocks.push_back(ptr);
    return variable(ptr);
}

lentus_list *program::create_empty_list()
{
    lentus_list *list = new lentus_list();
    variable_blocks.push_back(list);
    return list;
}

lentus_table *program::create_empty_table()
{
    lentus_table *table = new lentus_table();
    variable_blocks.push_back(table);
    return table;
}

std::optional<program_module *> program::load_module(const std::filesystem::path &path, std::ostream &error_stream, bool lp_debug)
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
    std::optional<program_module *> opt = create_module(this->constant_blocks, path, error_stream, this->create_empty_table(), lp_debug);
    if (!opt)
    {
        return std::nullopt;
    }

    program_module *created_module = opt.value();
    modules[path] = created_module;

    if (lp_debug)
    {
        std::cout << "Result:" << std::endl;
    }
    // Execute module
    execution_context execution(this);

    std::optional<reference> result = execution.execute_module(created_module);

    if (!result)
    {
        return std::nullopt;
    }

    return std::make_optional(created_module);
}

program::program() : anon_module(create_empty_table())
{
    native_functions = {
        {"print", track_native_function(new print_function())},
        {"print_line", track_native_function(new print_line_function())},
        {"number", track_native_function(new number_cast_function())},
        {"string", track_native_function(new string_cast_function())},
    };
}

program::program(program &&prog) : anon_module(std::move(prog.anon_module))
{
    this->modules = std::move(prog.modules);
    this->variable_blocks = std::move(prog.variable_blocks);
    this->constant_blocks = std::move(prog.constant_blocks);
}

program &program::operator=(program &&prog)
{
    if (this != &prog)
    {
        this->modules = std::move(prog.modules);
        this->variable_blocks = std::move(prog.variable_blocks);
        this->constant_blocks = std::move(prog.constant_blocks);
        this->anon_module = std::move(prog.anon_module);
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

    for (variable_block *var : variable_blocks)
    {
        delete var;
    }

    for (variable_block *var : constant_blocks)
    {
        delete var;
    }
}
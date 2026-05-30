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

function *program::track_native_function(function *fn)
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

std::vector<source_line> split_lines(const std::string &source_string)
{
    std::vector<source_line> lines;
    std::string current_line;
    int pos = 0;
    int line_start = 0;
    for (int i = 0; i < (int)source_string.length(); i++)
    {
        char c = source_string[i];
        if (c == '\n')
        {
            lines.push_back({std::move(current_line), line_start});
            current_line.clear();
            pos = i + 1;
            line_start = pos;
        }
        else
        {
            current_line += c;
        }
    }
    if (!current_line.empty())
    {
        lines.push_back({std::move(current_line), line_start});
    }
    return lines;
}

program::program() : anon_module(create_empty_table())
{
    // General
    register_native_function("print", new print_function());
    register_native_function("print_line", new print_line_function());
    register_native_function("length", new length_function());
    register_native_function("input", new input_function());

    // Numbers
    register_native_function("to_number", new number_cast_function());
    register_native_function("is_number", new is_number_function());
    register_native_function("is_integer", new is_integer_function());
    register_native_function("rand_int", new rand_int_function());

    // Lists and tables
    register_native_function("list", new list_function());
    register_native_function("table", new table_function());
    register_native_function("list_add", new list_add_function());
    register_native_function("list_remove", new list_remove_function());

    // Strings
    register_native_function("to_string", new string_cast_function());
    register_native_function("contains", new contains_function());
    register_native_function("to_uppercase", new to_uppercase_function());
    register_native_function("to_lowercase", new to_lowercase_function());
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

function *program::register_native_function(std::string name, function *fn)
{
    function *registered_fn = track_native_function(fn);
    native_functions[name] = registered_fn;
    return registered_fn;
}

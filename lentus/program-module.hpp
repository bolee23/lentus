#ifndef PROGRAM_MODULE_HPP
#define PROGRAM_MODULE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "variable.hpp"
#include "statement.hpp"

class program_module
{
private:
    std::filesystem::path path;
    statement *root_statement;
    variable *global_table; /** Global table contains only the module's variables/functions that are marked with 'export' */

public:
    program_module(const std::filesystem::path &path, statement *root_statement, variable *global_table) : path(path), root_statement(root_statement), global_table(global_table) {}

    variable *get_global_table() const { return global_table; }

    std::filesystem::path get_path() const { return path; }
    const statement* get_root_statement() const { return root_statement; }

    program_module(const program_module &prgm_module) = delete;
    program_module(program_module &&prgm_module);

    program_module &operator=(const program_module &prgm_module) = delete;
    program_module &operator=(program_module &&prgm_module);

    ~program_module();
};

/**
 * Anonymous modules are used for executing code from the REPL.
 * They do not have a path and their global table is not shared with any other module.
 */
class anonymous_module
{
private:
    variable *global_table;

public:
    anonymous_module(variable *global_table) : global_table(global_table) {}

    variable *get_global_table() const { return global_table; };
};

std::optional<program_module *> create_module(const std::filesystem::path &path, std::ostream &error_stream, variable *global_table);

#endif
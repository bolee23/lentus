#ifndef PROGRAM_MODULE_HPP
#define PROGRAM_MODULE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "data.hpp"
#include "statement.hpp"

namespace lentus
{
class source_line;

/**
 * Represents the parsed code for a single source file in the Lentus language.
 */
class program_module
{
private:
    std::filesystem::path path;
    statement *root_statement;
    table *global_table; /** Global table contains only the module's variables/functions that are marked with 'export' */

    std::string source_code;               /** The source code of the module, used for error reporting */
    std::vector<source_line> source_lines; /** The source code split into lines, used for error reporting */

public:
    program_module(const std::filesystem::path &path, statement *root_statement, table *global_table, std::string &&source_code = "", std::vector<source_line> &&source_lines = {}) : path(path), root_statement(root_statement), global_table(global_table), source_code(source_code), source_lines(source_lines) {}

    table *get_global_table() const { return global_table; }

    std::filesystem::path get_path() const { return path; }
    const statement *get_root_statement() const { return root_statement; }

    program_module(const program_module &prgm_module) = delete;
    program_module(program_module &&prgm_module);

    program_module &operator=(const program_module &prgm_module) = delete;
    program_module &operator=(program_module &&prgm_module);

    std::string get_source_code() const { return source_code; }
    const std::vector<source_line> &get_source_lines() const { return source_lines; }

    ~program_module();
};

/**
 * Anonymous modules are used for executing code from the REPL.
 * They do not have a path and their global table is not shared with any other module.
 */
class anonymous_module
{
private:
    table *global_table;

public:
    anonymous_module(table *global_table) : global_table(global_table) {}

    table *get_global_table() const { return global_table; };
};

/**
 * Creates a module from the given path. Does not execute it.
 * @param literal_variables Vector of literal variables to add constants to
 * @param path The path to the module to create
 * @param error_stream The stream to write error messages to
 * @param global_table The global table to use for the module.
 * @param lp_debug Whether to print debug messages during lexing and parsing
 * @return The created module, or std::nullopt if creation failed
 */
std::optional<program_module *> create_module(std::vector<value_block *> &literal_variables, const std::filesystem::path &path, std::ostream &error_stream, table *global_table, bool lp_debug = false);

} // namespace lentus

#endif

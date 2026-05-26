#ifndef LENTUS_PROGRAM_HPP
#define LENTUS_PROGRAM_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "variable.hpp"
#include "program-module.hpp"

class program
{
private:
    std::unordered_map<std::filesystem::path, program_module *> modules;
    std::vector<variable_block *> variable_blocks; /** List of all dynamically allocated variable_blocks in the program */
    std::vector<variable_block *> constant_blocks; /** List of all variable_blocks that should live forever (ie literal strings, functions). Allocated during parsing */

    /**
     * Map of native functions that are available in the language by default.
     */
    std::unordered_map<std::string, function *> native_functions;
    anonymous_module anon_module;

public:
    program();

    program(const program &prog) = delete;
    program(program &&prog);

    program &operator=(const program &prog) = delete;
    program &operator=(program &&prog);

    /**
     * If the variable has a dynamically allocated variable_block,
     * track it so that it can be deallocated when no longer in use.
     * @param var The variable to track
     * @return The same variable
     */
    variable track_variable(variable var);

    /**
     * Track the given native function.
     * Native functions are only deallocated when the program is deallocated.
     */
    function *track_native_function(function *fn);

    /**
     * Creates a variable that holds a pointer to the given lentus_function and tracks the lentus_function for deallocation.
     */
    variable create_function(lentus_function *fn);

    /**
     * Creates a variable that holds a pointer to the given lentus_string and tracks the lentus_string for deallocation.
     */
    variable create_string(lentus_string *ptr);
    variable create_string(const std::string &str) { return create_string(new lentus_string(str)); }
    variable create_string(std::string &&str) { return create_string(new lentus_string(std::move(str))); }

    /**
     * Like create_string, but adds to the literal_blocks array instead.
     */
    variable create_string_literal(lentus_string *ptr);
    variable create_string_literal(const std::string &str) { return create_string_literal(new lentus_string(str)); }
    variable create_string_literal(std::string &&str) { return create_string_literal(new lentus_string(std::move(str))); }

    /**
     * Creates a variable that holds a pointer to the given lentus_list and tracks the lentus_list for deallocation.
     */
    variable create_list(lentus_list *ptr);
    variable create_list(const std::vector<variable> &elements) { return create_list(new lentus_list(elements)); }
    variable create_list(std::vector<variable> &&elements) { return create_list(new lentus_list(std::move(elements))); }

    /**
     * Creates a variable that holds a pointer to the given lentus_table and tracks the lentus_table for deallocation.
     */
    variable create_table(lentus_table *ptr);
    variable create_table(const std::unordered_map<variable, variable, variable_hasher, variable_equal> &entries) { return create_table(new lentus_table(entries)); }
    variable create_table(std::unordered_map<variable, variable, variable_hasher, variable_equal> &&entries) { return create_table(new lentus_table(std::move(entries))); }

    lentus_list *create_empty_list();
    lentus_table *create_empty_table();

    ~program();

    /**
     * Loads a module from the given path.
     * If the module has already been loaded, returns the existing module.
     *
     * @param path The path to the module to load
     * @param error_stream The stream to write error messages to
     * @param debug Whether to print debug messages during lexing and parsing
     * @return The module, or std::nullopt if there was an error during loading or execution
     */
    std::optional<program_module *> load_module(const std::filesystem::path &path, std::ostream &error_stream, bool lp_debug = false);

    /**
     * Executes an anonymous module with the given source code. This is used for executing code from the REPL.
     * @param str The source code of the anonymous module
     * @param error_stream The stream to write error messages to
     * @return The result of the execution as a variable, or std::nullopt if there was an error during execution
     */
    std::optional<variable *> execute_anonymous_module(const std::string &str, std::ostream &error_stream);

    const std::unordered_map<std::string, function *> &get_native_functions() const { return native_functions; }
    std::unordered_map<std::string, function *> &get_native_functions() { return native_functions; }
};

class lentus_runtime_exception : public std::exception
{
    std::string message;
    const ast_node *node;

public:
    lentus_runtime_exception(std::string &message, const ast_node *node = nullptr) : message(message), node(node) {}
    lentus_runtime_exception(std::string &&message, const ast_node *node = nullptr) : message(std::move(message)), node(node) {}

    const char *what() const noexcept { return message.c_str(); }

    const ast_node *get_node() const { return node; }
};

// The following exceptions are used for handling control flow. They are not "errors" but expected exceptions to be caught during execution.

class break_signal : public std::exception
{
public:
    break_signal() = default;
    const char *what() const noexcept { return "Break statement"; }
};

class continue_signal : public std::exception
{
public:
    continue_signal() = default;
    const char *what() const noexcept { return "Continue statement"; }
};

class return_signal : public std::exception
{
private:
    variable return_value;

public:
    return_signal(variable value) : return_value(value) {}
    return_signal(variable &&value) : return_value(std::move(value)) {}
    variable get_return_value() const { return return_value; }
    const char *what() const noexcept { return "Return statement"; }
};

#endif
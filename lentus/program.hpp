#ifndef LENTUS_PROGRAM_HPP
#define LENTUS_PROGRAM_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

#include "data.hpp"
#include "program-module.hpp"

namespace lentus
{

/**
 * Represents a single Lentus program, which contains zero or more modules.
 * Stores pointers to variable_blocks for memory management.
 */
class program
{
private:
    std::unordered_map<std::filesystem::path, program_module *> modules;
    std::vector<value_block *> variable_blocks; /** List of all dynamically allocated variable_blocks in the program */
    std::vector<value_block *> constant_blocks; /** List of all variable_blocks that should live forever (ie literal strings, functions). Allocated during parsing */

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
     * If the value has a dynamically allocated value_block,
     * track it so that it can be deallocated when no longer in use.
     * @param var The value to track
     * @return The same value
     */
    value track_variable(value var);

    /**
     * Track the given native function for deallocation
     * Native functions are only deallocated when the program is deallocated.
     */
    function *track_native_function(function *fn);

    /**
     * Register a native function with the given name. The function will be tracked for deallocation.
     * If a function with the same name already exists, it will be overwritten.
     */
    function *register_native_function(std::string name, function *fn);

    /**
     * Creates a value that holds a pointer to the given user_function and tracks the user_function for deallocation.
     */
    value create_function(user_function *fn);

    /**
     * Creates a value that holds a pointer to the given string and tracks the string for deallocation.
     */
    value create_string(string *ptr);
    value create_string(const std::string &str) { return create_string(new string(str)); }
    value create_string(std::string &&str) { return create_string(new string(std::move(str))); }

    /**
     * Like create_string, but adds to the literal_blocks array instead.
     */
    value create_string_literal(string *ptr);
    value create_string_literal(const std::string &str) { return create_string_literal(new string(str)); }
    value create_string_literal(std::string &&str) { return create_string_literal(new string(std::move(str))); }

    /**
     * Creates a value that holds a pointer to the given list and tracks the list for deallocation.
     */
    value create_list(list *ptr);
    value create_list(const std::vector<value> &elements) { return create_list(new list(elements)); }
    value create_list(std::vector<value> &&elements) { return create_list(new list(std::move(elements))); }

    /**
     * Creates a value that holds a pointer to the given table and tracks the table for deallocation.
     */
    value create_table(table *ptr);
    value create_table(const std::unordered_map<value, value, value_hasher, value_equal> &entries) { return create_table(new table(entries)); }
    value create_table(std::unordered_map<value, value, value_hasher, value_equal> &&entries) { return create_table(new table(std::move(entries))); }

    list *create_empty_list();
    table *create_empty_table();

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
     * @return The result of the execution as a value, or std::nullopt if there was an error during execution
     */
    std::optional<value *> execute_anonymous_module(const std::string &str, std::ostream &error_stream);

    const std::unordered_map<std::string, function *> &get_native_functions() const { return native_functions; }
    std::unordered_map<std::string, function *> &get_native_functions() { return native_functions; }
};

class runtime_exception : public std::exception
{
    std::string message;
    const ast_node *node;

public:
    runtime_exception(std::string &message, const ast_node *node = nullptr) : message(message), node(node) {}
    runtime_exception(std::string &&message, const ast_node *node = nullptr) : message(std::move(message)), node(node) {}

    const char *what() const noexcept { return message.c_str(); }

    const ast_node *get_node() const { return node; }
};

// The following exceptions are used for handling control flow. They are not "errors" but expected exceptions to be caught during execution.

/**
 * Thrown when executing a break statement.
 */
class break_signal : public std::exception
{
public:
    break_signal() = default;
    const char *what() const noexcept { return "Break statement"; }
};

/**
 * Thrown when executing a continue statement.
 */
class continue_signal : public std::exception
{
public:
    continue_signal() = default;
    const char *what() const noexcept { return "Continue statement"; }
};

/**
 * Thrown when executing a return statement. 
 * Contains the return value.
 * If no value is returned, the return value is a value of type NONE.
 */
class return_signal : public std::exception
{
private:
    value return_value;

public:
    return_signal(value val) : return_value(val) {}
    return_signal(value &&value) : return_value(std::move(value)) {}
    value get_return_value() const { return return_value; }
    const char *what() const noexcept { return "Return statement"; }
};

/** Represents a line of source code (for error reporting) */
struct source_line
{
    std::string text;
    int start_position;
};

std::vector<source_line> split_lines(const std::string &source_string);

} // namespace lentus

#endif

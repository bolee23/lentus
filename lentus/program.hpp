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
    std::vector<variable *> variables; /** List of all variables in the program */
    anonymous_module anon_module;
    variable *add_variable(variable *var);

    // Preset variables
    variable *none_variable;
    variable *true_variable;
    variable *false_variable;

public:
    program();

    program(const program &prog) = delete;
    program(program &&prog);

    program &operator=(const program &prog) = delete;
    program &operator=(program &&prog);

    variable *add_variable(const variable &var);
    variable *add_variable(variable &&var);

    variable *create_variable(bool value);
    variable *create_variable(double value);
    variable *create_variable(const std::string &value);
    variable *create_variable(std::string &&value);
    variable *create_variable(const std::vector<variable *> &value);
    variable *create_variable(std::vector<variable *> &&value);
    variable *create_variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value);
    variable *create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &&value);
    variable *create_function(const function *value);

    /**
     * @return the pointer to the none variable
     */
    variable *get_none_variable() { return none_variable; }

    /**
     * @return the pointer to the true variable
     */
    variable *get_true_variable() { return true_variable; }

    /**
     * @return the pointer to the false variable
     */
    variable *get_false_variable() { return false_variable; }

    /**
     * @param value Boolean value
     * @return a pointer to a true or false variable
     */
    variable *get_boolean_variable(bool value) { return value ? true_variable : false_variable; }

    variable *create_empty_list();
    variable *create_empty_table();

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

#endif
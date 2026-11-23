#ifndef LENTUS_FUNCTION_HPP
#define LENTUS_FUNCTION_HPP

#include "data.hpp"
#include "execution.hpp"
#include <optional>

namespace lentus
{

/**
 * Base class for all functions in the Lentus language, including native functions and user-defined functions.
 */
class function : public value_block
{
public:
    virtual value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) = 0;
};

class print_function : public function
{
public:
    print_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function print>"; }
};

class print_line_function : public function
{
public:
    print_line_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function print_line>"; }
};

class number_cast_function : public function
{
public:
    number_cast_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_number>"; }
};

class is_number_function : public function
{
public:
    is_number_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function is_number>"; }
};

class is_integer_function : public function
{
public:
    is_integer_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function is_integer>"; }
};

class string_cast_function : public function
{
public:
    string_cast_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_string>"; }
};

class input_function : public function
{
public:
    input_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function input>"; }
};

class rand_int_function : public function
{
public:
    rand_int_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function random>"; }
};

class length_function : public function
{
public:
    length_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function length>"; }
};

class list_function : public function
{
public:
    list_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, [[maybe_unused]] const ast_node *executor) override { return execution.get_program().create_list(parameters); }
    std::string to_string() const override { return "<function list>"; }
};

class table_function : public function
{
public:
    table_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function table>"; }
};

class list_add_function : public function
{
public:
    list_add_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function list_add>"; }
};

class list_remove_function : public function
{
public:
    list_remove_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function list_remove>"; }
};

class contains_function : public function
{
public:
    contains_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function contains>"; }
};

class to_lowercase_function : public function
{
public:
    to_lowercase_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_lowercase>"; }
};

class to_uppercase_function : public function
{
public:
    to_uppercase_function() {};
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_uppercase>"; }
};

/**
 * Represents a user-defined function closure in the Lentus language.
 * Contains the function's body as well as capturing the variables in the scope where the function was defined.
 */
class user_function : public function
{
    // Known during parsing
    string *name;
    statement *body;
    const std::vector<string *> *parameter_names;

    // Known during runtime
    program_module *current_module;
    std::vector<table *> captured_variable_tables;

public:
    user_function(string *name, statement *body, const std::vector<string *> *parameter_names, program_module *module, std::vector<table *> &&captured_tables) : name(name), body(body), parameter_names(parameter_names), current_module(module), captured_variable_tables(std::move(captured_tables)) {};

    statement *get_body() const { return body; }
    const std::vector<string *> *get_parameter_names() const { return parameter_names; }
    value evaluate(execution_context &execution, std::vector<value> &parameters, const ast_node *executor) override;

    user_function &operator=(const user_function &other) = delete;
    user_function(const user_function &other) = delete;

    user_function &operator=(user_function &&other);
    user_function(user_function &&other);

    program_module *get_current_module() const { return current_module; }
    const std::vector<table *> &get_captured_variable_tables() const { return captured_variable_tables; }

    ~user_function() override = default;

    std::string to_string() const override
    {
        if (name)
        {
            return "<function " + name->to_string() + ">";
        }
        else
        {
            return "<function @" + std::to_string((size_t)this) + ">";
        }
    }
};

} // namespace lentus

#endif

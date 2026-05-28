#ifndef LENTUS_FUNCTION_HPP
#define LENTUS_FUNCTION_HPP

#include "variable.hpp"
#include "execution.hpp"
#include <optional>

class function : public variable_block
{
public:
    virtual variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) = 0;
};

class print_function : public function
{
public:
    print_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function print>"; }
};

class print_line_function : public function
{
public:
    print_line_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function print_line>"; }
};

class number_cast_function : public function
{
public:
    number_cast_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_number>"; }
};

class is_number_function : public function
{
public:
    is_number_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function is_number>"; }
};

class is_integer_function : public function
{
public:
    is_integer_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function is_integer>"; }
};

class string_cast_function : public function
{
public:
    string_cast_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function to_string>"; }
};

class input_function : public function
{
public:
    input_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function input>"; }
};

class rand_int_function : public function
{
public:
    rand_int_function() {};
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;
    std::string to_string() const override { return "<function random>"; }
};

class lentus_function : public function
{
    // Known during parsing
    lentus_string *name;
    statement *body;
    const std::vector<lentus_string *> *parameter_names;

    // Known during runtime
    program_module *current_module;
    std::vector<lentus_table *> captured_variable_tables;

public:
    lentus_function(lentus_string *name, statement *body, const std::vector<lentus_string *> *parameter_names, program_module *module, std::vector<lentus_table *> &&captured_tables) : name(name), body(body), parameter_names(parameter_names), current_module(module), captured_variable_tables(std::move(captured_tables)) {};

    statement *get_body() const { return body; }
    const std::vector<lentus_string *> *get_parameter_names() const { return parameter_names; }
    variable evaluate(execution_context &execution, std::vector<variable> &parameters, const ast_node *executor) override;

    lentus_function &operator=(const lentus_function &other) = delete;
    lentus_function(const lentus_function &other) = delete;

    lentus_function &operator=(lentus_function &&other);
    lentus_function(lentus_function &&other);

    program_module *get_current_module() const { return current_module; }
    const std::vector<lentus_table *> &get_captured_variable_tables() const { return captured_variable_tables; }

    ~lentus_function() override = default;

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

#endif
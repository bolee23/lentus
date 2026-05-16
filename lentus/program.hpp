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
    std::unordered_map<std::filesystem::path, program_module> modules;
    std::vector<variable> variables;
    variable &add_variable(variable *var);

public:
    program() : modules({}), variables({}) {};
    program_module *initialize_module(const std::filesystem::path &path);
    variable &add_variable(const variable &var);
    variable &add_variable(variable &&var);

    variable &create_variable(bool value);
    variable &create_variable(double value);
    variable &create_variable(const std::string &value);
    variable &create_variable(std::string &&value);
    variable &create_variable(const std::vector<variable *> &value);
    variable &create_variable(std::vector<variable *> &&value);
    variable &create_variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value);
    variable &create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &&value);
    variable &create_function(const function *value);
};

class lentus_runtime_exception : public std::exception
{
    std::string message;
    const expression *node;

public:
    lentus_runtime_exception(std::string &message, const expression *node = nullptr) : message(message), node(node) {}
    lentus_runtime_exception(std::string &&message, const expression *node = nullptr) : message(std::move(message)), node(node) {}

    const char *what() const noexcept { return message.c_str(); }

    const expression *get_node() const { return node; }
};

#endif
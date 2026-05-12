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

public:
    program_module *initialize_module(const std::filesystem::path &path);
    variable &add_variable(variable &&variable);

    variable &create_variable(bool value);
    variable &create_variable(double value);
    variable &create_variable(std::string &value);
    variable &create_variable(std::vector<variable *> &value);
    variable &create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value);
    variable &create_function(function *value);
};

#endif
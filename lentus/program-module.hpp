#ifndef PROGRAM_MODULE_HPP
#define PROGRAM_MODULE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "variable.hpp"
#include "parser.hpp"

class program_module
{
private:
    std::filesystem::path path;
    parsed_code_unit *parsed_code;
    variable *root_table;

public:
    program_module(const std::filesystem::path &path) : path(path) {}
};

#endif
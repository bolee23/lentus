#ifndef PROGRAM_MODULE_HPP
#define PROGRAM_MODULE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "variable.hpp"
#include "ast.hpp"

class program_module
{
private:
    std::filesystem::path path;
    ast_node *ast_root;
    variable *root_table;

public:
    program_module(const std::filesystem::path &path) : path(path) {}
};

#endif
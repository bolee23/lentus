#ifndef LENTUS_PARSER_HPP
#define LENTUS_PARSER_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

class program;

class ast_node
{
private:
    int line_number;

public:
    virtual void visit(program &program);
    virtual std::vector<ast_node *> get_children();
};

class parsed_code_unit
{
private:
    ast_node *root_node;
    std::filesystem::path path;

public:
    parsed_code_unit(ast_node *root_node, const std::filesystem::path &path) : root_node(root_node), path(path) {};
};

#endif
#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "program.hpp"

struct stack_frame
{
    std::unordered_map<std::string, variable *> local_variables;
};

struct ast_stack_frame
{
    ast_node *node;
};

class execution_context
{
private:
    std::vector<stack_frame> call_stack;
    std::vector<ast_stack_frame> ast_stack;
    program *program;

public:
    void execute_node(ast_node *node);
    void execute_function(variable *function, std::vector<variable *> &arguments);
    void execute_module(program_module *module);
};

#endif
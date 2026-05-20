#ifndef LENTUS_AST_NODE_HPP
#define LENTUS_AST_NODE_HPP

#include <string>
#include "variable.hpp"

class execution_context;

class ast_node
{
protected:
    int line_number;
    int start_position;
    int end_position;

public:
    int get_line_number() const { return line_number; }
    int get_start_position() const { return start_position; }
    int get_end_position() const { return end_position; }

    virtual std::string to_string() const = 0;

    virtual ~ast_node() = default;

    virtual reference evaluate(execution_context &execution) const = 0;
};

#endif
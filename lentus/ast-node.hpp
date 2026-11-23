#ifndef LENTUS_AST_NODE_HPP
#define LENTUS_AST_NODE_HPP

#include <string>
#include "data.hpp"

namespace lentus
{

class execution_context;

/**
 * Base class for all AST nodes in the Lentus language.
 */
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

    /**
     * @return a string representation of the AST node for debugging
     */
    virtual std::string to_string() const = 0;

    virtual ~ast_node() = default;

    /**
     * Evaluate the AST node in a given execution context.
     * @return a reference to the result of the evaluation
     */
    virtual reference evaluate(execution_context &execution) const = 0;
};

} // namespace lentus

#endif

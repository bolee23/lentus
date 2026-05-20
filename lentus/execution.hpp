#ifndef LENTUS_EXECUTION_HPP
#define LENTUS_EXECUTION_HPP

#include "program.hpp"
#include "variable.hpp"

struct stack_frame
{
    program_module *current_module;
    lentus_table *local_variables;
};

class execution_context
{
private:
    std::vector<stack_frame> stack;
    program *prgm;

public:
    execution_context(program *prgm) : stack(), prgm(prgm) {}

    std::optional<reference> execute_function(variable *function, std::vector<variable *> &arguments);

    std::optional<reference> execute_module(program_module *module);

    program *get_program() { return prgm; }
    const program *get_program() const { return prgm; }

    /**
     * Push a new stack frame for executing a new module onto the stack
     * @param current_module the module to push
     * @return the new stack frame
     */
    stack_frame &push_new_module(program_module *current_module) { return stack.emplace_back(current_module, new lentus_table()); }

    /**
     * Push a new stack frame
     * @return the new stack frame
     */
    stack_frame &push() { return stack.emplace_back(peek().current_module, new lentus_table()); }

    /**
     * Get the current stack frame
     * @return the current stack frame
     */
    stack_frame &peek() { return stack.at(stack.size() - 1); }

    /**
     * Pop the current stack frame
     */
    void pop() { stack.pop_back(); }

    /**
     * Get the entire stack
     */
    std::vector<stack_frame> &get_stack() { return stack; }

    /**
     * Get the entire stack
     */
    const std::vector<stack_frame> &get_stack() const { return stack; }

    /**
     * Get the current size of the stack.
     * This is used with synchronize to remove elements from the stack after an exception has occured.
     * @return current size of the stack
     */
    int stack_position() { return (int)stack.size(); }

    /**
     * Method for removing succeeding frames from the stack after an exception has occured
     * After resizing, the stack should be in the same state as before the try block
     * @param size the stack size before the try block
     */
    void stack_synchronize(int size) { this->stack.resize(size); }

    /**
     * Get the current module that is being executed
     * @return the module or nullptr if no module is being executed
     */
    program_module *get_current_module();

    /**
     * Searches down the stack for the nearest definition of a local or global variable with a given identifier
     * The search stops when another module is encountered in the call stack, or when the stack has been exhausted.
     * Before stopping, the method will also search for the module's global variable table
     * @param identifier Identifier to search for
     * @return Modifiable reference to the variable. nullopt if not found.
     */
    std::optional<reference> get_variable(const std::string &identifier);
};

#endif
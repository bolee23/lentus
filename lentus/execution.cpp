#include "execution.hpp"
#include <iostream>

std::optional<reference> execution_context::execute_module(program_module *mdl)
{
    const statement *current_statement = mdl->get_root_statement();

    push_new_module(mdl);
    int size = this->stack_position();
    try
    {
        reference result = current_statement->evaluate(*this);
        pop();
        return result;
    }
    catch (lentus_runtime_exception &exception)
    {
        std::cout << "Unhandled runtime error while executing " + mdl->get_path().filename().string() << ":" << std::endl;
        std::cout << exception.what() << std::endl;

        this->stack_synchronize(size);
        pop();
        return std::nullopt;
    }
}

program_module *execution_context::get_current_module()
{
    if (stack.empty())
    {
        return nullptr;
    }
    else
    {
        return peek().current_module;
    }
}

std::optional<reference> execution_context::get_variable(const std::string &identifier)
{
    if (stack.empty())
    {
        return std::nullopt;
    }

    program_module *current_module = get_current_module();
    for (int i = stack.size() - 1; i >= 0; i--)
    {
        stack_frame &frame = stack[i];

        if (current_module != frame.current_module)
        {
            break;
        }
        if (frame.local_variables.contains(identifier))
        {
            return std::optional<reference>(reference(&frame.local_variables[identifier]));
        }
    }

    // One last search in the current module's global table
    lentus_table *table_ptr = current_module->get_global_table();
    lentus_string str = identifier;

    if (table_ptr->contains(&str))
    {
        return std::optional<reference>(reference(&table_ptr->at(&str)));
    }
    else
    {
        return std::nullopt;
    }
}
#include "execution.hpp"
#include <iostream>

std::optional<variable *> execution_context::execute_module(program_module *mdl)
{
    const statement *current_statement = mdl->get_root_statement();

    push_new_module(mdl);
    int size = this->stack_position();
    try
    {
        variable *result = current_statement->evaluate(*this);
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

variable *execution_context::get_variable(const std::string &identifier)
{
    if (stack.empty())
    {
        return nullptr;
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
            return frame.local_variables.at(identifier);
        }
    }

    // One last search in the current module's global table
    auto table_ptr = current_module->get_global_table()->get_table();
    variable identifier_as_variable = variable(std::move(identifier)); // identifier not needed anymore

    if (table_ptr->contains(&identifier_as_variable))
    {
        return table_ptr->at(&identifier_as_variable);
    }
    else
    {
        return nullptr;
    }
}

bool execution_context::set_variable(const std::string &identifier, variable *value)
{
    if (stack.empty())
    {
        return false;
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
            frame.local_variables.at(identifier) = value;
            return true;
        }
    }

    // One last search in the current module's global table
    auto table_ptr = current_module->get_global_table()->get_table();
    variable identifier_as_variable = variable(std::move(identifier)); // identifier not needed anymore

    if (table_ptr->contains(&identifier_as_variable))
    {
        table_ptr->at(&identifier_as_variable) = value;
        return true;
    }
    else
    {
        return false;
    }
}

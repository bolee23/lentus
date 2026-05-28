#include "execution.hpp"
#include <iostream>
#include "function.hpp"

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
        if (exception.get_node())
        {
            std::cout << "Runtime error while executing " + mdl->get_path().filename().string() + " at line " + std::to_string(exception.get_node()->get_line_number()) << ": " << std::endl;
            if (!mdl->get_source_lines().empty() && !mdl->get_source_code().empty())
            {
                int line_index = exception.get_node()->get_line_number() - 1;
                if (line_index >= (int)mdl->get_source_lines().size())
                {
                    line_index = mdl->get_source_lines().size() - 1;
                }

                std::cout << mdl->get_source_lines()[line_index].text << std::endl;

                // Position the arrows
                for (int i = 0; i < exception.get_node()->get_start_position() - mdl->get_source_lines()[line_index].start_position; i++)
                {
                    std::cout << ' ';
                }

                // One arrow for each character in the token
                for (int i = exception.get_node()->get_start_position(); i < exception.get_node()->get_end_position(); i++)
                {
                    std::cout << '^';
                }

                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "Runtime error while executing " + mdl->get_path().filename().string() + ": " << std::endl;
        }
        std::cout << exception.what() << std::endl;

        this->stack_synchronize(size);
        pop();
        return std::nullopt;
    }
}

stack_frame &execution_context::push_new_function(lentus_function *current_function)
{
    return stack.emplace_back(current_function->get_current_module(), current_function, prgm->create_empty_table());
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

lentus_function *execution_context::get_current_function()
{
    if (stack.empty())
    {
        return nullptr;
    }
    else
    {
        return peek().current_function;
    }
}

std::optional<reference> execution_context::get_variable(const std::string &identifier)
{
    if (stack.empty())
    {
        return std::nullopt;
    }

    lentus_string str = identifier;

    program_module *current_module = get_current_module();
    lentus_function *current_function = get_current_function();

    for (int i = stack.size() - 1; i >= 0; i--)
    {
        stack_frame &frame = stack[i];

        if (current_module != frame.current_module)
        {
            break;
        }
        if (frame.local_variables->contains(&str))
        {
            return std::optional<reference>(reference(&frame.local_variables->at(&str)));
        }
    }

    // Search function's captured variables if inside a function
    if (current_function != nullptr)
    {
        for (const auto &table : current_function->get_captured_variable_tables())
        {
            if (table->contains(&str))
            {
                return std::optional<reference>(reference(&table->at(&str)));
            }
        }
    }

    // Search global table if not inside function.
    if (current_module != nullptr)
    {
        lentus_table *table_ptr = current_module->get_global_table();

        if (table_ptr->contains(&str))
        {
            return std::optional<reference>(reference(&table_ptr->at(&str)));
        }
    }

    // One last last search for native functions. This returns an unmodifiable reference.
    if (prgm->get_native_functions().contains(identifier))
    {
        return std::optional<reference>(reference(prgm->get_native_functions().at(identifier)));
    }

    return std::nullopt;
}

void execution_context::stack_synchronize(int size)
{
    for (int i = stack.size(); i > size; i--)
    {
        stack.pop_back();
    }
}

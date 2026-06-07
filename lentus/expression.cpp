#include "expression.hpp"
#include "program.hpp"
#include "execution.hpp"
#include <iostream>
#include "function.hpp"
#include <algorithm>

unary_expression::unary_expression(unary_expression &&other)
{
    op = std::move(other.op);
    operand = other.operand;
    other.operand = nullptr;
}

unary_expression &unary_expression::operator=(unary_expression &&other)
{
    if (this != &other)
    {
        delete operand;
        op = std::move(other.op);
        operand = other.operand;
        other.operand = nullptr;
    }
    return *this;
}

std::string unary_expression::to_string() const
{
    if (is_postfix)
    {
        return "POST_" + this->op.to_string() + "(" + this->operand->to_string() + ")";
    }
    else
    {
        return this->op.to_string() + "(" + this->operand->to_string() + ")";
    }
}

unary_expression::~unary_expression()
{
    delete operand;
}

binary_expression::binary_expression(binary_expression &&other)
{
    op = std::move(other.op);
    left = other.left;
    right = other.right;
    other.left = nullptr;
    other.right = nullptr;
}

binary_expression &binary_expression::operator=(binary_expression &&other)
{
    if (this != &other)
    {
        delete left;
        delete right;
        op = std::move(other.op);
        left = other.left;
        right = other.right;
        other.left = nullptr;
        other.right = nullptr;
    }
    return *this;
}

std::string binary_expression::to_string() const
{
    return this->op.to_string() + "[" + this->left->to_string() + ", " + this->right->to_string() + "]";
}

binary_expression::~binary_expression()
{
    delete left;
    delete right;
}

subscript_expression::subscript_expression(subscript_expression &&other)
{
    left = other.left;
    right = other.right;
    other.left = nullptr;
    other.right = nullptr;
}

subscript_expression &subscript_expression::operator=(subscript_expression &&other)
{
    if (this != &other)
    {
        delete left;
        delete right;
        left = other.left;
        right = other.right;
        other.left = nullptr;
        other.right = nullptr;
    }
    return *this;
}

std::string subscript_expression::to_string() const
{
    return "INDEX[" + this->left->to_string() + ", " + this->right->to_string() + "]";
}

subscript_expression::~subscript_expression()
{
    delete left;
    delete right;
}

grouping_expression::grouping_expression(grouping_expression &&other)
{
    inner_expression = other.inner_expression;
    other.inner_expression = nullptr;
}

grouping_expression &grouping_expression::operator=(grouping_expression &&other)
{
    if (this != &other)
    {
        delete inner_expression;
        inner_expression = other.inner_expression;
        other.inner_expression = nullptr;
    }
    return *this;
}

std::string grouping_expression::to_string() const
{
    return "(" + this->inner_expression->to_string() + ")";
}

grouping_expression::~grouping_expression()
{
    delete inner_expression;
}

literal_expression::literal_expression(lentus_string *string, int line_number, int start_position, int end_position)
{
    this->value = variable(string);
    this->line_number = line_number;
    this->start_position = start_position;
    this->end_position = end_position;
}

literal_expression::literal_expression(lentus_function *function, int line_number, int start_position, int end_position)
{
    this->value = variable(function);
    this->line_number = line_number;
    this->start_position = start_position;
    this->end_position = end_position;
}

literal_expression::literal_expression(literal_expression &&other) : value(std::move(other.value))
{
    other.value = variable();
}

literal_expression &literal_expression::operator=(literal_expression &&other)
{
    if (this != &other)
    {
        value = std::move(other.value);
        other.value = variable();
    }
    return *this;
}

literal_expression::~literal_expression()
{
}

std::string literal_expression::to_string() const
{
    switch (value.get_type())
    {
    case variable_type::NONE:
        return "none";
    case variable_type::BOOLEAN:
        return value.get_boolean() ? "true" : "false";
    case variable_type::NUMBER:
        return std::to_string(value.get_number());
    case variable_type::STRING:
        return "\"" + value.get_string()->get_value() + "\"";
    case variable_type::FUNCTION:
        return value.get_function()->to_string();
    default:
        return "literal";
    }
}

identifier_expression::identifier_expression(identifier_expression &&other)
{
    identifier = std::move(other.identifier);
    this->line_number = other.line_number;
    this->start_position = other.start_position;
    this->end_position = other.end_position;
}

identifier_expression &identifier_expression::operator=(identifier_expression &&other)
{
    if (this != &other)
    {
        identifier = std::move(other.identifier);
        this->line_number = other.line_number;
        this->start_position = other.start_position;
        this->end_position = other.end_position;
    }
    return *this;
}

std::string identifier_expression::to_string() const
{
    return identifier;
}

call_expression::call_expression(call_expression &&other)
{
    this->callee = other.callee;
    other.callee = nullptr;
    this->arguments = std::move(other.arguments);
    this->line_number = other.line_number;
    this->start_position = other.start_position;
    this->end_position = other.end_position;
}

call_expression &call_expression::operator=(call_expression &&other)
{
    if (this != &other)
    {
        delete callee;
        for (expression *argument : arguments)
        {
            delete argument;
        }
        this->callee = other.callee;
        other.callee = nullptr;
        this->arguments = std::move(other.arguments);
        this->line_number = other.line_number;
        this->start_position = other.start_position;
        this->end_position = other.end_position;
    }
    return *this;
}

std::string call_expression::to_string() const
{
    std::string str = "CALL[" + callee->to_string() + "(";
    for (int i = 0; i < (int)arguments.size(); i++)
    {
        if (i != 0)
        {
            str += ", ";
        }
        expression *argument = arguments[i];
        str += argument->to_string();
    }
    str += ")]";
    return str;
}

call_expression::~call_expression()
{
    delete callee;
    for (expression *argument : arguments)
    {
        delete argument;
    }
}

std::string function_expression::to_string() const
{
    std::string str = "FUNCTION[";
    if (name != nullptr)
    {
        for (size_t i = 0; i < parameter_names.size(); ++i)
        {
            if (i != 0)
            {
                str += ", ";
            }
            str += parameter_names[i]->get_value();
        }
    }
    else
    {
        str += "<anonymous>";
    }
    str += "] {\n" + body->to_string() + "}";
    return str;
}

function_expression::~function_expression()
{
    delete body;
}

reference error_expression::evaluate([[maybe_unused]] execution_context &execution) const
{
    throw lentus_runtime_exception("Tried to evaluate error_node");
}

reference literal_expression::evaluate([[maybe_unused]] execution_context &execution) const
{
    return this->value;
}

reference unary_expression::evaluate(execution_context &execution) const
{
    reference rhs = this->operand->evaluate(execution);

    switch (op.get_type())
    {
    case token_type::SUBTRACT: // negation
        if (!rhs->is(variable_type::NUMBER))
        {
            throw lentus_runtime_exception(std::string("Invalid type in negation: NUMBER expected, got ") + rhs->get_type_name() + " instead", this);
        }
        return reference(-rhs->get_number());
    case token_type::NOT:
        if (!rhs->is(variable_type::BOOLEAN))
        {
            throw lentus_runtime_exception(std::string("Invalid type in logical NOT: BOOLEAN expected, got ") + rhs->get_type_name() + " instead", this);
        }
        return reference(!rhs->get_boolean());
    case token_type::INCREMENT:
        if (!rhs->is(variable_type::NUMBER))
        {
            throw lentus_runtime_exception(std::string("Invalid type in increment: NUMBER expected, got ") + rhs->get_type_name() + " instead", this);
        }
        else if (rhs.is_modifiable_reference())
        {
            if (is_postfix) // var++;
            {
                reference original = rhs;
                *rhs = variable(rhs->get_number() + 1);
                return reference(*original); // Convert to rvalue
            }
            else // ++var;
            {
                *rhs = variable(rhs->get_number() + 1);
                return rhs;
            }
        }
        else
        {
            throw lentus_runtime_exception("Cannot apply increment operator to unmodifiable value", this);
        }
    case token_type::DECREMENT:
        if (!rhs->is(variable_type::NUMBER))
        {
            throw lentus_runtime_exception(std::string("Invalid type in decrement: NUMBER expected, got ") + rhs->get_type_name() + " instead", this);
        }
        else if (rhs.is_modifiable_reference())
        {
            if (is_postfix) // var--;
            {
                reference original = rhs;
                *rhs = variable(rhs->get_number() - 1);
                return reference(*original); // Convert to rvalue
            }
            else // --var;
            {
                *rhs = variable(rhs->get_number() - 1);
                return rhs;
            }
        }
        else
        {
            throw lentus_runtime_exception("Cannot apply decrement operator to unmodifiable value", this);
        }
    default:
        throw lentus_runtime_exception("Invalid unary operator token " + op.to_string(), this);
    }
}

/**
 * checks the data types for antisymmetric comparison operators (<=, <, >=, >)
 */
void antisymmetric_comparison_check(const binary_expression *ptr, reference lhs, reference rhs)
{
    if (lhs->get_type() == variable_type::TABLE || lhs->get_type() == variable_type::FUNCTION)
    {
        throw lentus_runtime_exception(std::string("Cannot use comparison operator with type ") + lhs->get_type_name(), ptr);
    }
    else if (lhs->get_type() == variable_type::LIST && lhs->get_list()->size() != rhs->get_list()->size())
    {
        throw lentus_runtime_exception(std::string("Cannot compare lists with mismatched sizes: lhs_size=") + std::to_string(lhs->get_list()->size()) + " rhs_size=" + std::to_string(rhs->get_list()->size()), ptr);
    }
}

reference binary_expression::evaluate(execution_context &execution) const
{
    reference lhs = this->left->evaluate(execution);
    reference rhs = this->right->evaluate(execution);

    switch (op.get_type())
    {
    case token_type::ADD:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return reference(lhs->get_number() + rhs->get_number());
        }
        else if (lhs->is(variable_type::STRING) && rhs->is(variable_type::STRING))
        {
            lentus_string *left_string = lhs->get_string();
            lentus_string *right_string = rhs->get_string();
            std::string str;
            str.reserve(left_string->get_value().size() + right_string->get_value().size());
            str.append(left_string->get_value());
            str.append(right_string->get_value());
            return reference(execution.get_program().create_string(std::move(str)));
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid / mismatched types in '+': lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::SUBTRACT:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return reference(lhs->get_number() - rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '-': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::MULTIPLY:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return reference(lhs->get_number() * rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '*': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::DIVIDE:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return reference(lhs->get_number() / rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '/': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            return reference(*lhs == *rhs);
        }
        else if (lhs->is(variable_type::NONE) || rhs->is(variable_type::NONE)) // Allow comparisons from any type to none
        {
            return reference(false);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"==\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::NOT_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            return reference(*lhs != *rhs);
        }
        else if (lhs->is(variable_type::NONE) || rhs->is(variable_type::NONE)) // Allow comparisons from any type to none
        {
            return reference(true);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"!=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::LESS:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);
            return reference(*lhs < *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"<\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::LESS_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return reference(*lhs <= *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"<=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::GREATER:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return reference(*lhs > *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \">\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::GREATER_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return reference(*lhs >= *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \">=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::OR:
        if (lhs->is(variable_type::BOOLEAN) && rhs->is(variable_type::BOOLEAN))
        {
            return reference(lhs->get_boolean() || rhs->get_boolean());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in \"or\": expected BOOLEAN, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::AND:
        if (lhs->is(variable_type::BOOLEAN) && rhs->is(variable_type::BOOLEAN))
        {
            return reference(lhs->get_boolean() && rhs->get_boolean());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in \"and\": expected BOOLEAN, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::ASSIGN:
        if (lhs.is_modifiable_reference())
        {
            *lhs = *rhs;
            return lhs;
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid assignment target: cannot assign to a non-modifiable reference"), this);
        }
    case token_type::ADD_ASSIGN:
        if (lhs.is_modifiable_reference())
        {
            if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
            {
                *lhs = variable(lhs->get_number() + rhs->get_number());
                return lhs;
            }
            else if (lhs->is(variable_type::STRING) && rhs->is(variable_type::STRING))
            {
                lentus_string *left_string = lhs->get_string();
                lentus_string *right_string = rhs->get_string();
                lentus_string *new_string = new lentus_string(std::string(**left_string + **right_string));
                *lhs = variable(execution.get_program().create_string(new_string));
                return lhs;
            }
            else
            {
                throw lentus_runtime_exception(std::string("Invalid / mismatched types in '+=': lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
            }
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid assignment target: cannot assign to a non-modifiable reference"), this);
        }
    case token_type::SUBTRACT_ASSIGN:
        if (lhs.is_modifiable_reference())
        {
            if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
            {
                *lhs = variable(lhs->get_number() - rhs->get_number());
                return lhs;
            }
            else
            {
                throw lentus_runtime_exception(std::string("Invalid types in '-=': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
            }
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid assignment target: cannot assign to a non-modifiable reference"), this);
        }
    case token_type::MULTIPLY_ASSIGN:
        if (lhs.is_modifiable_reference())
        {
            if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
            {
                *lhs = variable(lhs->get_number() * rhs->get_number());
                return lhs;
            }
            else
            {
                throw lentus_runtime_exception(std::string("Invalid types in '*=': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
            }
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid assignment target: cannot assign to a non-modifiable reference"), this);
        }
    case token_type::DIVIDE_ASSIGN:
        if (lhs.is_modifiable_reference())
        {
            if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
            {
                *lhs = variable(lhs->get_number() / rhs->get_number());
                return lhs;
            }
            else
            {
                throw lentus_runtime_exception(std::string("Invalid types in '/=': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
            }
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid assignment target: cannot assign to a non-modifiable reference"), this);
        }
    default:
        throw lentus_runtime_exception("Invalid binary operator token " + op.to_string(), this);
    }
}

reference grouping_expression::evaluate(execution_context &execution) const
{
    return this->inner_expression->evaluate(execution);
}

reference identifier_expression::evaluate(execution_context &execution) const
{
    std::optional<reference> opt = execution.get_variable(identifier);
    if (opt)
    {
        return opt.value();
    }
    else
    {
        throw lentus_runtime_exception(std::string("Unrecognized symbol " + identifier), this);
    }
}

reference call_expression::evaluate(execution_context &execution) const
{
    reference callee_var = callee->evaluate(execution);
    if (callee_var->is(variable_type::FUNCTION))
    {
        function *fn = callee_var->get_function();
        std::vector<variable> argument_values;
        for (expression *argument : arguments)
        {
            argument_values.push_back(argument->evaluate(execution).get());
        }
        return fn->evaluate(execution, argument_values, this);
    }
    else
    {
        throw lentus_runtime_exception(std::string("Attempted to call a non-function value of type ") + callee_var->get_type_name(), this);
    }

    return variable();
}

function_expression::function_expression(function_expression &&other)
{
    name = other.name;
    other.name = nullptr;
    body = other.body;
    other.body = nullptr;
    parameter_names = std::move(other.parameter_names);
    this->line_number = other.line_number;
    this->start_position = other.start_position;
    this->end_position = other.end_position;
}

function_expression &function_expression::operator=(function_expression &&other)
{
    if (this != &other)
    {
        delete body;
        name = other.name;
        other.name = nullptr;
        body = other.body;
        other.body = nullptr;
        parameter_names = std::move(other.parameter_names);
        this->line_number = other.line_number;
        this->start_position = other.start_position;
        this->end_position = other.end_position;
    }
    return *this;
}

reference function_expression::evaluate(execution_context &execution) const
{
    program_module *current_module = execution.get_current_module();
    std::vector<stack_frame> stack = execution.get_stack();
    lentus_function *current_fn = execution.get_current_function();

    std::vector<lentus_table *> captured_stack;

    // Capture local variables
    for (int i = stack.size() - 1; i >= 0; i--)
    {
        stack_frame &frame = stack[i];
        if (frame.current_module != current_module)
        {
            break;
        }
        captured_stack.push_back(frame.local_variables);
    }

    // Capture local variables from current function
    if (current_fn != nullptr)
    {
        std::vector<lentus_table *> previous_captured_stack = current_fn->get_captured_variable_tables();
        for (int i = previous_captured_stack.size() - 1; i >= 0; i--)
        {
            captured_stack.push_back(previous_captured_stack[i]);
        }
    }

    // after reversing
    // local variables at the end (higher priority)
    // captures from other functions at the start (lower priority)
    std::reverse(captured_stack.begin(), captured_stack.end());

    return execution.get_program().create_function(new lentus_function(name, body, &parameter_names, current_module, std::move(captured_stack)));
}

reference subscript_expression::evaluate(execution_context &execution) const
{
    reference left_ref = left->evaluate(execution);
    reference right_ref = right->evaluate(execution);

    if (left_ref->is(variable_type::TABLE)) // Index into table
    {
        if (right_ref->is(variable_type::TABLE) || right_ref->is(variable_type::FUNCTION) || right_ref->is(variable_type::LIST))
        {
            throw lentus_runtime_exception("Invalid table key type " + right_ref->get_type_name(), this);
        }
        lentus_table *table = left_ref->get_table();
        return reference(&table->operator[](*right_ref));
    }
    else if (left_ref->is(variable_type::LIST)) // Index into list
    {
        lentus_list *list = left_ref->get_list();
        if (!right_ref->is(variable_type::NUMBER))
        {
            throw lentus_runtime_exception(std::string("List index must be a number, got ") + right_ref->get_type_name(), this);
        }
        long long index = (long long)right_ref->get_number();
        if (index < 0 || (size_t)index >= list->size())
        {
            throw lentus_runtime_exception(std::string("List index ") + std::to_string(index) + " out of bounds for list size " + std::to_string(list->size()), this);
        }
        return reference(&(*list)[index]);
    }
    else
    {
        throw lentus_runtime_exception(std::string("Attempted to index into a value of type ") + left_ref->get_type_name(), this);
    }
}

#include "ast.hpp"
#include "program.hpp"

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
        op = std::move(other.op);
        operand = other.operand;
        other.operand = nullptr;
    }
    return *this;
}

std::string unary_expression::to_string() const
{
    return this->op.to_string() + "(" + this->operand->to_string() + ")";
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

literal_expression::literal_expression(literal_expression &&other) : value(std::move(other.value))
{
}

literal_expression &literal_expression::operator=(literal_expression &&other)
{
    if (this != &other)
    {
        value = std::move(other.value);
    }
    return *this;
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
        return "\"" + *value.get_string() + "\"";
    default:
        return "literal";
    }
}

literal_expression::~literal_expression()
{
}

variable *error_node::evaluate(program &program) const
{
    throw new lentus_runtime_exception("Tried to evaluate error_node");
}

variable *literal_expression::evaluate(program &program) const
{
    return &program.add_variable(this->value);
}

variable *unary_expression::evaluate(program &program) const
{
    variable *rhs = this->operand->evaluate(program);

    switch (op.get_type())
    {
    case token_type::SUBTRACT: // negation
        if (!rhs->is(variable_type::NUMBER))
        {
            throw lentus_runtime_exception(std::string("Invalid type in negation: NUMBER expected, got ") + rhs->get_type_name() + " instead", this);
        }
        return &program.add_variable(-rhs->get_number());
    case token_type::NOT:
        if (!rhs->is(variable_type::BOOLEAN))
        {
            throw lentus_runtime_exception(std::string("Invalid type in logical NOT: BOOLEAN expected, got ") + rhs->get_type_name() + " instead", this);
        }
        return &program.add_variable(!rhs->get_boolean());
    default:
        throw lentus_runtime_exception("Invalid unary operator token " + op.to_string(), this);
    }
}

/**
 * Helper function for checking the data for antisymmetric comparison operators (<=, <, >=, >)
 */
void antisymmetric_comparison_check(const binary_expression *ptr, variable *lhs, variable *rhs)
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

variable *binary_expression::evaluate(program &program) const
{
    variable *lhs = this->left->evaluate(program);
    variable *rhs = this->right->evaluate(program);

    switch (op.get_type())
    {
    case token_type::ADD:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return &program.add_variable(lhs->get_number() + rhs->get_number());
        }
        else if (lhs->is(variable_type::STRING) && rhs->is(variable_type::STRING))
        {
            const std::string *left_string = lhs->get_string();
            const std::string *right_string = rhs->get_string();
            std::string new_string = std::string(left_string->size() + right_string->size(), '\0');
            new_string += *left_string;
            new_string += *right_string;
            return &program.add_variable(new_string);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid / mismatched types in '+': lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::SUBTRACT:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return &program.add_variable(lhs->get_number() - rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '-': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::MULTIPLY:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return &program.add_variable(lhs->get_number() * rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '*': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::DIVIDE:
        if (lhs->is(variable_type::NUMBER) && rhs->is(variable_type::NUMBER))
        {
            return &program.add_variable(lhs->get_number() / rhs->get_number());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in '/': NUMBER expected, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            return &program.add_variable(*lhs == *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"==\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::NOT_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            return &program.add_variable(*lhs != *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"!=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::LESS:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);
            return &program.add_variable(*lhs < *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"<\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::LESS_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return &program.add_variable(*lhs <= *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \"<=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::GREATER:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return &program.add_variable(*lhs > *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \">\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::GREATER_EQUAL:
        if (lhs->get_type() == rhs->get_type())
        {
            antisymmetric_comparison_check(this, lhs, rhs);

            return &program.add_variable(*lhs >= *rhs);
        }
        else
        {
            throw lentus_runtime_exception(std::string("Mismatched types in \">=\", got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::OR:
        if (lhs->is(variable_type::BOOLEAN) && rhs->is(variable_type::BOOLEAN))
        {
            return &program.add_variable(lhs->get_boolean() || rhs->get_boolean());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in \"or\": expected BOOLEAN, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    case token_type::AND:
        if (lhs->is(variable_type::BOOLEAN) && rhs->is(variable_type::BOOLEAN))
        {
            return &program.add_variable(lhs->get_boolean() && rhs->get_boolean());
        }
        else
        {
            throw lentus_runtime_exception(std::string("Invalid types in \"or\": expected BOOLEAN, got lhs=") + lhs->get_type_name() + " rhs=" + rhs->get_type_name(), this);
        }
    default:
        throw lentus_runtime_exception("Invalid binary operator token " + op.to_string(), this);
    }
}

variable *grouping_expression::evaluate(program &program) const
{
    return this->inner_expression->evaluate(program);
}
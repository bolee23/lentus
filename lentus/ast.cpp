#include "ast.hpp"

variable *error_node::evaluate(program &program) const
{
    return nullptr;
}

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

variable *unary_expression::evaluate(program &program) const
{
    return nullptr;
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

variable *binary_expression::evaluate(program &program) const
{
    return nullptr;
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

variable *grouping_expression::evaluate(program &program) const
{
    return nullptr;
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

variable *literal_expression::evaluate(program &program) const
{
    return nullptr;
}

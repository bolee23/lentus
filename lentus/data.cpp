#include "data.hpp"
#include <magic_enum/magic_enum.hpp>
#include <cmath>
#include <format>
#include <iostream>
#include "function.hpp"

namespace lentus
{

std::size_t value_hasher::operator()(const value &var) const
{
    switch (var.get_type())
    {
    case value_type::NONE:
        return 0;
    case value_type::BOOLEAN:
        return std::hash<bool>()(var.get_boolean()) + 1;
    case value_type::NUMBER:
        return std::hash<double>()(var.get_number());
    case value_type::STRING:
        return std::hash<std::string>()(var.get_string()->get_value());
    case value_type::LIST: // lists, tables and functions are not allowed to be used as keys
        return 0;
    case value_type::TABLE:
        return 0;
    case value_type::FUNCTION:
        return 0;
    default:
        return 0;
    }
}

bool value_equal::operator()(const value &lhs, const value &rhs) const
{
    return lhs == rhs;
}

value::value(value_type type) : type(type)
{
    switch (type)
    {
    case value_type::NONE:
        break;
    case value_type::BOOLEAN:
        boolean = false;
        break;
    case value_type::NUMBER:
        number = 0;
        break;
    case value_type::STRING:
        string_value = nullptr;
        break;
    case value_type::LIST:
        list_value = nullptr;
        break;
    case value_type::TABLE:
        table_value = nullptr;
        break;
    case value_type::FUNCTION:
        fn = nullptr;
        break;
    default:
        break;
    }
}

value::value(const value &var) : type(var.type)
{
    switch (type)
    {
    case value_type::NONE:
        break;
    case value_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case value_type::NUMBER:
        number = var.number;
        break;
    case value_type::STRING:
        string_value = var.string_value;
        break;
    case value_type::LIST:
        list_value = var.list_value;
        break;
    case value_type::TABLE:
        table_value = var.table_value;
        break;
    case value_type::FUNCTION:
        fn = var.fn;
        break;
    }
}

value::value(value &&var) : type(var.type)
{
    switch (type)
    {
    case value_type::NONE:
        break;
    case value_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case value_type::NUMBER:
        number = var.number;
        break;
    case value_type::STRING:
        string_value = var.string_value;
        var.string_value = nullptr;
        break;
    case value_type::LIST:
        list_value = var.list_value;
        var.list_value = nullptr;
        break;
    case value_type::TABLE:
        table_value = var.table_value;
        var.table_value = nullptr;
        break;
    case value_type::FUNCTION:
        fn = var.fn;
        var.fn = nullptr;
        break;
    }
    var.type = value_type::NONE;
}

value &value::operator=(const value &var)
{
    if (this == &var)
        return *this;

    type = var.type;
    switch (type)
    {
    case value_type::NONE:
        break;
    case value_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case value_type::NUMBER:
        number = var.number;
        break;
    case value_type::STRING:
        string_value = var.string_value;
        break;
    case value_type::LIST:
        list_value = var.list_value;
        break;
    case value_type::TABLE:
        table_value = var.table_value;
        break;
    case value_type::FUNCTION:
        fn = var.fn;
        break;
    }
    return *this;
}

value &value::operator=(value &&var)
{
    if (this == &var)
        return *this;

    type = var.type;
    switch (type)
    {
    case value_type::NONE:
        break;
    case value_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case value_type::NUMBER:
        number = var.number;
        break;
    case value_type::STRING:
        string_value = var.string_value;
        var.string_value = nullptr;
        break;
    case value_type::LIST:
        list_value = var.list_value;
        var.list_value = nullptr;
        break;
    case value_type::TABLE:
        table_value = var.table_value;
        var.table_value = nullptr;
        break;
    case value_type::FUNCTION:
        fn = var.fn;
        var.fn = nullptr;
        break;
    }
    var.type = value_type::NONE;
    return *this;
}

value::~value()
{
    switch (type)
    {
    case value_type::STRING:
        string_value = nullptr;
        break;
    case value_type::LIST:
        list_value = nullptr;
        break;
    case value_type::TABLE:
        table_value = nullptr;
        break;
    case value_type::FUNCTION:
        fn = nullptr;
        break;
    default:
        break;
    }
}

bool value::operator==(const value &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case value_type::NONE:
        return true;
    case value_type::BOOLEAN:
        return this->boolean == var.boolean;
    case value_type::NUMBER:
        return this->number == var.number;
    case value_type::STRING:
        return this->string_value->get_value() == var.string_value->get_value();
    case value_type::LIST:
        if (this->list_value->size() != var.list_value->size())
        {
            return false;
        }

        for (size_t i = 0; i < this->list_value->size(); i++)
        {
            if (this->list_value->at(i) != var.list_value->at(i))
            {
                return false;
            }
        }
        return true;
    case value_type::TABLE:
        if (this->table_value->size() != var.table_value->size())
        {
            return false;
        }

        for (const auto &[key, value] : *this->table_value)
        {
            if (var.table_value->contains(key))
            {
                if (value != var.table_value->at(key))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    case value_type::FUNCTION:
        return this->fn == var.fn;
    default:
        return false;
    }
}

bool value::operator<=(const value &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case value_type::NONE:
        return true;
    case value_type::BOOLEAN:
        return this->boolean <= var.boolean;
    case value_type::NUMBER:
        return this->number <= var.number;
    case value_type::STRING:
        return this->string_value->get_value() <= var.string_value->get_value();
    case value_type::LIST:
        if (this->list_value->size() != var.list_value->size())
        {
            return false;
        }

        // Lexical ordering
        for (size_t i = 0; i < this->list_value->size(); i++)
        {
            if (this->list_value->at(i) < var.list_value->at(i))
            {
                return true;
            }
            else if (this->list_value->at(i) > var.list_value->at(i))
            {
                return false;
            }
        }

        // They equal, return true
        return true;
    case value_type::TABLE:
        return false;
    case value_type::FUNCTION:
        return false;
    default:
        return false;
    }
}

bool value::operator<(const value &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case value_type::NONE:
        return false;
    case value_type::BOOLEAN:
        return this->boolean < var.boolean;
    case value_type::NUMBER:
        return this->number < var.number;
    case value_type::STRING:
        return this->string_value->get_value() < var.string_value->get_value();
    case value_type::LIST:
        if (this->list_value->size() != var.list_value->size())
        {
            return false;
        }

        // Lexical ordering
        for (size_t i = 0; i < this->list_value->size(); i++)
        {
            if (this->list_value->at(i) < var.list_value->at(i))
            {
                return true;
            }
            else if (this->list_value->at(i) > var.list_value->at(i))
            {
                return false;
            }
        }

        // They equal, return false
        return false;
    case value_type::TABLE:
        return false;
    case value_type::FUNCTION:
        return false;
    default:
        return false;
    }
}

double value::get_number() const
{
    if (type == value_type::NUMBER)
    {
        return number;
    }
    else
    {
        return 0;
    }
}

const string *value::get_string() const
{
    if (type == value_type::STRING)
    {
        return string_value;
    }
    else
    {
        return nullptr;
    }
}

const list *value::get_list() const
{
    if (type == value_type::LIST)
    {
        return list_value;
    }
    else
    {
        return nullptr;
    }
}

const table *value::get_table() const
{
    if (type == value_type::TABLE)
    {
        return table_value;
    }
    else
    {
        return nullptr;
    }
}

string *value::get_string()
{
    if (type == value_type::STRING)
    {
        return string_value;
    }
    else
    {
        return nullptr;
    }
}

list *value::get_list()
{
    if (type == value_type::LIST)
    {
        return list_value;
    }
    else
    {
        return nullptr;
    }
}

table *value::get_table()
{
    if (type == value_type::TABLE)
    {
        return table_value;
    }
    else
    {
        return nullptr;
    }
}

function *value::get_function()
{
    return fn;
}

const function *value::get_function() const
{
    if (type == value_type::FUNCTION)
    {
        return fn;
    }
    else
    {
        return nullptr;
    }
}

bool value::get_boolean() const
{
    if (type == value_type::BOOLEAN)
    {
        return boolean;
    }
    else
    {
        return false;
    }
}

std::string value::get_type_name() const
{
    return std::string(magic_enum::enum_name(type));
}

void value::write_to_string_recursive(std::string &str) const
{
    switch (type)
    {
    case value_type::NONE:
        str += "none";
        break;
    case value_type::BOOLEAN:
        if (boolean)
        {
            str += "true";
        }
        else
        {
            str += "false";
        }
        break;
    case value_type::NUMBER:
        str += std::to_string(number);
        break;
    case value_type::STRING:
        str += string_value->get_value();
        break;
    case value_type::LIST:
    {
        str += "[";
        for (size_t i = 0; i < list_value->size(); i++)
        {
            if (i != 0)
            {
                str += ", ";
            }
            value &var = (*list_value)[i];
            var.write_to_string_recursive(str);
        }
        break;
    }
    case value_type::TABLE:
    {
        str += "{";
        bool first = true;
        for (auto entry : *table_value)
        {
            if (!first)
            {
                str += ", ";
            }
            else
            {
                first = false;
            }

            entry.first.write_to_string_recursive(str);
            str += " = ";
            entry.second.write_to_string_recursive(str);
        }
        str += "}";
        break;
    }
    case value_type::FUNCTION:
        str += "<function @" + std::to_string((size_t)this->fn) + ">";
        break;
    default:
        str += "<error>";
        break;
    }
}

std::string value::to_string_recursive() const
{
    switch (type)
    {
    case value_type::NONE:
        return "none";
    case value_type::BOOLEAN:
        if (boolean)
        {
            return "true";
        }
        else
        {
            return "false";
        }
    case value_type::NUMBER:
        if (std::floor(number) == number)
        {
            return std::to_string((long)number);
        }
        else
        {
            return std::to_string(number);
        }
    case value_type::STRING:
        return "\"" + string_value->get_value() + "\"";
    case value_type::LIST:
    {
        std::string str = "[";
        for (size_t i = 0; i < list_value->size(); i++)
        {
            if (i != 0)
            {
                str += ", ";
            }
            value &var = (*list_value)[i];
            var.write_to_string_recursive(str);
        }
        str += "]";
        return str;
    }
    case value_type::TABLE:
    {
        std::string str = "{";
        bool first = true;
        for (auto entry : *table_value)
        {
            if (!first)
            {
                str += ", ";
            }
            else
            {
                first = false;
            }

            entry.first.write_to_string_recursive(str);
            str += " = ";
            entry.second.write_to_string_recursive(str);
        }
        str += "}";
        return str;
    }
    case value_type::FUNCTION:
        return "<function @" + std::to_string((size_t)this->fn) + ">";
    default:
        return "<error>";
    }
}

std::string value::to_string() const
{
    switch (type)
    {
    case value_type::NONE:
        return "none";
    case value_type::BOOLEAN:
        if (boolean)
        {
            return "true";
        }
        else
        {
            return "false";
        }
    case value_type::NUMBER:
        return std::format("{:g}", number);
    case value_type::STRING:
        return string_value->get_value();
    case value_type::LIST:
    {
        return "<list @" + std::to_string((size_t)this->list_value) + ">";
    }
    case value_type::TABLE:
    {
        return "<table @" + std::to_string((size_t)this->table_value) + ">";
    }
    case value_type::FUNCTION:
        return this->fn->to_string();
    default:
        return "<error>";
    }
}
} // namespace lentus

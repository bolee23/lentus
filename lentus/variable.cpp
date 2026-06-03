#include "variable.hpp"
#include <magic_enum/magic_enum.hpp>
#include <cmath>
#include <format>
#include <iostream>
#include "function.hpp"

std::size_t variable_hasher::operator()(const variable &var) const
{
    switch (var.get_type())
    {
    case variable_type::NONE:
        return 0;
    case variable_type::BOOLEAN:
        return std::hash<bool>()(var.get_boolean()) + 1;
    case variable_type::NUMBER:
        return std::hash<double>()(var.get_number());
    case variable_type::STRING:
        return std::hash<std::string>()(var.get_string()->get_value());
    case variable_type::LIST: // lists, tables and functions are not allowed to be used as keys
        return 0;
    case variable_type::TABLE:
        return 0;
    case variable_type::FUNCTION:
        return 0;
    default:
        return 0;
    }
}

bool variable_equal::operator()(const variable &lhs, const variable &rhs) const
{
    return lhs == rhs;
}

variable::variable(variable_type type) : type(type)
{
    switch (type)
    {
    case variable_type::NONE:
        break;
    case variable_type::BOOLEAN:
        boolean = false;
        break;
    case variable_type::NUMBER:
        number = 0;
        break;
    case variable_type::STRING:
        string = nullptr;
        break;
    case variable_type::LIST:
        list = nullptr;
        break;
    case variable_type::TABLE:
        table = nullptr;
        break;
    case variable_type::FUNCTION:
        fn = nullptr;
        break;
    default:
        break;
    }
}

variable::variable(const variable &var) : type(var.type)
{
    switch (type)
    {
    case variable_type::NONE:
        break;
    case variable_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case variable_type::NUMBER:
        number = var.number;
        break;
    case variable_type::STRING:
        string = var.string;
        break;
    case variable_type::LIST:
        list = var.list;
        break;
    case variable_type::TABLE:
        table = var.table;
        break;
    case variable_type::FUNCTION:
        fn = var.fn;
        break;
    }
}

variable::variable(variable &&var) : type(var.type)
{
    switch (type)
    {
    case variable_type::NONE:
        break;
    case variable_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case variable_type::NUMBER:
        number = var.number;
        break;
    case variable_type::STRING:
        string = var.string;
        var.string = nullptr;
        break;
    case variable_type::LIST:
        list = var.list;
        var.list = nullptr;
        break;
    case variable_type::TABLE:
        table = var.table;
        var.table = nullptr;
        break;
    case variable_type::FUNCTION:
        fn = var.fn;
        var.fn = nullptr;
        break;
    }
    var.type = variable_type::NONE;
}

variable &variable::operator=(const variable &var)
{
    if (this == &var)
        return *this;

    type = var.type;
    switch (type)
    {
    case variable_type::NONE:
        break;
    case variable_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case variable_type::NUMBER:
        number = var.number;
        break;
    case variable_type::STRING:
        string = var.string;
        break;
    case variable_type::LIST:
        list = var.list;
        break;
    case variable_type::TABLE:
        table = var.table;
        break;
    case variable_type::FUNCTION:
        fn = var.fn;
        break;
    }
    return *this;
}

variable &variable::operator=(variable &&var)
{
    if (this == &var)
        return *this;

    type = var.type;
    switch (type)
    {
    case variable_type::NONE:
        break;
    case variable_type::BOOLEAN:
        boolean = var.boolean;
        break;
    case variable_type::NUMBER:
        number = var.number;
        break;
    case variable_type::STRING:
        string = var.string;
        var.string = nullptr;
        break;
    case variable_type::LIST:
        list = var.list;
        var.list = nullptr;
        break;
    case variable_type::TABLE:
        table = var.table;
        var.table = nullptr;
        break;
    case variable_type::FUNCTION:
        fn = var.fn;
        var.fn = nullptr;
        break;
    }
    var.type = variable_type::NONE;
    return *this;
}

variable::~variable()
{
    switch (type)
    {
    case variable_type::STRING:
        string = nullptr;
        break;
    case variable_type::LIST:
        list = nullptr;
        break;
    case variable_type::TABLE:
        table = nullptr;
        break;
    case variable_type::FUNCTION:
        fn = nullptr;
        break;
    default:
        break;
    }
}

bool variable::operator==(const variable &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case variable_type::NONE:
        return true;
    case variable_type::BOOLEAN:
        return this->boolean == var.boolean;
    case variable_type::NUMBER:
        return this->number == var.number;
    case variable_type::STRING:
        return this->string->get_value() == var.string->get_value();
    case variable_type::LIST:
        if (this->list->size() != var.list->size())
        {
            return false;
        }

        for (size_t i = 0; i < this->list->size(); i++)
        {
            if (this->list->at(i) != var.list->at(i))
            {
                return false;
            }
        }
        return true;
    case variable_type::TABLE:
        if (this->table->size() != var.table->size())
        {
            return false;
        }

        for (const auto &[key, value] : *this->table)
        {
            if (var.table->contains(key))
            {
                if (value != var.table->at(key))
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
    case variable_type::FUNCTION:
        return this->fn == var.fn;
    default:
        return false;
    }
}

bool variable::operator<=(const variable &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case variable_type::NONE:
        return true;
    case variable_type::BOOLEAN:
        return this->boolean <= var.boolean;
    case variable_type::NUMBER:
        return this->number <= var.number;
    case variable_type::STRING:
        return this->string->get_value() <= var.string->get_value();
    case variable_type::LIST:
        if (this->list->size() != var.list->size())
        {
            return false;
        }

        // Lexical ordering
        for (size_t i = 0; i < this->list->size(); i++)
        {
            if (this->list->at(i) < var.list->at(i))
            {
                return true;
            }
            else if (this->list->at(i) > var.list->at(i))
            {
                return false;
            }
        }

        // They equal, return true
        return true;
    case variable_type::TABLE:
        return false;
    case variable_type::FUNCTION:
        return false;
    default:
        return false;
    }
}

bool variable::operator<(const variable &var) const
{
    if (this->get_type() != var.get_type())
        return false;
    switch (this->get_type())
    {
    case variable_type::NONE:
        return false;
    case variable_type::BOOLEAN:
        return this->boolean < var.boolean;
    case variable_type::NUMBER:
        return this->number < var.number;
    case variable_type::STRING:
        return this->string->get_value() < var.string->get_value();
    case variable_type::LIST:
        if (this->list->size() != var.list->size())
        {
            return false;
        }

        // Lexical ordering
        for (size_t i = 0; i < this->list->size(); i++)
        {
            if (this->list->at(i) < var.list->at(i))
            {
                return true;
            }
            else if (this->list->at(i) > var.list->at(i))
            {
                return false;
            }
        }

        // They equal, return false
        return false;
    case variable_type::TABLE:
        return false;
    case variable_type::FUNCTION:
        return false;
    default:
        return false;
    }
}

double variable::get_number() const
{
    if (type == variable_type::NUMBER)
    {
        return number;
    }
    else
    {
        return 0;
    }
}

const lentus_string *variable::get_string() const
{
    if (type == variable_type::STRING)
    {
        return string;
    }
    else
    {
        return nullptr;
    }
}

const lentus_list *variable::get_list() const
{
    if (type == variable_type::LIST)
    {
        return list;
    }
    else
    {
        return nullptr;
    }
}

const lentus_table *variable::get_table() const
{
    if (type == variable_type::TABLE)
    {
        return table;
    }
    else
    {
        return nullptr;
    }
}

lentus_string *variable::get_string()
{
    if (type == variable_type::STRING)
    {
        return string;
    }
    else
    {
        return nullptr;
    }
}

lentus_list *variable::get_list()
{
    if (type == variable_type::LIST)
    {
        return list;
    }
    else
    {
        return nullptr;
    }
}

lentus_table *variable::get_table()
{
    if (type == variable_type::TABLE)
    {
        return table;
    }
    else
    {
        return nullptr;
    }
}

function *variable::get_function()
{
    return fn;
}

const function *variable::get_function() const
{
    if (type == variable_type::FUNCTION)
    {
        return fn;
    }
    else
    {
        return nullptr;
    }
}

bool variable::get_boolean() const
{
    if (type == variable_type::BOOLEAN)
    {
        return boolean;
    }
    else
    {
        return false;
    }
}

std::string variable::get_type_name() const
{
    return std::string(magic_enum::enum_name(type));
}

void variable::write_to_string_recursive(std::string &str) const
{
    switch (type)
    {
    case variable_type::NONE:
        str += "none";
        break;
    case variable_type::BOOLEAN:
        if (boolean)
        {
            str += "true";
        }
        else
        {
            str += "false";
        }
        break;
    case variable_type::NUMBER:
        str += std::to_string(number);
        break;
    case variable_type::STRING:
        str += string->get_value();
        break;
    case variable_type::LIST:
    {
        str += "[";
        for (size_t i = 0; i < list->size(); i++)
        {
            if (i != 0)
            {
                str += ", ";
            }
            variable &var = (*list)[i];
            var.write_to_string_recursive(str);
        }
        break;
    }
    case variable_type::TABLE:
    {
        str += "{";
        bool first = true;
        for (auto entry : *table)
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
    case variable_type::FUNCTION:
        str += "<function @" + std::to_string((size_t)this->fn) + ">";
        break;
    default:
        str += "<error>";
        break;
    }
}

std::string variable::to_string_recursive() const
{
    switch (type)
    {
    case variable_type::NONE:
        return "none";
    case variable_type::BOOLEAN:
        if (boolean)
        {
            return "true";
        }
        else
        {
            return "false";
        }
    case variable_type::NUMBER:
        if (std::floor(number) == number)
        {
            return std::to_string((long)number);
        }
        else
        {
            return std::to_string(number);
        }
    case variable_type::STRING:
        return "\"" + string->get_value() + "\"";
    case variable_type::LIST:
    {
        std::string str = "[";
        for (size_t i = 0; i < list->size(); i++)
        {
            if (i != 0)
            {
                str += ", ";
            }
            variable &var = (*list)[i];
            var.write_to_string_recursive(str);
        }
        str += "]";
        return str;
    }
    case variable_type::TABLE:
    {
        std::string str = "{";
        bool first = true;
        for (auto entry : *table)
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
    case variable_type::FUNCTION:
        return "<function @" + std::to_string((size_t)this->fn) + ">";
    default:
        return "<error>";
    }
}

std::string variable::to_string() const
{
    switch (type)
    {
    case variable_type::NONE:
        return "none";
    case variable_type::BOOLEAN:
        if (boolean)
        {
            return "true";
        }
        else
        {
            return "false";
        }
    case variable_type::NUMBER:
        return std::format("{:g}", number);
    case variable_type::STRING:
        return string->get_value();
    case variable_type::LIST:
    {
        return "<list @" + std::to_string((size_t)this->list) + ">";
    }
    case variable_type::TABLE:
    {
        return "<table @" + std::to_string((size_t)this->table) + ">";
    }
    case variable_type::FUNCTION:
        return this->fn->to_string();
    default:
        return "<error>";
    }
}
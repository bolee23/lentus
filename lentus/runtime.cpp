#include "runtime.hpp"

std::size_t variable_hasher::operator()(const variable *var) const
{
    switch (var->get_type())
    {
    case variable_type::NONE:
        return 0;
    case variable_type::BOOLEAN:
        return std::hash<bool>()(var->boolean) + 1;
    case variable_type::NUMBER:
        return std::hash<double>()(var->number);
    case variable_type::STRING:
        return std::hash<std::string>()(*var->string);
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

bool variable_equal::operator()(const variable *lhs, const variable *rhs) const
{
    return *lhs == *rhs;
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
        string = new std::string();
        break;
    case variable_type::LIST:
        list = new std::vector<variable *>();
        break;
    case variable_type::TABLE:
        table = new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(0, variable_hasher(), variable_equal());
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
        boolean = false;
        break;
    case variable_type::NUMBER:
        number = 0;
        break;
    case variable_type::STRING:
        string = new std::string();
        break;
    case variable_type::LIST:
        list = new std::vector<variable *>();
        break;
    case variable_type::TABLE:
        table = new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(0, variable_hasher(), variable_equal());
        break;
    case variable_type::FUNCTION:
        fn = nullptr;
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

    this->~variable();
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
        string = new std::string(*var.string);
        break;
    case variable_type::LIST:
        list = new std::vector<variable *>(*var.list);
        break;
    case variable_type::TABLE:
        table = new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(*var.table);
        break;
    case variable_type::FUNCTION:
        fn = new function(*var.fn);
        break;
    }
    return *this;
}

variable &variable::operator=(variable &&var)
{
    if (this == &var)
        return *this;

    this->~variable();
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
        delete string;
        string = nullptr;
        break;
    case variable_type::LIST:
        delete list;
        list = nullptr;
        break;
    case variable_type::TABLE:
        delete table;
        table = nullptr;
        break;
    case variable_type::FUNCTION:
        delete fn;
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
        return *this->string == *var.string;
    case variable_type::LIST:
        if (this->list->size() != var.list->size())
        {
            return false;
        }

        for (size_t i = 0; i < this->list->size(); i++)
        {
            if (*(*this->list)[i] != *(*var.list)[i])
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
                if (*value != *(*var.table)[key])
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

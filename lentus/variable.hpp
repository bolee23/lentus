#ifndef VARIABLE_HPP
#define VARIABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>

enum class variable_type : int
{
    NONE,
    BOOLEAN,
    NUMBER,
    STRING,
    LIST,
    TABLE,
    FUNCTION
};

class ast_node;

class function
{
private:
    ast_node *body;
    std::vector<std::string> parameters;

public:
    function(ast_node *body, std::vector<std::string> &&parameters) : body(body), parameters(parameters) {}

    function(const function &fn) : body(fn.body), parameters(fn.parameters) {}
};

class variable;

struct variable_hasher
{
    std::size_t operator()(const variable *var) const;
};

struct variable_equal
{
    bool operator()(const variable *lhs, const variable *rhs) const;
};

class variable
{
private:
    variable_type type;

public:
    union
    {
        bool boolean;
        double number;
        std::string *string;
        std::vector<variable *> *list;
        std::unordered_map<variable *, variable *, variable_hasher, variable_equal> *table;
        function *fn;
    };

    variable(variable_type type);
    variable(bool value) : type(variable_type::BOOLEAN), boolean(value) {}
    variable(double value) : type(variable_type::NUMBER), number(value) {}
    variable(std::string &value) : type(variable_type::STRING), string(new std::string(value)) {}
    variable(std::vector<variable *> &value) : type(variable_type::LIST), list(new std::vector<variable *>(value)) {}
    variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value) : type(variable_type::TABLE), table(new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(value)) {}
    variable(function *value) : type(variable_type::FUNCTION), fn(value) {}

    variable(const variable &var);

    variable(variable &&var);

    variable &operator=(const variable &var);

    variable &operator=(variable &&var);

    variable_type get_type() const { return type; }

    ~variable();

    bool operator==(const variable &var) const;
    bool operator!=(const variable &var) const { return !(*this == var); }
};

#endif
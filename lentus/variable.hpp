#ifndef LENTUS_VARIABLE_HPP
#define LENTUS_VARIABLE_HPP

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
    union
    {
        bool boolean;
        double number;
        const std::string *string;
        std::vector<variable *> *list;
        std::unordered_map<variable *, variable *, variable_hasher, variable_equal> *table;
        const function *fn;
    };

public:
    variable() : type(variable_type::NONE) {}
    variable(variable_type type);
    variable(bool value) : type(variable_type::BOOLEAN), boolean(value) {}
    variable(double value) : type(variable_type::NUMBER), number(value) {}

    variable(const std::string *value) : type(variable_type::STRING), string(value) {};
    variable(const std::string &value) : type(variable_type::STRING), string(new std::string(value)) {}
    variable(const std::string &&value) : type(variable_type::STRING), string(new std::string(std::move(value))) {}

    variable(const std::vector<variable *> &value) : type(variable_type::LIST), list(new std::vector<variable *>(value)) {}
    variable(const std::vector<variable *> &&value) : type(variable_type::LIST), list(new std::vector<variable *>(std::move(value))) {}

    variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value) : type(variable_type::TABLE), table(new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(value)) {}
    variable(const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &&value) : type(variable_type::TABLE), table(new std::unordered_map<variable *, variable *, variable_hasher, variable_equal>(std::move(value))) {}

    variable(function const *value) : type(variable_type::FUNCTION), fn(value) {}

    variable(const variable &var);
    variable(variable &&var);

    variable &operator=(const variable &var);
    variable &operator=(variable &&var);
    variable_type get_type() const { return type; }
    bool is(variable_type type) const { return this->type == type; }
    std::string get_type_name() const;

    void write_to_string(std::string &str);
    std::string to_string() const;

    ~variable();

    bool operator==(const variable &var) const;
    bool operator!=(const variable &var) const { return !(*this == var); }

    bool operator<=(const variable &var) const;
    bool operator<(const variable &var) const;

    bool operator>=(const variable &var) const { return !(*this < var); }
    bool operator>(const variable &var) const { return !(*this <= var); }

    double get_number() const;
    const std::string *get_string() const;
    const std::vector<variable *> *get_list() const;
    const std::unordered_map<variable *, variable *, variable_hasher, variable_equal> *get_table() const;
    const function *get_function() const;
    bool get_boolean() const;
};

#endif
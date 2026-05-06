#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "code-units.hpp"

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

class program_module
{
    std::filesystem::path path;
    parsed_code_unit code;
    variable root_table; // must be TABLE type
};

class program
{
    std::unordered_map<std::filesystem::path, program_module> modules;
    std::vector<variable> variables;

    program_module *initialize_module(std::filesystem::path &path);
    variable &add_variable(variable &&variable);

    variable &create_variable(bool value);
    variable &create_variable(double value);
    variable &create_variable(std::string &value);
    variable &create_variable(std::vector<variable *> &value);
    variable &create_variable(std::unordered_map<variable *, variable *, variable_hasher, variable_equal> &value);
    variable &create_function(function *value);
};

struct stack_frame
{
    std::unordered_map<std::string, variable *> local_variables;
};

struct ast_stack_frame
{
    ast_node *node;
};

class execution_context
{
private:
    std::vector<stack_frame> call_stack;
    std::vector<ast_stack_frame> ast_stack;
    program *program;

public:
    void execute_node(ast_node *node);
    void execute_function(variable *function, std::vector<variable *> &arguments);
    void execute_module(program_module *module);
};

#endif
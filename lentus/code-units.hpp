#ifndef CODE_UNITS_HPP
#define CODE_UNITS_HPP

#include <string>
#include <filesystem>
#include <vector>

class program;

class ast_node
{
private:
    int line_number;

public:
    virtual void visit(program &program);
    virtual std::vector<ast_node *> get_children();
};

class parsed_code_unit
{
private:
    ast_node *root_node;
    std::filesystem::path path;

public:
    parsed_code_unit(ast_node *root_node, const std::filesystem::path &path) : root_node(root_node), path(path) {};
};

enum class token_type
{
    IDENTIFIER,
    STRING_CONSTANT,
    NUMBER_CONSTANT,
};

struct token_data
{
    token_type type;
    int line_number;
    union
    {
        std::string *identifier_name;
        std::string *string_constant;
        double *number_constant;
    };
};

class lexed_code_unit
{
private:
    std::vector<token_data> tokens;
    std::filesystem::path path;

public:
    lexed_code_unit(std::vector<token_data> &&tokens, const std::filesystem::path &path) : tokens(tokens), path(path) {};

    void parse();
};

class raw_code_unit
{
private:
    std::string code;
    std::filesystem::path path;

public:
    raw_code_unit(std::string &&code, const std::filesystem::path &path) : code(code), path(path) {};

    lexed_code_unit tokenize();
};

parsed_code_unit compile_source(const std::filesystem::path &path);

#endif

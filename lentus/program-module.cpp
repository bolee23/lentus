#include "program-module.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"

std::optional<program_module *> create_module(const std::filesystem::path &path, std::ostream &error_stream, lentus_table *global_table, bool lp_debug)
{
    // Read file
    std::ifstream input_stream(path);
    std::string file_contents((std::istreambuf_iterator<char>(input_stream)), std::istreambuf_iterator<char>());

    std::optional<std::vector<token>> token_result = tokenize(file_contents, error_stream);

    if (!token_result)
    {
        error_stream << "Lexing failed due to syntax errors" << std::endl;
        return std::nullopt;
    }

    if (lp_debug)
    {
        std::cout << "Tokens:" << std::endl;
        write_tokens(token_result.value(), std::cout);
    }

    parse_result<statement> result_parsing = parse(token_result.value(), file_contents, error_stream);

    if (lp_debug)
    {
        std::cout << "AST:" << std::endl;
        write_ast_node(result_parsing.ast);
    }

    if (result_parsing.error)
    {
        delete result_parsing.ast;
        error_stream << "Parsing failed due to syntax errors" << std::endl;
        return std::nullopt;
    }

    return new program_module(path, result_parsing.ast, global_table);
}

program_module::program_module(program_module &&prgm_module) : path(prgm_module.path), root_statement(prgm_module.root_statement), global_table(prgm_module.global_table)
{
    prgm_module.root_statement = nullptr;
    prgm_module.global_table = nullptr;
}

program_module &program_module::operator=(program_module &&prgm_module)
{
    if (this != &prgm_module)
    {
        // global_table is owned by the program class
        delete root_statement;

        root_statement = prgm_module.root_statement;
        global_table = prgm_module.global_table;

        prgm_module.root_statement = nullptr;
        prgm_module.global_table = nullptr;
    }
    return *this;
}

program_module::~program_module()
{
    // global_table is owned by the program class
    delete root_statement;
}

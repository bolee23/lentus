#include "lentus/lentus.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        return 0;
    }
    else
    {
        const std::filesystem::path source_path(argv[1]);

        if (!std::filesystem::is_regular_file(source_path))
        {
            std::cout << "Error: " << source_path << " is not a file" << std::endl;
            return 1;
        }

        std::ifstream input_stream(source_path);
        std::string file_contents((std::istreambuf_iterator<char>(input_stream)), std::istreambuf_iterator<char>());

        std::optional<std::vector<token>> token_result = tokenize(file_contents, std::cout);

        if (token_result)
        {
            write_tokens(token_result.value(), std::cout);

            std::cout << "Parsing..." << std::endl;

            parse_result result_parsing = parse(token_result.value(), file_contents, std::cout);

            write_expression(result_parsing.ast);

            if (result_parsing.error)
            {
                delete result_parsing.ast;

                std::cout << "Parsing failed due to syntax errors" << std::endl;
                return 1;
            }

            program p = program();
            variable *result = result_parsing.ast->evaluate(p);
            std::cout << "Result: " << result->to_string();

            delete result_parsing.ast;

            return 0;
        }
        else
        {
            return 1;
        }
    }
}

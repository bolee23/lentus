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

        std::optional<std::vector<token>> result = tokenize(file_contents, std::cout);

        if (result)
        {
            write_tokens(result.value(), std::cout);
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

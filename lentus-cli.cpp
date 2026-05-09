#include "lentus/lentus.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
    }
    else
    {
        const std::filesystem::path source_path(argv[1]);

        auto size = std::filesystem::file_size(source_path);
        std::string file_contents(size, '\0');
        std::ifstream in(source_path);
        in.read(&file_contents[0], size);

        raw_code_unit raw_code(std::move(file_contents), source_path);
        std::optional<lexed_code_unit> result = tokenize(raw_code, std::cerr);
    }
    return 0;
}

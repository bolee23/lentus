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

        program prgm;
        std::optional<program_module *> opt = prgm.load_module(source_path, std::cout);

        if (opt)
        {
            std::cout << "Successfully executed";
            return 0;
        }
        else
        {
            std::cout << "Failed to execute";
            return 1;
        }
    }
}

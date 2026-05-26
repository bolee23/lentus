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

        bool debug_mode = false;
        if (argc >= 3)
        {
            if (std::string(argv[2]) == "--debug")
            {
                debug_mode = true;
            }
        }

        program *prgm = new program();
        std::optional<program_module *> opt = prgm->load_module(source_path, std::cout, debug_mode);

        if (opt)
        {
            std::string s;
            std::cout << "a";
            std::cin >> s;
            delete prgm;
            std::cout << "b";
            std::cin >> s;
            return 0;
        }
        else
        {
            std::cout << "Failed to execute";
            return 1;
        }
    }
}

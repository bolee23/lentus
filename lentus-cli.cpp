#include "lentus/lentus.hpp"

using namespace lentus;
#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Lentus v0.0.1\nUsage: lentus <path> [--debug]" << std::endl;
        return 0;
    }
    else
    {
        const std::filesystem::path source_path(argv[1]);

        bool debug_mode = false;
        
        // Extra arguments
        if (argc >= 3)
        {
            for (int i = 2; i < argc; i++)
            {
                if (std::string(argv[i]) == "--debug")
                {
                    debug_mode = true;
                }
            }
        }

        program prgm;
        std::optional<program_module *> opt = prgm.load_module(source_path, std::cout, debug_mode);

        if (opt)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

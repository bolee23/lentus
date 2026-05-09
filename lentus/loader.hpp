#ifndef LENTUS_LOADER_HPP
#define LENTUS_LOADER_HPP

#include <string>
#include <filesystem>

struct raw_code_unit
{
    std::string source;
    std::filesystem::path path;
};

#endif

#pragma once
#include "../CLI/CLI11.hpp"

enum Flag {
    DEFAULT,
    SHELL,
    BASENAME
};

const std::map<Flag, std::string> FLAG_NAME{
    {DEFAULT, "--default"},
    {SHELL, "--shell"},
    {BASENAME, "--basename"},
};

struct AppCli {
    CLI::App cli_app{"MacFuzzyFinder"};
    bool basename = false;

    AppCli() = default;
    int run(int argc, char* argv[]);
};

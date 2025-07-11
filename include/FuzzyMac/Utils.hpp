#pragma once

#include <string>
#include <variant>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;
void expandPaths(std::vector<std::string>& paths);

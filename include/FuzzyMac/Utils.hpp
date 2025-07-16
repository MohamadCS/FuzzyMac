#pragma once

#include <string>
#include <variant>
#include <vector>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;
void expandPaths(std::vector<std::string>& paths);
std::optional<double> evalMathExp(const std::string& exp);

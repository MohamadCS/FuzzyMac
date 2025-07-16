#pragma once

#include <string>
#include <variant>
#include <vector>
#include <filesystem>
#include <optional>
#include <QStringList>
namespace fs = std::filesystem;

void expandPaths(QStringList& paths);
std::optional<double> evalMathExp(const std::string& exp);

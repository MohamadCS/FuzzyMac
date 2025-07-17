#pragma once

#include <QStringList>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>
namespace fs = std::filesystem;

void expandPaths(QStringList& paths);
std::optional<double> evalMathExp(const std::string& exp);
QString convertToReadableFileSize(qint64 size);

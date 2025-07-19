#pragma once

#include <QStringList>
#include <optional>
#include <string>

void expandPaths(QStringList& paths);
std::optional<double> evalMathExp(const std::string& exp);
QString convertToReadableFileSize(qint64 size);

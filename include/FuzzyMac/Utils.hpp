#pragma once

#include <QDateTime>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <optional>
#include <string>

void expandPaths(QStringList& paths);
std::optional<double> evalMathExp(const std::string& exp);
QString cutPathPrefix(const QString& path, int char_count);
QString convertToReadableFileSize(qint64 size);
QStringList fromQList(const QList<std::string>& vec);
void loggingHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

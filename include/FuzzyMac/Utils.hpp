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
void loadDirs(const QString& d, QStringList& paths, bool rec = true);

template <typename T, typename Container>
QList<T> getKeys(const Container& container) {
    QList<T> result;
    result.reserve(container.size());

    for (auto& [key, value] : container) {
        result.push_back(key);
    }

    return result;
}

template <typename T, typename Container>
QList<T> getValues(const Container& container) {
    QList<T> result;
    result.reserve(container.size());

    for (auto& [key, value] : container) {
        result.push_back(value);
    }

    return result;
}

#pragma once

#include <QString>
#include <QStringList>
#include <functional>

int fuzzyScore(const QString& cand, const QString& query);
QStringList filter(const QString& query_, const QStringList& entries, std::vector<int>* idx_vec = nullptr,
                   std::optional<std::function<QString(const QString&)>> format = std::nullopt);

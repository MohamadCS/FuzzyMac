#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include <QString>

int fuzzyScore(const QString& cand, const QString& query);
QStringList filter(MainWindow* win, const QString& query_, const QStringList& entries);

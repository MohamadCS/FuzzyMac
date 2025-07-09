#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static void customSearch(const QString& query_, QListWidget* results_list, const std::vector<std::string>& entries,
                         std::vector<int>& results_indices);

/***************************/

void AppModeHandler::enterHandler(QListWidget* results_list) {
    int i = results_list->currentRow();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(apps[results_indices[i]]);
    qDebug() << QString::fromStdString(apps[results_indices[i]]);
    process->start("open", args);
    deactivateApp();
    results_list->window()->hide();
}

void AppModeHandler::fillData(QListWidget* results_list) {
    const fs::path apps_path = "/Applications";
    if (!fs::exists(apps_path)) {
        QMessageBox::critical(nullptr, "Error", "Wrong Path");
    }

    int i = 0;
    for (const auto& entry : fs::directory_iterator(apps_path)) {
        if (entry.path().extension() == ".app") {
            apps.push_back(entry.path().string());
            results_list->addItem(QString::fromStdString(entry.path().filename().string()));
            results_indices.push_back(i);
            ++i;
        }
    }
}

void AppModeHandler::updateResultsList(const QString& query_, QListWidget* results_list) {
    customSearch(query_, results_list, apps, results_indices);
}

/***************************/

void CLIModeHandler::enterHandler(QListWidget* results_list) {
    int i = results_list->currentRow();
    std::cout << entries[results_indices[i]];
    exit(0);
}

void CLIModeHandler::fillData(QListWidget* results_list) {
    qDebug() << "CLI TOOL";
    entries = {};
    std::string line;

    int i = 0;
    while (std::getline(std::cin, line)) {
        entries.push_back(line);
        results_list->addItem(QString::fromStdString(line));
        results_indices.push_back(i);
        ++i;
    }
}

void CLIModeHandler::updateResultsList(const QString& query_, QListWidget* results_list) {
    customSearch(query_, results_list, entries, results_indices);
}

/***************************/

void FileModeHandler::enterHandler(QListWidget* results_list) {
    int i = results_list->currentRow();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(abs_results[i]);
    process->start("open", args);
    deactivateApp();
    results_list->window()->hide();
}

void FileModeHandler::fillData(QListWidget* results_list) {
    // should not be the first
}

void FileModeHandler::updateResultsList(const QString& query_, QListWidget* results_list) {
    abs_results.clear();
    if (query_.size() <= 1) {
        return;
    }

    auto query = query_.right(query_.size() - 1);

    std::vector<std::string> paths{fs::absolute(std::format("{}/Library/Mobile Documents/", std::getenv("HOME")))};
    auto files = spotlightSearch(std::format("kMDItemDisplayName == '{}*'c", query.toStdString()), paths);

    for (const auto& file : files) {
        results_list->addItem(QString::fromStdString(fs::path(file).filename().string()));
    }

    abs_results = files;
}

/***************************/

static void customSearch(const QString& query_, QListWidget* results_list, const std::vector<std::string>& entries,
                         std::vector<int>& results_indices) {

    results_indices.clear();

    std::string query = query_.toLower().toStdString();

    std::vector<std::pair<int, int>> scores_per_idx{};

    // calculate the score for each entry.
    for (int i = 0; i < entries.size(); ++i) {
        int score = fuzzyScore(fs::path(entries[i]).filename().string(), query);

        if (score >= 0) {
            scores_per_idx.push_back({score, i});
        }
    }

    std::sort(scores_per_idx.begin(), scores_per_idx.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    });

    for (int i = 0; i < scores_per_idx.size(); ++i) {
        int idx = scores_per_idx[i].second;
        results_list->addItem(QString::fromStdString(fs::path(entries[idx]).filename().string()));
        results_indices.push_back(idx);
    }

    if (scores_per_idx.size() >= 1) {
        results_list->setCurrentRow(0);
    }
}

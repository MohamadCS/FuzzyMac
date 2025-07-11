#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"

#include <filesystem>
#include <iostream>
#include <wordexp.h>

namespace fs = std::filesystem;

static QStringList customSearch(const QString& query_, QListWidget* results_list,
                                const std::vector<std::string>& entries, std::vector<int>& results_indices);

/***************************/

std::vector<std::string> spotlightQuery(const std::vector<std::string>& dirs, const std::string& query) {
    std::vector<std::string> results;
    const int MAX_RESULTS = 30;
    int resultCount = 0;

    for (const std::string& dir : dirs) {
        if (resultCount >= MAX_RESULTS)
            break;

        QStringList args;
        args << "-onlyin" << QString::fromStdString(dir) << QString::fromStdString(query);

        QProcess process;
        process.start("/usr/bin/mdfind", args);
        if (!process.waitForFinished(3000)) { // Wait max 3 seconds
            continue;                         // Skip on timeout
        }

        QByteArray output = process.readAllStandardOutput();
        QList<QByteArray> lines = output.split('\n');

        for (const QByteArray& line : lines) {
            if (!line.isEmpty()) {
                results.emplace_back(line.constData());
                resultCount++;
                if (resultCount >= MAX_RESULTS)
                    break;
            }
        }
    }

    return results;
}

static void expand(std::vector<std::string>& paths) {
    for (auto& input : paths) {
        wordexp_t p;
        fs::path expanded_path;
        std::string quoted = "\"" + input + "\"";
        if (wordexp(quoted.c_str(), &p, 0) == 0) {
            if (p.we_wordc > 0) {
                input = p.we_wordv[0]; // Take the first expanded word
            }
            wordfree(&p);
        } else {
            std::cerr << "wordexp failed!" << std::endl;
        }
    }
}

void AppModeHandler::handleQuickLock(QListWidget* results_list) {
}

void AppModeHandler::enterHandler(QListWidget* results_list) {
    int i = results_list->currentRow();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(apps[results_indices[i]]);
    process->start("open", args);
    deactivateApp();
    results_list->window()->hide();
}

void AppModeHandler::fillData(QListWidget* results_list) {

    auto paths = get_array<std::string>(*config, {"mode", "apps", "dirs"});
    expand(paths);

    const std::vector<fs::path> special_apps{
        "/System/Library/CoreServices/Finder.app",
    };

    int i = 0;
    for (const auto& path : paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".app") {
                apps.push_back(entry.path().string());
                results_list->addItem(QString::fromStdString(entry.path().filename().string()));
                results_indices.push_back(i);
                ++i;
            }
        }
    }
    paths = get_array<std::string>(*config, {"mode", "apps", "apps"});
    expand(paths);

    for (const auto& path : paths) {
        apps.push_back(path);
        results_list->addItem(QString::fromStdString(fs::path(path).filename().string()));
        results_indices.push_back(i);
        ++i;
    }
}

QStringList AppModeHandler::getResults(const QString& query_, QListWidget* results_list) {
    return customSearch(query_, results_list, apps, results_indices);
}

/***************************/

void CLIModeHandler::enterHandler(QListWidget* results_list) {
    int i = results_list->currentRow();
    std::cout << entries[results_indices[i]];
    exit(0);
}

void CLIModeHandler::fillData(QListWidget* results_list) {
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

QStringList CLIModeHandler::getResults(const QString& query_, QListWidget* results_list) {
    return customSearch(query_, results_list, entries, results_indices);
}

void CLIModeHandler::handleQuickLock(QListWidget* results_list) {
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

QStringList FileModeHandler::getResults(const QString& query_, QListWidget* results_list) {
    abs_results.clear();
    if (query_.size() <= 1) {
        return {};
    }
    if (paths.empty()) {
        paths = get_array<std::string>(*config, {"mode", "files", "dirs"});
        expand(paths);
    }

    auto query = query_.right(query_.size() - 1);

    auto files = spotlightSearch(paths,std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));

    QStringList res{};
    for (const auto& file : files) {
        res.push_back(QString::fromStdString(fs::path(file).filename().string()));
    }

    abs_results = files;
    return res;
}

void FileModeHandler::handleQuickLock(QListWidget* results_list) {
    if (results_list->count() == 0) {
        return;
    }

    quickLock(abs_results[results_list->currentRow()]);
}

/***************************/

static QStringList customSearch(const QString& query_, QListWidget* results_list,
                                const std::vector<std::string>& entries, std::vector<int>& results_indices) {

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

    QStringList res;
    for (int i = 0; i < scores_per_idx.size(); ++i) {
        int idx = scores_per_idx[i].second;
        res.push_back(QString::fromStdString(fs::path(entries[idx]).filename().string()));
        results_indices.push_back(idx);
    }

    return res;
}

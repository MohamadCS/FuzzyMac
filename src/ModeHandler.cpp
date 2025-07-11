#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"

#include <filesystem>
#include <iostream>
#include <wordexp.h>

#include <QFileIconProvider>
namespace fs = std::filesystem;

static std::vector<QListWidgetItem*> customSearch(MainWindow* win, const QString& query_,
                                                  const std::vector<std::string>& entries,
                                                  std::vector<int>& results_indices, bool show_icons);

/***************************/

// std::vector<std::string> spotlightQuery(const std::vector<std::string>& dirs, const std::string& query) {
//     std::vector<std::string> results;
//     const int MAX_RESULTS = 30;
//     int resultCount = 0;
//
//     for (const std::string& dir : dirs) {
//         if (resultCount >= MAX_RESULTS)
//             break;
//
//         std::vector<QListWidgetItem> args;
//         args << "-onlyin" << QString::fromStdString(dir) << QString::fromStdString(query);
//
//         QProcess process;
//         process.start("/usr/bin/mdfind", args);
//         if (!process.waitForFinished(3000)) { // Wait max 3 seconds
//             continue;                         // Skip on timeout
//         }
//
//         QByteArray output = process.readAllStandardOutput();
//         QList<QByteArray> lines = output.split('\n');
//
//         for (const QByteArray& line : lines) {
//             if (!line.isEmpty()) {
//                 results.emplace_back(line.constData());
//                 resultCount++;
//                 if (resultCount >= MAX_RESULTS)
//                     break;
//             }
//         }
//     }
//
//     return results;
// }

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

void AppModeHandler::handleQuickLock() {
}

void AppModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(apps[results_indices[i]]);
    process->start("open", args);
    deactivateApp();
    win->sleep();
}

void AppModeHandler::fillData() {

    auto paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "dirs"});
    expand(paths);

    const std::vector<fs::path> special_apps{
        "/System/Library/CoreServices/Finder.app",
    };

    int i = 0;
    for (const auto& path : paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".app") {
                apps.push_back(entry.path().string());
                win->addToResultList(entry.path().filename().string(), entry.path());
                results_indices.push_back(i);
                ++i;
            }
        }
    }
    paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "apps"});
    expand(paths);

    for (const auto& path : paths) {
        apps.push_back(path);

        win->addToResultList(path, fs::path(path));
        results_indices.push_back(i);
        ++i;
    }
}

std::vector<QListWidgetItem*> AppModeHandler::getResults(const QString& query) {
    return customSearch(win, query, apps, results_indices, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"}));
}

/***************************/

void CLIModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    std::cout << entries[results_indices[i]];
    exit(0);
}

void CLIModeHandler::fillData() {
    entries = {};
    std::string line;

    int i = 0;
    while (std::getline(std::cin, line)) {
        entries.push_back(line);
        win->addToResultList(line);
        results_indices.push_back(i);
        ++i;
    }
}

std::vector<QListWidgetItem*> CLIModeHandler::getResults(const QString& query_) {
    return customSearch(win, query_, entries, results_indices, false);
}

void CLIModeHandler::handleQuickLock() {
}

/***************************/

void FileModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(abs_results[i]);
    process->start("open", args);
    deactivateApp();
    win->sleep();
}

void FileModeHandler::fillData() {
    // should not be the first
}

std::vector<QListWidgetItem*> FileModeHandler::getResults(const QString& query_) {
    abs_results.clear();
    if (query_.size() <= 1) {
        return {};
    }
    if (paths.empty()) {
        paths = get_array<std::string>(win->getConfig(), {"mode", "files", "dirs"});
        expand(paths);
    }

    auto query = query_.right(query_.size() - 1);

    auto files = spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));

    std::vector<QListWidgetItem*> res{};
    for (const auto& file : files) {
        std::optional<fs::path> path; 
        if(get<bool>(win->getConfig(), {"mode", "files", "show_icons"})) {
            path = fs::path(file);
        }

        res.push_back(win->createListItem(fs::path(file).filename().string(),path));
    }

    abs_results = files;
    return res;
}

void FileModeHandler::handleQuickLock() {
    if (win->resultsNum() == 0) {
        return;
    }

    quickLock(abs_results[win->getCurrentResultIdx()]);
}

/***************************/

static std::vector<QListWidgetItem*> customSearch(MainWindow* win, const QString& query_,
                                                  const std::vector<std::string>& entries,
                                                  std::vector<int>& results_indices, bool show_icons) {

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

    std::vector<QListWidgetItem*> res{};
    res.reserve(scores_per_idx.size());

    for (int i = 0; i < scores_per_idx.size(); ++i) {
        int idx = scores_per_idx[i].second;
        const auto& entry = entries[idx];
        if (show_icons) {
            res.push_back(win->createListItem(fs::path(entry).filename().string(), fs::path(entry)));
        } else {
            res.push_back(win->createListItem(fs::path(entry).filename().string()));
        }
        results_indices.push_back(idx);
    }
    return res;
}

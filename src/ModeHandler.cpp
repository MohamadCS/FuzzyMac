#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <filesystem>
#include <iostream>
#include <optional>
#include <wordexp.h>

#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>

namespace fs = std::filesystem;

static std::vector<QListWidgetItem*> customSearch(MainWindow* win, const QString& query_,
                                                  const std::vector<std::string>& entries,
                                                  std::vector<int>& results_indices, bool show_icons);

/***************************/

ModeHandler::ModeHandler(MainWindow* win)
    : win(win) {
}
void ModeHandler::handleCopy() {
}
void ModeHandler::handlePathCopy() {
}

void ModeHandler::handleDragAndDrop(QDrag* drag) {
}

/***************************/

AppModeHandler::AppModeHandler(MainWindow* win)
    : ModeHandler(win),
      app_watcher(new QFileSystemWatcher(nullptr)) {
    QObject::connect(app_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win](const QString&) {
        load();
        win->refreshResults();
    });
}

AppModeHandler::~AppModeHandler() {
    delete app_watcher;
};

void AppModeHandler::handleQuickLock() {
}

void AppModeHandler::load() {
    auto paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "dirs"});
    expandPaths(paths);

    apps.clear();
    results_indices.clear();

    int i = 0;
    for (const auto& path : paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".app") {
                apps.push_back(entry.path().string());
                results_indices.push_back(i);
                ++i;
            }
        }
    }

    app_watcher->removePaths(app_watcher->directories());
    QStringList paths_list{};
    for (const auto& path : paths) {
        paths_list.push_back(QString::fromStdString(path));
    }

    app_watcher->addPaths(paths_list);

    paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "apps"});
    expandPaths(paths);

    for (const auto& path : paths) {
        apps.push_back(path);
        win->addToResultList(path, fs::path(path));
        results_indices.push_back(i);
        ++i;
    }
}

void AppModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(apps[results_indices[i]]);
    process->start("open", args);
    win->sleep();
}

std::vector<QListWidgetItem*> AppModeHandler::getResults(const QString& query) {
    return customSearch(win, query, apps, results_indices, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"}));
}

/***************************/

CLIModeHandler::CLIModeHandler(MainWindow* win)
    : ModeHandler(win) {
}

void CLIModeHandler::load() {
    if (loaded) {
        return;
    }

    entries = {};
    std::string line;

    int i = 0;
    while (std::getline(std::cin, line)) {
        entries.push_back(line);
        results_indices.push_back(i);
        ++i;
    }
    loaded = true;
}

void CLIModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    std::cout << entries[results_indices[i]];
    exit(0);
}

std::vector<QListWidgetItem*> CLIModeHandler::getResults(const QString& query_) {
    return customSearch(win, query_, entries, results_indices, false);
}

void CLIModeHandler::handleQuickLock() {
}

/***************************/

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {
    load();
}
void FileModeHandler::load() {
    paths = get_array<std::string>(win->getConfig(), {"mode", "files", "dirs"});
    expandPaths(paths);
}

void FileModeHandler::enterHandler() {
    int i = win->getCurrentResultIdx();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(abs_results[i]);
    process->start("open", args);
    win->sleep();
}

std::vector<QListWidgetItem*> FileModeHandler::getResults(const QString& query_) {
    abs_results.clear();
    if (query_.size() <= 1) {
        return {};
    }

    auto query = query_.right(query_.size() - 1);

    auto files = spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));

    std::vector<QListWidgetItem*> res{};
    for (const auto& file : files) {
        std::optional<fs::path> path;
        if (get<bool>(win->getConfig(), {"mode", "files", "show_icons"})) {
            path = fs::path(file);
        }

        res.push_back(win->createListItem(fs::path(file).filename().string(), path));
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

void FileModeHandler::handleCopy() {
    QMimeData* mime_data = new QMimeData();

    // Create a list with a single file URL
    QString file_path = QString::fromStdString(abs_results[win->getCurrentResultIdx()]);
    QList<QUrl> urls;
    qDebug() << file_path;
    urls.append(QUrl::fromLocalFile(std::move(file_path)));

    mime_data->setUrls(urls);

    // Set the clipboard contents
    QGuiApplication::clipboard()->setMimeData(mime_data);
}

void FileModeHandler::handleDragAndDrop(QDrag* drag) {
    auto path = abs_results[win->getCurrentResultIdx()];
    QIcon icon = win->getFileIcon(path);
    QMimeData* mime_data = new QMimeData;
    QPixmap pixmap = icon.pixmap(64, 64); // Icon size for the drag
    mime_data->setUrls({QUrl::fromLocalFile(QString::fromStdString(path))});
    drag->setMimeData(mime_data);
    drag->setPixmap(pixmap);
    drag->exec(Qt::CopyAction);
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

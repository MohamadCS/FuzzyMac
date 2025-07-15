#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

namespace fs = std::filesystem;

static std::vector<std::string> customSearch(MainWindow* win, const QString& query_,
                                             const std::vector<std::string>& entries, bool show_icons);

/***************************/

ModeHandler::ModeHandler(MainWindow* win)
    : win(win) {
}

std::string ModeHandler::handleModeText() {
    return "empty";
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

std::string AppModeHandler::handleModeText() {
    return "";
}
void AppModeHandler::handleQuickLock() {
}

void AppModeHandler::load() {
    auto paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "dirs"});
    expandPaths(paths);

    apps.clear();
    widgets.clear();

    // reload apps in defined app dirs
    for (const auto& path : paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".app") {
                apps.push_back(entry.path().string());
            }
        }
    }

    // watch new app dirs
    if (app_watcher->directories().size() > 0) {
        app_watcher->removePaths(app_watcher->directories());
    }

    QStringList paths_list{};
    for (const auto& path : paths) {
        paths_list.push_back(QString::fromStdString(path));
    }
    app_watcher->addPaths(paths_list);

    paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "apps"});
    expandPaths(paths);

    // add special apps(specific apps instead of dirs)
    for (const auto& path : paths) {
        apps.push_back(path);
        widgets.push_back(new FileWidget(win, path, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"})));
    }
    win->processResults(widgets);
}

void AppModeHandler::enterHandler() {

    if (win->getResultsNum() == 0) {
        return;
    }

    int i = win->getCurrentResultIdx();
    widgets[i]->enterHandler();
}

void AppModeHandler::invokeQuery(const QString& query) {
    qDebug() << "Invoking App query";
    widgets.clear();

    auto exp = evalMathExp(query.toStdString());

    if (query.isEmpty()) {
        math_mode = false;
    }

    if (math_mode || exp.has_value()) {
        math_mode = true;
        auto* calc_widget = new CalculatorWidget(win);
        if (exp.has_value()) {
            calc_widget->answer_label->setText(std::format("{}", exp.value()).c_str());
        }
        widgets.push_back(calc_widget);
    }

    auto search_results = customSearch(win, query, apps, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"}));

    for (const auto& path : search_results) {
        widgets.push_back(new FileWidget(win, path, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"})));
    }

    qDebug() << "Finished invoking";
    win->processResults(widgets);
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

    while (std::getline(std::cin, line)) {
        entries.push_back(line);
        widgets.push_back(new TextWidget(win, line));
    }
    loaded = true;
}

void CLIModeHandler::enterHandler() {
    if (win->getResultsNum() == 0) {
        return;
    }

    int i = win->getCurrentResultIdx();
    std::cout << widgets[i]->getValue();
    exit(0);
}

void CLIModeHandler::invokeQuery(const QString& query_) {
    auto results = customSearch(win, query_, entries, false);

    ResultsVec res{};
    res.reserve(entries.size());

    for (auto& entry : entries) {
        res.push_back(new TextWidget(win, entry));
    }

    win->processResults(res);
}

void CLIModeHandler::handleQuickLock() {
}

std::string CLIModeHandler::handleModeText() {
    return std::format("Results");
}

/***************************/

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {
    watcher = new QFutureWatcher<std::vector<std::string>>();

    QObject::connect(watcher, &QFutureWatcher<std::vector<std::string>>::finished, [this, win]() {
        if (win->getQuery().empty()) {
            return;
        }

        widgets.clear();
        auto results = watcher->result();
        for (const auto& file : results) {
            std::optional<fs::path> path;
            if (get<bool>(win->getConfig(), {"mode", "files", "show_icons"})) {
                path = fs::path(file);
            }

            widgets.push_back(
                new FileWidget(win, path->string(), get<bool>(win->getConfig(), {"mode", "files", "show_icons"})));
        }

        win->processResults(widgets);
    });

    load();
}
void FileModeHandler::load() {
    paths = get_array<std::string>(win->getConfig(), {"mode", "files", "dirs"});
    expandPaths(paths);
}

void FileModeHandler::enterHandler() {

    if (win->getResultsNum() == 0) {
        return;
    }

    int i = win->getCurrentResultIdx();
    widgets[i]->enterHandler();
}

void FileModeHandler::invokeQuery(const QString& query_) {
    qDebug() << "Invoking File query";

    auto query = query_.trimmed();

    if (query.isEmpty()) {
        return;
    }

    if (watcher->isRunning()) {
        watcher->cancel();
    }

    widgets.clear();

    auto future = QtConcurrent::run([this, query]() -> std::vector<std::string> {
        return spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));
    });

    watcher->setFuture(future);

    qDebug() << "Finished Invoking";
}

void FileModeHandler::handleQuickLock() {
    if (win->getResultsNum() == 0) {
        return;
    }

    quickLock(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
}

std::string FileModeHandler::handleModeText() {
    return std::format("Files");
}

void FileModeHandler::handleCopy() {
    QMimeData* mime_data = new QMimeData();

    // Create a list with a single file URL
    QString file_path =
        QString::fromStdString(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
    QList<QUrl> urls;
    urls.append(QUrl::fromLocalFile(std::move(file_path)));

    mime_data->setUrls(urls);

    // Set the clipboard contents
    QGuiApplication::clipboard()->setMimeData(mime_data);
}

void FileModeHandler::handleDragAndDrop(QDrag* drag) {
    auto path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();
    QIcon icon = win->getFileIcon(path);
    QMimeData* mime_data = new QMimeData;
    QPixmap pixmap = icon.pixmap(64, 64); // Icon size for the drag
    mime_data->setUrls({QUrl::fromLocalFile(QString::fromStdString(path))});
    drag->setMimeData(mime_data);
    drag->setPixmap(pixmap);
    drag->exec(Qt::CopyAction);
}

/***************************/

static std::vector<std::string> customSearch(MainWindow* win, const QString& query_,
                                             const std::vector<std::string>& entries, bool show_icons) {

    std::string query = query_.toLower().toStdString();

    std::vector<std::pair<int, int>> scores_per_idx{};

    // calculate the score for each entry.
    for (int i = 0; i < entries.size(); ++i) {
        int score = fuzzyScore(fs::path(entries[i]).filename().string(), query);

        if (score >= 0) {
            scores_per_idx.push_back({score, i});
        }
    }

    // sort in decreasing order according to score
    std::sort(scores_per_idx.begin(), scores_per_idx.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    });

    std::vector<std::string> res{};
    res.reserve(scores_per_idx.size());

    // fill resultslist based on the scores
    for (int i = 0; i < scores_per_idx.size(); ++i) {
        int idx = scores_per_idx[i].second;
        res.push_back(entries[idx]);
    }

    return res;
}

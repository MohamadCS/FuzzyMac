#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <utility>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

namespace fs = std::filesystem;

static std::vector<std::string> customSearch(MainWindow* win, const QString& query_,
                                             const std::vector<std::string>& entries);

/***************************/

ModeHandler::ModeHandler(MainWindow* win)
    : win(win),
      main_widget(new QWidget(nullptr)) {
}

void ModeHandler::freeWidgets() {
}

std::string ModeHandler::handleModeText() {
    return "empty";
}

std::optional<QIcon> ModeHandler::getIcon() const {
    return {};
}
void ModeHandler::handleCopy() {
}

void ModeHandler::handlePathCopy() {
}

void ModeHandler::handleDragAndDrop(QDrag* drag) const {
}

std::string ModeHandler::getPrefix() const {
    return "";
}

/***************************/

AppModeHandler::AppModeHandler(MainWindow* win)
    : ModeHandler(win),
      app_watcher(new QFileSystemWatcher(win)) {
    modes.insert_or_assign("Search files", Mode::FILE);
    QObject::connect(app_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win](const QString& path) {
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
void AppModeHandler::handleQuickLook() {
}

void AppModeHandler::load() {
    auto paths = get_array<std::string>(win->getConfig(), {"mode", "apps", "dirs"});
    expandPaths(paths);

    apps.clear();
    freeWidgets();

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
        widgets.push_back(
            new FileWidget(win, main_widget, path, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"})));
    }

    std::vector<std::string> keys;
    keys.reserve(modes.size());
    std::transform(modes.begin(), modes.end(), std::back_inserter(keys), [](const auto& pair) { return pair.first; });

    for (const auto& key : keys) {
        widgets.push_back(
            new ModeWidget(win, main_widget, key, modes[key], win->getModeHandler(modes[key])->getIcon()));
    }

    win->processResults(widgets);
}

void AppModeHandler::enterHandler() {

    if (win->getResultsNum() == 0) {
        return;
    }

    int i = std::max(win->getCurrentResultIdx(),0);
    widgets[i]->enterHandler();
}

void AppModeHandler::freeWidgets() {
    delete main_widget;
    widgets.clear();
    main_widget = new QWidget();
}

void AppModeHandler::invokeQuery(const QString& query) {
    qDebug() << "Invoking App query";

    // clear all widgets from memory
    freeWidgets();

    auto exp = evalMathExp(query.toStdString());

    if (query.isEmpty()) {
        math_mode = false;
    }

    if (math_mode || exp.has_value()) {
        math_mode = true;
        auto* calc_widget = new CalculatorWidget(win, main_widget);
        if (exp.has_value()) {
            calc_widget->answer_label->setText(std::format("{}", exp.value()).c_str());
        }
        widgets.push_back(calc_widget);
    }

    auto search_results = customSearch(win, query, apps);

    for (const auto& path : search_results) {
        widgets.push_back(
            new FileWidget(win, main_widget, path, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"})));
    }

    std::vector<std::string> keys;
    keys.reserve(modes.size());
    std::transform(modes.begin(), modes.end(), std::back_inserter(keys), [](const auto& pair) { return pair.first; });
    auto modes_results = customSearch(win, query, keys);

    for (const auto& key : modes_results) {
        widgets.push_back(
            new ModeWidget(win, main_widget, key, modes[key], win->getModeHandler(modes[key])->getIcon()));
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
        widgets.push_back(new TextWidget(win, main_widget, line));
    }
    loaded = true;
}

void CLIModeHandler::enterHandler() {
    if (win->getResultsNum() == 0) {
        return;
    }

    int i = std::max(win->getCurrentResultIdx(),0);
    std::cout << widgets[i]->getValue();
    exit(0);
}

void CLIModeHandler::invokeQuery(const QString& query_) {
    auto results = customSearch(win, query_, entries);

    ResultsVec res{};
    res.reserve(entries.size());

    for (auto& entry : entries) {
        res.push_back(new TextWidget(win, main_widget, entry));
    }

    win->processResults(res);
}

void CLIModeHandler::handleQuickLook() {
}

std::string CLIModeHandler::handleModeText() {
    return std::format("Results");
}

/***************************/

void FileModeHandler::freeWidgets() {
    mutex.lock();

    delete main_widget;
    widgets.clear();

    main_widget = new QWidget(nullptr);

    mutex.unlock();
}

static void loadDirs(const std::string& dir, std::vector<std::string>& paths) {
    for (auto entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            paths.push_back(entry.path());
        } else if (entry.is_directory()) {
            paths.push_back(entry.path());
            loadDirs(entry.path(), paths);
        }
    }
}

void FileModeHandler::load() {
    paths = get_array<std::string>(win->getConfig(), {"mode", "files", "dirs"});
    expandPaths(paths);

    icon = win->createIcon(":/res/icons/search_files_icon.svg",
                           QColor(get<std::string>(win->getConfig(), {"colors", "results_list", "text"}).c_str()));

    entries.clear();
    for (auto& dir : paths) {
        loadDirs(dir, entries);
    }

    QStringList paths_list{};
    for (const auto& path : entries) {
        if (fs::is_directory(path)) {
            paths_list.push_back(path.c_str());
        }
    }

    if (dir_watcher->directories().size() > 0) {
        dir_watcher->removePaths(dir_watcher->directories());
        dir_watcher->removePaths(dir_watcher->files());
    }

    dir_watcher->addPaths(paths_list);
}

std::optional<QIcon> FileModeHandler::getIcon() const {
    return icon;
}

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {
    future_watcher = new QFutureWatcher<std::vector<std::string>>(win);
    dir_watcher = new QFileSystemWatcher(win);

    QObject::connect(dir_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win](const QString&) {
        load();
        win->refreshResults();
    });

    QObject::connect(future_watcher, &QFutureWatcher<std::vector<std::string>>::finished, [this, win]() {
        if (win->getQuery().empty()) {
            return;
        }

        freeWidgets();

        auto results = future_watcher->result();
        for (const auto& file : results) {
            if (widgets.size() >= 25) {
                break;
            }

            widgets.push_back(
                new FileWidget(win, main_widget, file, get<bool>(win->getConfig(), {"mode", "files", "show_icons"})));
        }

        win->processResults(widgets);
    });

    load();
}

FileModeHandler::~FileModeHandler() {
    delete dir_watcher;
}

void FileModeHandler::invokeQuery(const QString& query_) {
    qDebug() << "Invoking File query";

    auto query = query_.trimmed();

    if (query.isEmpty()) {
        win->processResults({});
        return;
    }

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> std::vector<std::string> {
        // return spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));
        return customSearch(win, query, entries);
    });

    future_watcher->setFuture(future);

    qDebug() << "Finished Invoking";
}

void FileModeHandler::handleQuickLook() {
    if (win->getResultsNum() == 0) {
        return;
    }

    // quickLook(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
}

void FileModeHandler::enterHandler() {

    if (win->getResultsNum() == 0) {
        return;
    }

    int i = std::max(win->getCurrentResultIdx(), 0 );
    widgets[i]->enterHandler();
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

void FileModeHandler::handleDragAndDrop(QDrag* drag) const {
    auto path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();
    QIcon icon = win->getFileIcon(path);
    QMimeData* mime_data = new QMimeData;
    QPixmap pixmap = icon.pixmap(64, 64); // Icon size for the drag
    mime_data->setUrls({QUrl::fromLocalFile(QString::fromStdString(path))});
    drag->setMimeData(mime_data);
    drag->setPixmap(pixmap);
    drag->exec(Qt::CopyAction);
}

std::string FileModeHandler::getPrefix() const {
    return " ";
}

/***************************/

static std::vector<std::string> customSearch(MainWindow* win, const QString& query_,
                                             const std::vector<std::string>& entries) {

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

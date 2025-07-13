#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
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

static ResultsVec customSearch(MainWindow* win, const QString& query_, const std::vector<std::string>& entries,
                               std::vector<int>& results_indices, bool show_icons);

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
    calc_widget = new CalculatorWidget(win);
}

bool AppModeHandler::needsAsyncFetch() {
    return false;
};

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
    results_indices.clear();

    int i = 0;
    // reload apps in defined app dirs
    for (const auto& path : paths) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".app") {
                apps.push_back(entry.path().string());
                results_indices.push_back(i);
                ++i;
            }
        }
    }

    // watch new app dirs
    app_watcher->removePaths(app_watcher->directories());
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
        win->addToResultList(path, fs::path(path));
        results_indices.push_back(i);
        ++i;
    }
}

CalculatorWidget::CalculatorWidget(MainWindow* win)
    : QWidget(),
      win(win) {
    title_label = new QLabel(this);
    answer_label = new QLabel(this);
    const auto& config = win->getConfig();
    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    title_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 20px;
        }
    )")
                                   .arg(get<std::string>(config, {"colors", "mode_label", "text"}))
                                   .arg(get<std::string>(config, {"font"})));

    answer_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 30px;
        }
    )")
                                    .arg(get<std::string>(config, {"colors", "mode_label", "text"}))
                                    .arg(get<std::string>(config, {"font"})));

    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    answer_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    title_label->setText("Result");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(title_label);
    layout->addWidget(answer_label);
}

void AppModeHandler::enterHandler() {

    if (win->getResultsNum() == 0) {
        return;
    }

    if (win->isWidgetCurrentSelection(calc_widget)) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(calc_widget->answer_label->text());
        win->sleep();
        return;
    }

    int i = win->getCurrentResultIdx();
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(apps[results_indices[i]]);
    process->start("open", args);
    win->sleep();
}

ResultsVec AppModeHandler::fetch(const QString& query) {
    ResultsVec res{};

    if (math_mode || evalMathExp(query.toStdString())) {
        math_mode = true;
        res.push_back(calc_widget);
    }

    auto search_results =
        customSearch(win, query, apps, results_indices, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"}));

    res.insert(res.end(), search_results.begin(), search_results.end());

    return res;
}

void AppModeHandler::beforeFetch() {
    // prepare the calculator widget
    delete calc_widget;
    calc_widget = new CalculatorWidget(win);
    if (win->getQuery().empty()) {
        math_mode = false;
    }
}

void AppModeHandler::afterFetch() {
    if (auto exp = evalMathExp(win->getQuery())) {
        calc_widget->answer_label->setText(std::format("{}", exp.value()).c_str());
    }
}

/***************************/

CLIModeHandler::CLIModeHandler(MainWindow* win)
    : ModeHandler(win) {
}

void CLIModeHandler::beforeFetch() {
}

void CLIModeHandler::afterFetch() {
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
    if (win->getResultsNum() == 0) {
        return;
    }

    int i = win->getCurrentResultIdx();
    std::cout << entries[results_indices[i]];
    exit(0);
}

ResultsVec CLIModeHandler::fetch(const QString& query_) {
    return customSearch(win, query_, entries, results_indices, false);
}

void CLIModeHandler::handleQuickLock() {
}

std::string CLIModeHandler::handleModeText() {
    return std::format("Results");
}

bool CLIModeHandler::needsAsyncFetch() {
    return true;
};

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

    if (win->getResultsNum() == 0) {
        return;
    }

    int i = win->getCurrentResultIdx();
    // start a new process that open the full path
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(abs_results[i]);
    process->start("open", args);
    win->sleep();
}

ResultsVec FileModeHandler::fetch(const QString& query_) {
    abs_results.clear();
    if (query_.size() <= 2) {
        return {};
    }

    auto query = query_.right(query_.size() - 2);

    auto files = spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));

    ResultsVec res{};
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
    if (win->getResultsNum() == 0) {
        return;
    }

    quickLock(abs_results[win->getCurrentResultIdx()]);
}

std::string FileModeHandler::handleModeText() {
    return std::format("Files");
}

void FileModeHandler::handleCopy() {
    QMimeData* mime_data = new QMimeData();

    // Create a list with a single file URL
    QString file_path = QString::fromStdString(abs_results[win->getCurrentResultIdx()]);
    QList<QUrl> urls;
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

void FileModeHandler::beforeFetch() {
}

void FileModeHandler::afterFetch() {
}

bool FileModeHandler::needsAsyncFetch(){
    return true;
};

/***************************/

static ResultsVec customSearch(MainWindow* win, const QString& query_, const std::vector<std::string>& entries,
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

    // sort in decreasing order according to score
    std::sort(scores_per_idx.begin(), scores_per_idx.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first > rhs.first;
    });

    ResultsVec res{};
    res.reserve(scores_per_idx.size());

    // fill resultslist based on the scores
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

#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <unordered_map>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

AppModeHandler::AppModeHandler(MainWindow* win)
    : ModeHandler(win) {

    app_watcher = new QFileSystemWatcher(win);
    QObject::connect(app_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win](const QString& path) {
        load();
        win->refreshResults();
    });
}

AppModeHandler::~AppModeHandler() {};

QString AppModeHandler::handleModeText() {
    return "";
}

void AppModeHandler::handleQuickLook() {
}

void AppModeHandler::load() {
    app_paths.clear();
    freeWidgets();

    QStringList paths;
    for (const auto& p : win->getConfigManager().getList<std::string>({"mode", "apps", "dirs"})) {
        paths.push_back(QString::fromStdString(p));
    }
    expandPaths(paths);

    // reload apps in defined app dirs
    for (const auto& path : paths) {

        QDir dir(path);
        if (!dir.exists())
            continue;

        QFileInfoList entryInfoList = dir.entryInfoList(QDir::Dirs);
        for (const QFileInfo& entry : entryInfoList) {
            if (entry.suffix() == "app") {
                app_paths << entry.absoluteFilePath();
            }
        }
    }

    // watch new app dirs
    if (app_watcher->directories().size() > 0) {
        app_watcher->removePaths(app_watcher->directories());
    }

    QStringList paths_list{};
    for (const auto& path : paths) {
        paths_list.push_back(path);
    }
    app_watcher->addPaths(paths_list);

    paths = {};
    for (const auto& p : win->getConfigManager().getList<std::string>({"mode", "apps", "apps"})) {
        paths.push_back(QString::fromStdString(p.c_str()));
    }
    expandPaths(paths);

    // add special apps(specific apps instead of dirs)
    for (const auto& path : paths) {
        app_paths.push_back(path);
        widgets.push_back(
            new FileWidget(win, main_widget, path, win->getConfigManager().get<bool>({"mode", "apps", "show_icons"})));
    }

    win->processResults(widgets);
}

void AppModeHandler::enterHandler() {

    if (win->getResultsNum() == 0 || win->getCurrentResultIdx() < 0) {
        return;
    }

    int i = std::max(win->getCurrentResultIdx(), 0);
    qDebug() << i;
    qDebug() << win->getResultsNum();
    widgets[i]->enterHandler();
}

void AppModeHandler::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget();
}

void AppModeHandler::invokeQuery(const QString& query) {

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

    std::vector<int> indices{};
    auto search_results =
        filter(query, app_paths, &indices, [](const QString& str) { return QFileInfo(str).fileName(); });

    // BUG: Mode widgets make memory consumption like a 1GB for some reason.

    auto modes_widgets = win->getModesWidgets();
    std::unordered_map<QString, FuzzyWidget*> search_to_widget{};

    QStringList modes_search_phrases{};
    modes_search_phrases.reserve(modes_widgets.size());
    for (auto* widget : modes_widgets) {
        widget->setParent(main_widget);
        modes_search_phrases.push_back(widget->getSearchPhrase());
        search_to_widget.insert({widget->getSearchPhrase(), widget});
    }
    auto modes_results = filter(query, modes_search_phrases);

    for (int i = 0; i < search_results.size(); ++i) {
        widgets.push_back(new FileWidget(win,
                                         main_widget,
                                         app_paths[indices[i]],
                                         win->getConfigManager().get<bool>({"mode", "apps", "show_icons"})));
    }

    for (const auto& key : modes_results) {
        widgets.push_back(search_to_widget[key]);
    }

    win->processResults(widgets);
}

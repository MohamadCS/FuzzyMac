#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

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

QString AppModeHandler::handleModeText() {
    return "";
}
void AppModeHandler::handleQuickLook() {
}

void AppModeHandler::load() {
    QStringList paths;
    for (const auto& p : get_array<std::string>(win->getConfig(), {"mode", "apps", "dirs"})) {
        paths.push_back(QString::fromStdString(p));
    }
    expandPaths(paths);

    apps.clear();
    freeWidgets();

    // reload apps in defined app dirs
    for (const auto& path : paths) {

        QDir dir(path);
        if (!dir.exists())
            continue;

        QFileInfoList entryInfoList = dir.entryInfoList(QDir::Dirs);
        for (const QFileInfo& entry : entryInfoList) {
            if (entry.suffix() == "app") {
                apps << entry.absoluteFilePath();
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
    for (const auto& p : get_array<std::string>(win->getConfig(), {"mode", "apps", "apps"})) {
        paths.push_back(QString::fromStdString(p.c_str()));
    }
    expandPaths(paths);

    // add special apps(specific apps instead of dirs)
    for (const auto& path : paths) {
        apps.push_back(path);
        widgets.push_back(
            new FileWidget(win, main_widget, path, get<bool>(win->getConfig(), {"mode", "apps", "show_icons"})));
    }

    QStringList keys;
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

    int i = std::max(win->getCurrentResultIdx(), 0);
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

    QStringList keys;
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

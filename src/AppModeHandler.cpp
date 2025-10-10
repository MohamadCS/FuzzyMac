#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/Utils.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <optional>
#include <print>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

AppModeHandler::AppModeHandler(MainWindow* win)
    : ModeHandler(win) {
    createBindings();

    future_watcher = new QFutureWatcher<QStringList>(win);

    QObject::connect(future_watcher, &QFutureWatcher<QStringList>::finished, win, [this, win]() {
        if (win->getQuery().isEmpty()) {
            return;
        }

        auto results = future_watcher->result();

        std::println("Found {} results", results.size());
        const bool show_icons = win->getConfigManager().get<bool>({"mode", "apps", "show_icons"});

        for (const auto& app_path : results) {
            if (widgets.size() >= 25) {
                break;
            }
            widgets.push_back(new FileWidget(win, main_widget, app_path, show_icons));
        }

        win->processResults(widgets);
    });
}

AppModeHandler::~AppModeHandler() {};

QString AppModeHandler::handleModeText() {
    return "";
}

void AppModeHandler::createBindings() {

    // Calls the enter handler for each widget.
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0 || win->getCurrentResultIdx() < 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        qDebug() << i;
        qDebug() << win->getResultsNum();
        widgets[i]->enterHandler();
    });
}

void AppModeHandler::load() {

    const auto& new_dirs = win->getConfigManager().getList<std::string>({"mode", "apps", "dirs"});
    const auto& special_apps = win->getConfigManager().getList<std::string>({"mode", "apps", "apps"});

    app_dirs.clear(); // clear old dirs
    app_dirs.reserve(new_dirs.size());

    // add new dirs to limit search for
    for (const auto& path : new_dirs) {
        app_dirs.push_back(QString::fromStdString(path));
    }

    app_paths.clear();
    app_paths.reserve(special_apps.size());
    for (const auto& path : special_apps) {
        app_paths.push_back(QString::fromStdString(path));
    }

    std::println("Loaded {} dirs", app_dirs.size());
}

void AppModeHandler::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget();
}

void AppModeHandler::setupCalcWidget(const QString& query) {

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
}

void AppModeHandler::invokeQuery(const QString& query) {

    qDebug() << "Query Invoked";
    freeWidgets();
    setupCalcWidget(query);

    if (query.isEmpty()) {
        win->processResults({});
        return;
    }

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        auto results = spotlightSearch(app_dirs, "kMDItemContentType == 'com.apple.application-bundle'");
        results += app_paths;
        results.removeDuplicates();
        return filter(query, results, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
    });

    future_watcher->setFuture(future);
}

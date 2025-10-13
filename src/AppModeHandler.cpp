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
    createBindings();

    future_watcher = new QFutureWatcher<QStringList>(win);
    fs_watcher = new QFileSystemWatcher(win);

    QObject::connect(fs_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win] { reloadEntries(); });

    QObject::connect(future_watcher, &QFutureWatcher<QStringList>::finished, win, [this, win]() {
        auto modes_widgets = win->getModesWidgets();
        std::unordered_map<QString, FuzzyWidget*> phrase_to_widget{};
        QStringList phrases{};
        phrases.reserve(modes_widgets.size());

        for (auto* widget : modes_widgets) {
            widget->setParent(main_widget);
            phrase_to_widget[widget->getSearchPhrase()] = widget;
            phrases.push_back(widget->getSearchPhrase());
        }

        auto modes_results = filter(win->getQuery(), phrases);

        if (win->getQuery().isEmpty()) {
            for (const auto& phrase : modes_results) {
                widgets.push_back(phrase_to_widget[phrase]);
            }
            win->processResults(widgets);

            return;
        }

        auto results = future_watcher->result();
        const bool show_icons = win->getConfigManager().get<bool>({"mode", "apps", "show_icons"});

        // Create modes main mode widgets

        // Create widgets and process them
        for (const auto& app_path : results) {
            if (widgets.size() >= 25) {
                break;
            }
            widgets.push_back(new FileWidget(win, main_widget, app_path, show_icons));
        }

        for (const auto& phrase : modes_results) {
            widgets.push_back(phrase_to_widget[phrase]);
        }

        win->processResults(widgets);
    });
}

AppModeHandler::~AppModeHandler() {};

void AppModeHandler::reloadEntries() {
    loaded_apps = spotlightSearch(app_dirs, "kMDItemContentType == 'com.apple.application-bundle'");
    loaded_apps += special_apps;
    loaded_apps.removeDuplicates();
}

QString AppModeHandler::getModeText() {
    return "FuzzyMac";
}

void AppModeHandler::createBindings() {

    // Calls the enter handler for each widget.
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0 || win->getCurrentResultIdx() < 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        widgets[i]->enterHandler();
    });
}

void AppModeHandler::load() {

    // get configs
    app_dirs = fromQList(win->getConfigManager().getList<std::string>({"mode", "apps", "dirs"}));
    special_apps = fromQList(win->getConfigManager().getList<std::string>({"mode", "apps", "apps"}));

    fs_watcher->addPaths(app_dirs);

    reloadEntries();
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

void AppModeHandler::setupBluetoothWidgets(const QString& query) {
    auto bluetooth_devices = getPairedBluetoothDevices();
    QStringList bluetooth_names;
    for (auto& [name, _] : bluetooth_devices) {
        bluetooth_names.push_back(name);
    }

    bluetooth_names = filter(query, bluetooth_names);
    for (const auto& name : bluetooth_names) {
        widgets.push_back(new BluetoothDeviceWidget(win, main_widget, bluetooth_devices[name], name));
    }
}

void AppModeHandler::invokeQuery(const QString& query) {

    freeWidgets();
    setupCalcWidget(query);
    setupBluetoothWidgets(query);

    // Cancel current query search
    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        return filter(query, loaded_apps, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
    });

    future_watcher->setFuture(future);
}

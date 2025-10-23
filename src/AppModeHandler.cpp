#include "FuzzyMac/AppModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/Utils.hpp"
#include "spdlog/spdlog.h"

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

AppModeHandler::AppModeHandler(QWidget* parent, API* api)
    : ModeHandler(parent, api) {

    // Scripts

    createBindings();

    future_watcher = new QFutureWatcher<QStringList>(parent);
    fs_watcher = new QFileSystemWatcher(parent);
    scripts_dir_watcher = new QFileSystemWatcher(parent);

    QObject::connect(fs_watcher, &QFileSystemWatcher::directoryChanged, parent, [this, parent] { reloadEntries(); });

    QObject::connect(
        scripts_dir_watcher, &QFileSystemWatcher::directoryChanged, parent, [this, parent] { reloadScripts(); });

    QObject::connect(future_watcher, &QFutureWatcher<QStringList>::finished, parent, [this]() {
        auto modes_widgets = this->api->getModesWidgets();
        std::unordered_map<QString, FuzzyWidget*> phrase_to_widget{};
        QStringList phrases{};
        phrases.reserve(modes_widgets.size());

        for (auto* widget : modes_widgets) {
            widget->setParent(main_widget);
            phrase_to_widget[widget->getSearchPhrase()] = widget;
            phrases.push_back(widget->getSearchPhrase());
        }

        auto modes_results = filter(this->api->getQuery(), phrases);

        if (this->api->getQuery().isEmpty()) {
            for (const auto& phrase : modes_results) {
                widgets.push_back(phrase_to_widget[phrase]);
            }
            this->api->processResults(widgets);

            return;
        }

        auto results = future_watcher->result();
        const bool show_icons = this->api->getConfigManager().get<bool>({"mode", "apps", "show_icons"});

        // Create modes main mode widgets

        // Create widgets and process them
        for (const auto& app_path : results) {
            if (widgets.size() >= 25) {
                break;
            }
            widgets.push_back(new FileWidget(this->main_widget, this->api, app_path, show_icons));
        }

        for (const auto& phrase : modes_results) {
            widgets.push_back(phrase_to_widget[phrase]);
        }

        this->api->processResults(widgets);
    });
    load();
}

AppModeHandler::~AppModeHandler() {};

void AppModeHandler::reloadScripts() {

    scripts.clear();

    QStringList cand_scripts = {};
    for (const auto& dir : scripts_dir_paths) {
        loadDirs(dir, cand_scripts, false);
    }

    for (const auto& script_path : cand_scripts) {
        QFileInfo info(script_path);
        if (info.isExecutable()) {
            scripts.push_back(script_path);
        }
    }

    spdlog::info("Got {} dirs", scripts_dir_paths.size());
    spdlog::info("Loaded {} scripts", scripts.size());
}
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
        if (api->getResultsNum() == 0 || api->getCurrentResultIdx() < 0) {
            return;
        }

        int i = std::max(api->getCurrentResultIdx(), 0);
        widgets[i]->enterHandler();
    });
}

void AppModeHandler::load() {

    // get configs
    fs_watcher->removePaths(app_dirs);
    scripts_dir_watcher->removePaths(scripts_dir_paths);

    app_dirs = fromQList(api->getConfigManager().getList<std::string>({"mode", "apps", "dirs"}));
    expandPaths(app_dirs);

    special_apps = fromQList(api->getConfigManager().getList<std::string>({"mode", "apps", "apps"}));
    expandPaths(special_apps);

    scripts_dir_paths = fromQList(api->getConfigManager().getList<std::string>({"mode", "apps", "script_paths"}));
    expandPaths(scripts_dir_paths);

    fs_watcher->addPaths(app_dirs);
    scripts_dir_watcher->addPaths(scripts_dir_paths);

    reloadEntries();
    reloadScripts();
}

void AppModeHandler::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget();
}

void AppModeHandler::setupActions(const QString& query) {
    if (query.isEmpty()) {
        return;
    }

    auto filtered_scripts =
        filter(query, scripts, nullptr, [](const QString& path) { return QFileInfo(path).fileName(); });

    for (const auto& script_path : filtered_scripts) {
        widgets.push_back(new ActionWidget(main_widget, api, QFileInfo(script_path).fileName(), script_path));
    }
}

void AppModeHandler::setupCalcWidget(const QString& query) {

    auto exp = evalMathExp(query.toStdString());

    if (query.isEmpty()) {
        math_mode = false;
    }

    if (math_mode || exp.has_value()) {
        math_mode = true;
        auto* calc_widget = new CalculatorWidget(main_widget, api);
        if (exp.has_value()) {
            calc_widget->answer_label->setText(std::format("{}", exp.value()).c_str());
        }
        widgets.push_back(calc_widget);
    }
}

void AppModeHandler::setupBluetoothWidgets(const QString& query) {
    if (query.isEmpty()) {
        return;
    }
    auto bluetooth_devices = getPairedBluetoothDevices();

    // Assumes devices have different names
    std::unordered_map<QString, BluetoothDevice> name_to_dev;
    QStringList bluetooth_names;
    for (const auto& device : bluetooth_devices) {
        bluetooth_names.push_back(device.name);
        name_to_dev.insert_or_assign(device.name, device);
    }

    bluetooth_names = filter(query, bluetooth_names);
    for (const auto& name : bluetooth_names) {
        auto device = name_to_dev[name];
        widgets.push_back(new BluetoothDeviceWidget(main_widget, api, device));
    }
}

void AppModeHandler::invokeQuery(const QString& query) {

    freeWidgets();
    // TODO: combine filtering using all of the items below.
    setupCalcWidget(query);
    setupBluetoothWidgets(query);
    setupActions(query);

    // Cancel current query search
    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        return filter(query, loaded_apps, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
    });

    future_watcher->setFuture(future);
}

#include "FuzzyMac/WallpaperMode.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ImageViewerInfoPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/Utils.hpp"

#include "spdlog/spdlog.h"

#include <QtConcurrent>

WallpaperMode::WallpaperMode(QWidget* parent, API* api)
    : ModeHandler(parent, api) {
    fs_watcher = new QFileSystemWatcher(parent);
    future_watcher = new QFutureWatcher<QStringList>(parent);

    setupKeymaps();

    QObject::connect(fs_watcher, &QFileSystemWatcher::directoryChanged, parent, [this](const auto& p) {
        spdlog::debug("Directory changed");
        reloadEntries();
    });

    QObject::connect(future_watcher, &QFutureWatcher<QStringList>::finished, [this, parent]() {
        freeWidgets();
        for (const auto& file_path : future_watcher->result()) {
            auto show_icons = this->api->getConfigManager().get<bool>({"mode", "wallpaper", "show_icons"});
            widgets.push_back(new ImageWidget(this->main_widget, this->api, file_path));
        }

        this->api->processResults(widgets);
    });
}

void WallpaperMode::setupKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (api->getResultsNum() == 0) {
            return;
        }

        int i = std::max(api->getCurrentResultIdx(), 0);
        auto path = dynamic_cast<ImageWidget*>(widgets[api->getCurrentResultIdx()])->getPath();

        setWallpaperForAllMonitors(path);

        api->sleep();
    });

    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_Return), [this] {
        if (api->getResultsNum()) {
            // TODO: Free memory after quiting quicklook, or find why its not crucial to do so.
            showQuickLookPanel(dynamic_cast<ImageWidget*>(widgets[api->getCurrentResultIdx()])->getPath());
        }
    });
}

void WallpaperMode::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget(nullptr);
}

void WallpaperMode::reloadEntries() {
    entries = spotlightSearch(paths, "kMDItemFSName != ''");
    spdlog::info("Reloaded {} wallpapers", entries.size());
}

void WallpaperMode::load() {
    freeWidgets();

    const auto old_paths = paths;
    paths = fromQList(api->getConfigManager().getList<std::string>({"mode", "wallpaper", "paths"}));
    expandPaths(paths);

    if (paths == old_paths) {
        return;
    }

    fs_watcher->removePaths(old_paths);
    fs_watcher->addPaths(paths);

    reloadEntries();
}

void WallpaperMode::invokeQuery(const QString& query) {
    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        if (query.isEmpty()) {
            return entries;
        }
        return filter(query, entries, nullptr, [](const QString& path) { return QFileInfo(path).fileName(); });
    });

    future_watcher->setFuture(future);
}

QString WallpaperMode::getModeText() {
    return QString("Pick a wallpaper (loaded %1)").arg(entries.size());
}

QString WallpaperMode::getPrefix() const {
    return "ww";
}

InfoPanelContent* WallpaperMode::getInfoPanelContent() const {
    return nullptr;
}

std::vector<FuzzyWidget*> WallpaperMode::createMainModeWidgets() {
    return {
        new ModeWidget(
            parent,
            api,
            "Change Wallpaper",
            Mode::WALLPAPER,
            [this]() { api->changeMode(Mode::WALLPAPER); },
            api->getIcons()["wallpaper"]),
    };
}

void WallpaperMode::onModeExit() {
    api->setResultsListView(QListView::ListMode);
}

void WallpaperMode::onModeEnter() {
    api->setResultsListView(QListView::IconMode);
}

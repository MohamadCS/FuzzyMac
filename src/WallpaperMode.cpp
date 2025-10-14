#include "FuzzyMac/WallpaperMode.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ImageViewerInfoPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/Utils.hpp"

#include "spdlog/spdlog.h"

#include <QtConcurrent>

WallpaperMode::WallpaperMode(MainWindow* win)
    : ModeHandler(win) {
    fs_watcher = new QFileSystemWatcher(win);
    future_watcher = new QFutureWatcher<QStringList>(win);

    setupKeymaps();

    QObject::connect(fs_watcher, &QFileSystemWatcher::directoryChanged, win, [this](const auto& p) {
        spdlog::debug("Directory changed");
        reloadEntries();
    });

    QObject::connect(future_watcher, &QFutureWatcher<QStringList>::finished, [this, win]() {
        freeWidgets();
        for (const auto& file_path : future_watcher->result()) {
            auto show_icons = win->getConfigManager().get<bool>({"mode", "wallpaper", "show_icons"});
            spdlog::info(show_icons);

            widgets.push_back(new FileWidget(win, main_widget, file_path, show_icons));
        }

        win->processResults(widgets);
    });
}

void WallpaperMode::setupKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        auto path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();

        QString script = QString("tell application \"System Events\"\n"
                                 "set desktopCount to count of desktops\n"
                                 "repeat with i from 1 to desktopCount\n"
                                 "set picture of desktop i to \"%1\"\n"
                                 "end repeat\n"
                                 "end tell")
                             .arg(path);

        QProcess process;
        process.start("osascript", QStringList() << "-e" << script);
        process.waitForFinished();
        win->sleep();
    });

    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_Return), [this] {
        if (win->getResultsNum()) {
            // TODO: Free memory after quiting quicklook, or find why its not crucial to do so.
            showQuickLookPanel(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
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
    paths = fromQList(win->getConfigManager().getList<std::string>({"mode", "wallpaper", "paths"}));
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
    if (win->getResultsNum() == 0) {
        return nullptr;
    }

    int i = std::max(win->getCurrentResultIdx(), 0);

    return new ImageViewerInfoPanel(main_widget, win, dynamic_cast<FileWidget*>(widgets[i])->getPath());
}

std::vector<FuzzyWidget*> WallpaperMode::createMainModeWidgets() {
    return {
        new ModeWidget(
            win,
            nullptr,
            "Change Wallpaper",
            Mode::WALLPAPER,
            [this]() { win->changeMode(Mode::WALLPAPER); },
            win->getIcons()["wallpaper"]),
    };
}

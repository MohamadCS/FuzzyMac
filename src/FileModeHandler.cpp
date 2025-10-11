#include "FuzzyMac/FileModeHandler.hpp"
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
#include <utility>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

static QString getParentDirPath(const QString& path) {
    QFileInfo info(path);
    return info.dir().absolutePath();
}

static void loadDirs(const QString& d, QStringList& paths, bool rec = true) {

    QDir dir(d);
    QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& entry : entryInfoList) {
        auto abs_path = entry.absoluteFilePath();

        if (rec && entry.isDir() && !entry.isSymLink()) {
            loadDirs(abs_path, paths);
        }

        paths.push_back(abs_path);
    }
}

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {

    // QObjects
    future_watcher = new QFutureWatcher<QStringList>(win);
    fs_watcher = new QFileSystemWatcher(win);

    setupKeymaps();
    connectHandlers();

    load();
}

void FileModeHandler::reloadEntries() {
    entries = spotlightSearch(paths, "kMDItemFSName != ''"); // store all indexed files in config dirs
    spdlog::info("App file memory reloaded with {} files", entries.size());
}

void FileModeHandler::setupKeymaps() {

    // Handle Enter
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        widgets[i]->enterHandler();
    });

    // Handle Enter
    keymap.bind(QKeySequence::Copy, [this]() {
        QMimeData* mime_data = new QMimeData();

        // Create a list with a single file URL
        QString file_path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();
        QList<QUrl> urls;
        urls.append(QUrl::fromLocalFile(std::move(file_path)));

        mime_data->setUrls(urls);

        // Set the clipboard contents
        QGuiApplication::clipboard()->setMimeData(mime_data);
    });

    // Do Quicklook
    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_Return), [this]() {
        if (win->getResultsNum()) {
            // TODO: Free memory after quiting quicklook, or find why its not crucial to do so.
            showQuickLookPanel(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
        }
    });

    // Navigate back
    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_B), [this]() {
        if (isRelativeFileSearch()) {
            curr_path = getParentDirPath(curr_path.value());
            win->refreshResults();
        }
    });

    // Navigate to folder.
    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_O), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        auto path = dynamic_cast<FileWidget*>(widgets[i])->getPath();
        QFileInfo info(path);
        if (!info.isDir()) {
            return;
        }

        curr_path = path;

        win->clearQuery();
    });
}

bool FileModeHandler::isRelativeFileSearch() const {
    return curr_path.has_value();
}

void FileModeHandler::freeWidgets() {

    main_widget->deleteLater();
    widgets.clear();

    main_widget = new QWidget(nullptr);
}

void FileModeHandler::load() {
    if (future_watcher->isRunning()) {
        future_watcher->cancel();
        future_watcher->waitForFinished();
    }

    freeWidgets();

    auto& cfg = win->getConfigManager();

    paths = fromQList(cfg.getList<std::string>({"mode", "files", "dirs"}));
    expandPaths(paths);

    reloadEntries();
}

FileModeHandler::~FileModeHandler() {
}

void FileModeHandler::invokeQuery(const QString& query_) {
    auto query = query_.trimmed();

    if (query.isEmpty() && !isRelativeFileSearch()) {
        win->processResults({});
    }

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    // TODO: Organize this
    if (isRelativeFileSearch()) {

        QStringList curr_entries{};

        // load current path files and dirs into curr_entries non-recursivly
        loadDirs(curr_path.value(), curr_entries, false);

        // fuzzy search over those entries
        auto future = QtConcurrent::run([this, query, curr_entries]() -> QStringList {
            if (query.isEmpty()) {
                return curr_entries;
            }
            return filter(query, curr_entries, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
        });

        future_watcher->setFuture(future);

        return;
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        return filter(query, entries, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
    });

    future_watcher->setFuture(future);
}

QString FileModeHandler::getModeText() {

    // in relative search the text will be the trimmed current path.
    if (isRelativeFileSearch()) {
        QFileInfo info(curr_path.value());
        return QString("%1").arg(cutPathPrefix(info.absoluteFilePath(), 70));
    }

    return "Files";
}

void FileModeHandler::handleDragAndDrop(QDrag* drag) const {
    auto path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();

    QIcon icon = win->getFileIcon(path);

    QMimeData* mime_data = new QMimeData;
    QPixmap pixmap = icon.pixmap(64, 64); // Creates an icon when dragging the file

    mime_data->setUrls({QUrl::fromLocalFile(path)}); // URL to copy

    drag->setMimeData(mime_data);
    drag->setPixmap(pixmap);
    drag->exec(Qt::CopyAction);
}

QString FileModeHandler::getPrefix() const {
    return " ";
}

InfoPanelContent* FileModeHandler::getInfoPanelContent() const {
    if (win->getResultsNum() == 0) {
        return nullptr;
    }

    int i = std::max(win->getCurrentResultIdx(), 0);

    return new FileInfoPanel(main_widget, win, dynamic_cast<FileWidget*>(widgets[i])->getPath());
}

std::vector<FuzzyWidget*> FileModeHandler::createMainModeWidgets() {
    return {
        new ModeWidget(
            win,
            nullptr,
            "Files",
            Mode::FILE,
            [this]() { win->changeMode(Mode::FILE); },
            win->getIcons()["search_files"]),
    };
}

void FileModeHandler::onModeExit() {
    curr_path = std::nullopt;
}

void FileModeHandler::connectHandlers() {
    QObject::connect(
        fs_watcher, &QFileSystemWatcher::directoryChanged, win, [this](const QString&) { reloadEntries(); });

    QObject::connect(future_watcher, &QFutureWatcher<std::vector<QString>>::finished, [this]() {
        if (win->getQuery().isEmpty() && !isRelativeFileSearch()) {
            return;
        }

        freeWidgets();
        auto results = future_watcher->result();
        for (const auto& file : results) {
            if (widgets.size() >= 25) {
                break;
            }

            widgets.push_back(new FileWidget(
                win, main_widget, file, win->getConfigManager().get<bool>({"mode", "files", "show_icons"})));
        }

        win->processResults(widgets);
    });
}

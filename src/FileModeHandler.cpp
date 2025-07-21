#include "FuzzyMac/FileModeHandler.hpp"
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
#include <utility>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

void FileModeHandler::freeWidgets() {

    main_widget->deleteLater();
    widgets.clear();

    main_widget = new QWidget(nullptr);
}

static void loadDirs(const QString& d, QStringList& paths) {

    QDir dir(d);
    QFileInfoList entryInfoList = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& entry : entryInfoList) {
        auto abs_path = entry.absoluteFilePath();
        if (entry.isDir() && !entry.isSymLink()) {
            loadDirs(abs_path, paths);
        }
        paths.push_back(abs_path);
    }
}

void FileModeHandler::load() {

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
        future_watcher->waitForFinished();
    }

    paths.clear();
    freeWidgets();

    auto& cfg = win->getConfigManager();

    for (auto& p : cfg.getList<std::string>({"mode", "files", "dirs"})) {
        paths.push_back(QString::fromStdString(p));
    }
    expandPaths(paths);

    for (auto& dir : paths) {
        loadDirs(dir, entries);
    }

    QStringList paths_list{};
    for (const auto& path : entries) {
        if (QFileInfo(path).isDir()) {
            paths_list.push_back(path);
        }
    }

    if (dir_watcher->directories().size() > 0) {
        dir_watcher->removePaths(dir_watcher->directories());
    }

    dir_watcher->addPaths(paths_list);
}

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {
    future_watcher = new QFutureWatcher<QStringList>(win);
    dir_watcher = new QFileSystemWatcher(win);

    QObject::connect(dir_watcher, &QFileSystemWatcher::directoryChanged, win, [this, win](const QString&) {
        load();
        win->refreshResults();
    });

    QObject::connect(future_watcher, &QFutureWatcher<std::vector<QString>>::finished, [this, win]() {
        if (win->getQuery().isEmpty()) {
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

    load();
}

FileModeHandler::~FileModeHandler() {
}

void FileModeHandler::invokeQuery(const QString& query_) {

    auto query = query_.trimmed();

    if (query.isEmpty()) {
        win->processResults({});
        return;
    }

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, query]() -> QStringList {
        // return spotlightSearch(paths, std::format("kMDItemDisplayName LIKE[cd] '{}*'", query.toStdString()));
        return filter(win, query, entries);
    });

    future_watcher->setFuture(future);
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

    int i = std::max(win->getCurrentResultIdx(), 0);
    widgets[i]->enterHandler();
}

QString FileModeHandler::handleModeText() {
    return "Files";
}

void FileModeHandler::handleCopy() {
    QMimeData* mime_data = new QMimeData();

    // Create a list with a single file URL
    QString file_path = dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath();
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
    mime_data->setUrls({QUrl::fromLocalFile(path)});
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
            main_widget,
            "Search Files",
            Mode::FILE,
            [this]() { win->changeMode(Mode::FILE); },
            win->getIcons()["search_files"]),
    };
}

FileInfoPanel::FileInfoPanel(QWidget* parent, MainWindow* win, QString path)
    : InfoPanelContent(parent, win) {

    image_watcher = new QFutureWatcher<QImage>(this);

    auto& cfg = win->getConfigManager();

    setAutoFillBackground(true);
    setStyleSheet(QString(R"(
            color : %1;
            background-color: %1;
            border-left: 2 solid %2;
    )")
                      .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                      .arg(cfg.get<std::string>({"colors", "inner_border_color"})));

    QString sheet = QString(R"(
            color : %1;
            background: %2;
            font-weight: 300;
            font-family: %3;
            font-size: 12px;
            padding: 5px;
            border: transparent;
    )")
                        .arg(cfg.get<std::string>({"colors", "mode_label", "text"}))
                        .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                        .arg(cfg.get<std::string>({"font"}));

    QFileInfo info{path};

    std::vector<std::pair<QString, QString>> file_info{
        {"File Name", info.fileName()},
        {"Path", info.path()},
        {"Size", convertToReadableFileSize(info.size())},
        {"Modified", info.lastModified().toString()},
    };

    auto* layout = new QVBoxLayout(this);

    auto* thumb = new QLabel(this);
    connect(image_watcher, &QFutureWatcher<QImage>::finished, [this, thumb]() {
        thumb->setPixmap(QPixmap::fromImage(image_watcher->result()));
    });

    thumb->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    thumb->setStyleSheet(sheet);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(thumb, 0);

    auto future = QtConcurrent::run([this, path]() -> QImage {
        auto image = getThumbnailImage(path, 128, 128);
        if (image.isNull()) {
            return this->win->getFileIcon(path).pixmap(128, 128).toImage();
        }
        return image;
    });

    image_watcher->setFuture(future);

    QWidget* center = new QWidget(this);

    center->setStyleSheet(sheet);
    layout->addWidget(center, 1);

    for (auto [name, data] : file_info) {
        auto* label_layout = new QHBoxLayout;
        label_layout->setSpacing(0);
        label_layout->setContentsMargins(0, 0, 0, 0);

        QLabel* name_label = new QLabel(name, this);
        QLabel* data_label = new QLabel(data, this);

        data_label->setWordWrap(true);
        data_label->setStyleSheet(sheet);
        data_label->setAlignment(Qt::AlignRight);

        name_label->setWordWrap(true);
        name_label->setStyleSheet(sheet);

        label_layout->addWidget(name_label);
        label_layout->addWidget(data_label, 1);

        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Plain);
        line->setStyleSheet(QString(R"( color : %1;)").arg(cfg.get<std::string>({"colors", "inner_border_color"})));

        layout->addLayout(label_layout);
        layout->addWidget(line);
    }

    setLayout(layout);
}

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

void FileModeHandler::createKeyMaps() {

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
        if (win->getResultsNum() == 0) {
            return;
        }

        showQuickLookPanel(dynamic_cast<FileWidget*>(widgets[win->getCurrentResultIdx()])->getPath());
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

    paths.clear();
    entries.clear();
    freeWidgets();

    auto& cfg = win->getConfigManager();

    for (auto& p : cfg.getList<std::string>({"mode", "files", "dirs"})) {
        paths.push_back(QString::fromStdString(p));
    }

    expandPaths(paths);

    // for (auto& dir : paths) {
    //     loadDirs(dir, entries);
    // }
    //
    // QStringList paths_list{};
    // for (const auto& path : entries) {
    //     if (QFileInfo(path).isDir()) {
    //         paths_list.push_back(path);
    //     }
    // }
    //
    // if (dir_watcher->directories().size() > 0) {
    //     dir_watcher->removePaths(dir_watcher->directories());
    // }
    //
    // dir_watcher->addPaths(paths_list);
}

FileModeHandler::FileModeHandler(MainWindow* win)
    : ModeHandler(win) {

    createKeyMaps();
    future_watcher = new QFutureWatcher<QStringList>(win);

    QObject::connect(future_watcher, &QFutureWatcher<std::vector<QString>>::finished, [this, win]() {
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

    load();
}

FileModeHandler::~FileModeHandler() {
}

void FileModeHandler::invokeQuery(const QString& query_) {
    qDebug() << "Query Invoked";
    auto query = query_.trimmed();

    if (query.isEmpty() && !isRelativeFileSearch()) {
        win->processResults({});
    }

    if (future_watcher->isRunning()) {
        future_watcher->cancel();
    }

    if (isRelativeFileSearch()) {
        QStringList curr_entries{};
        loadDirs(curr_path.value(), curr_entries, false);
        qDebug() << curr_entries.size();

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
        auto entries_ = spotlightSearch(paths, "kMDItemFSName != ''");
        return filter(query, entries_, nullptr, [](const QString& str) { return QFileInfo(str).fileName(); });
    });

    future_watcher->setFuture(future);
    qDebug() << "Finshed query invoke";
}

QString FileModeHandler::handleModeText() {
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
                      .arg(cfg.get<std::string>({"colors", "inner_border"})));

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
        const auto& image = image_watcher->result();
        if (image.isNull()) {
            return;
        } else {
            thumb->setPixmap(QPixmap::fromImage(image));
        }
    });

    thumb->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    thumb->setStyleSheet(sheet);

    layout->setSpacing(0);
    layout->setContentsMargins(1, 0, 0, 0);
    layout->addWidget(thumb, 1);

    auto future = QtConcurrent::run([this, path]() -> QImage { return getThumbnailImage(path, 128, 128); });

    image_watcher->setFuture(future);

    for (int i = 0; i < file_info.size(); ++i) {
        auto [name, data] = file_info[i];
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
        line->setStyleSheet(QString(R"( color : %1;)").arg(cfg.get<std::string>({"colors", "inner_border"})));
        layout->addWidget(line);
        layout->addLayout(label_layout);
    }

    setLayout(layout);
}

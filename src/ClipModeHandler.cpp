#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ClipInfoPanel.hpp"

#include "spdlog/spdlog.h"
#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QLabel>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QStandardPaths>
#include <QThreadPool>
#include <QtConcurrent>
#include <optional>
#include <variant>

static std::optional<ClipboardManager::Entry::Content> getClipboardData() {
    QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();

    // detect similar content

    if (mime->hasUrls()) {
        return mime->urls();
    }

    if (mime->hasText()) {
        if (mime->text().isEmpty()) {
            return std::nullopt;
        }
        return mime->text();
    }

    return std::nullopt;
}

void ClipModeHandler::createKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        QClipboard* clipboard = QGuiApplication::clipboard();
        QMimeData* mime_data = new QMimeData();

        // Create a list with a single file URL
        auto content = dynamic_cast<ClipboardWidget*>(widgets[win->getCurrentResultIdx()])->getContent();
        if (std::holds_alternative<QString>(content)) {
            const QString& text = std::get<QString>(content);
            mime_data->setText(text);

        } else {
            const QList<QUrl>& urls = std::get<QList<QUrl>>(content);
            mime_data->setUrls(urls);
        }

        // swap entries
        // suppress next change
        //

        auto& entries = clipboard_manager.getEntries();
        auto* widget = dynamic_cast<ClipboardWidget*>(widgets[win->getCurrentResultIdx()]);
        auto entry = entries[widget->getIdx()];
        entries.erase(entries.begin() + widget->getIdx());
        entries.push_back(entry);

        suppress_next_change = true;

        clipboard->setMimeData(mime_data);

        win->sleep();
        win->refreshResults();
    });
}

ClipModeHandler::ClipModeHandler(MainWindow* win)
    : ModeHandler(win),
      clipboard_count{getClipboardCount()},
      suppress_next_change(false) {
    createKeymaps();
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); // ensure directory exists
    path = dataDir + "/clipboard.json";

    QObject::connect(&timer, &QTimer::timeout, [this, win]() {
        int new_count = getClipboardCount();
        if (suppress_next_change) {
            suppress_next_change = false;
            clipboard_count = new_count;
            return;
        }

        if (clipboard_count != new_count) {
            clipboard_count = new_count;
            auto content = getClipboardData();
            auto app_path = QString::fromStdString(getFrontmostAppName());

            for (const auto& banned_app : black_list) {
                if (app_path.contains(QString::fromStdString(banned_app), Qt::CaseInsensitive)) {
                    return;
                }
            }

            if (content.has_value()) {
                dirty = true;
                clipboard_manager.addEntry(content.value(), std::move(app_path));
                win->refreshResults();
            }
        }
    });

    QObject::connect(&save_timer, &QTimer::timeout, [this]() {
        if (dirty) {
            clipboard_manager.saveToFile(path);
        }
        dirty = false;

        spdlog::info("Saved clipboard history to {}", path.toStdString());
    });

    timer.start(500);
    save_timer.start(60000);

    load();
}

void ClipModeHandler::load() {
    freeWidgets();
    black_list = win->getConfigManager().getList<std::string>({"mode", "clipboard", "blacklist"});
    clipboard_manager.loadFromFile(path);
}

void ClipModeHandler::invokeQuery(const QString& query) {
    freeWidgets();

    std::vector<int> idx_vec;
    auto& entries = clipboard_manager.getEntries();
    QStringList list;
    list.reserve(entries.size());

    for (auto& entry : entries) {
        if (std::holds_alternative<QString>(entry.value)) {
            list.push_back(std::get<QString>(entry.value));
        } else {
            list.push_back(std::get<QList<QUrl>>(entry.value).first().toString());
        }
    }

    if (query.isEmpty()) {
        for (int i = entries.size() - 1; i >= 0; --i) {
            widgets.push_back(new ClipboardWidget(win, main_widget, &entries[i].value, i));
        }
    } else {
        filter(query, list, &idx_vec);
        for (int i = 0; i < idx_vec.size(); ++i) {
            widgets.push_back(new ClipboardWidget(win, main_widget, &entries[idx_vec[i]].value, idx_vec[i]));
        }
    }

    win->processResults(widgets);
}

void ClipboardManager::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonArray arr = doc.array();

    QMutexLocker locker(&entries_mutex);
    entries.clear();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            entries.append(Entry::fromJson(val.toObject()));
        }
    }
}

void ClipboardManager::saveToFile(const QString& path) const {
    // Copy entries under lock for thread-safety
    QList<Entry> local_entries;
    {
        QMutexLocker locker(&entries_mutex);
        local_entries = entries;
    }

    // Write asynchronously — capture by value to avoid use-after-free
    auto result = QtConcurrent::run([local_entries, path]() {
        QJsonArray arr;
        for (const auto& entry : local_entries)
            arr.append(entry.toJson());

        QJsonDocument doc(arr);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            return;

        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    });
}

QJsonObject ClipboardManager::Entry::toJson() const {
    QJsonObject obj;
    obj["app"] = app;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);

    if (std::holds_alternative<QString>(value)) {
        obj["type"] = "text";
        obj["value"] = std::get<QString>(value);
    } else if (std::holds_alternative<QList<QUrl>>(value)) {
        obj["type"] = "urls";
        QJsonArray arr;
        for (const QUrl& url : std::get<QList<QUrl>>(value))
            arr.append(url.toString());
        obj["urls"] = arr;
    }

    return obj;
}

ClipboardManager::Entry ClipboardManager::Entry::fromJson(const QJsonObject& obj) {
    Entry e;
    e.app = obj["app"].toString();
    e.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);

    QString type = obj["type"].toString();
    if (type == "text") {
        e.value = obj["value"].toString();
    } else if (type == "urls") {
        QList<QUrl> urls;
        for (const QJsonValue& v : obj["urls"].toArray())
            urls.append(QUrl(v.toString()));
        e.value = urls;
    } else {
        e.value = QString(); // fallback
    }

    return e;
}

void ClipboardManager::addEntry(const Entry::Content& value, const QString& app) {
    QMutexLocker locker(&entries_mutex);

    entries.push_back(Entry{
        .value = value,
        .app = app,
        .timestamp = QDateTime::currentDateTime(),
    });
}

QList<ClipboardManager::Entry>& ClipboardManager::getEntries() {
    return entries;
}

const QList<ClipboardManager::Entry>& ClipboardManager::getEntries() const {
    return entries;
}

QString ClipModeHandler::getPrefix() const {
    return "yy";
}

QString ClipModeHandler::getModeText() {
    return "Clipboard";
}

ClipboardWidget::ClipboardWidget(MainWindow* win, QWidget* parent, ClipboardManager::Entry::Content* value, int idx)
    : FuzzyWidget(win, parent),
      content(value),
      idx(idx) {
    if (std::holds_alternative<QString>(*value)) {
        text = new QLabel(std::get<QString>(*value).left(width()).trimmed().replace('\n', " ⏎ "), this);
    } else {
        text = new QLabel(std::get<QList<QUrl>>(*value).first().toString().left(width()).replace('\n', " ⏎ ") + "...",
                          this);
    }
}

int ClipboardWidget::getIdx() const {
    return idx;
}

ClipboardManager::Entry::Content& ClipboardWidget::getContent() {
    return *content;
}

const ClipboardManager::Entry::Content& ClipboardWidget::getContent() const {
    return *content;
}

std::variant<QListWidgetItem*, FuzzyWidget*> ClipboardWidget::getItem() {
    if (std::holds_alternative<QString>(*content)) {
        return win->createListItem(text->text(), win->getIcons()["text"]);
    }

    return win->createListItem(text->text(), win->getFileIcon(std::get<QList<QUrl>>(*content).first().toLocalFile()));
}

void ClipboardManager::clear() {
    QMutexLocker locker(&entries_mutex);
    entries.clear();
}

std::vector<FuzzyWidget*> ClipModeHandler::createMainModeWidgets() {
    const auto& icons = win->getIcons();
    return {
        new ModeWidget(
            win,
            nullptr,
            "Open Clipboard",
            Mode::CLIP,
            [this]() { win->changeMode(Mode::CLIP); },
            icons.at("clipboard")),

        new ModeWidget(
            win,
            nullptr,
            "Clear Clipboard",
            Mode::CLIP,
            [this]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(win,
                                              "Confirm Action",
                                              "Are you sure you want to clear clipboard?",
                                              QMessageBox::Yes | QMessageBox::No);
                if (reply == QMessageBox::No) {
                    return;
                }

                freeWidgets();
                clipboard_manager.clear();
                clipboard_manager.saveToFile(path);
            },
            icons.at("clear_clipboard")),
    };
}

void ClipModeHandler::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget(nullptr);
}

InfoPanelContent* ClipModeHandler::getInfoPanelContent() const {
    if (win->getResultsNum() == 0) {
        return nullptr;
    }

    auto& entries = clipboard_manager.getEntries();

    return new ClipboardInfoPanel(
        main_widget, win, entries[dynamic_cast<ClipboardWidget*>(widgets[win->getCurrentResultIdx()])->getIdx()]);
}


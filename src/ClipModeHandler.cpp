#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/ClipInfoPanel.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"

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
        if (api->getResultsNum() == 0) {
            return;
        }

        QClipboard* clipboard = QGuiApplication::clipboard();
        QMimeData* mime_data = new QMimeData();

        // Create a list with a single file URL
        auto content = dynamic_cast<ClipboardWidget*>(widgets[api->getCurrentResultIdx()])->getContent();
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
        auto* widget = dynamic_cast<ClipboardWidget*>(widgets[api->getCurrentResultIdx()]);
        auto entry = entries[widget->getIdx()];
        entries.erase(entries.begin() + widget->getIdx());
        entries.push_back(entry);

        suppress_next_change = true;

        clipboard->setMimeData(mime_data);

        api->sleep();
        api->refreshResults();
    });
}

ClipModeHandler::ClipModeHandler(QWidget* parent, API* api)
    : ModeHandler(parent, api),
      clipboard_count{getClipboardCount()},
      suppress_next_change(false) {
    createKeymaps();
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); // ensure directory exists
    path = dataDir + "/clipboard.json";

    QObject::connect(&timer, &QTimer::timeout, [this]() {
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
                this->api->refreshResults();
            }
        }
    });

    QObject::connect(&save_timer, &QTimer::timeout, [this]() {
        if (dirty) {
            clipboard_manager.saveToFile(path);
        }
        dirty = false;

        spdlog::info(
            "Saved clipboard history to {}({} entries)", path.toStdString(), clipboard_manager.getEntries().size());
    });

    timer.start(500);
    save_timer.start(60000);

    load();
}

void ClipModeHandler::load() {
    freeWidgets();
    black_list = api->getConfigManager().getList<std::string>({"mode", "clipboard", "blacklist"});
    clipboard_manager.setLimit(api->getConfigManager().get<int>({"mode", "clipboard", "limit"}));
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
            widgets.push_back(new ClipboardWidget(main_widget, api, &entries[i].value, i));
        }
    } else {
        filter(query, list, &idx_vec);
        for (int i = 0; i < idx_vec.size(); ++i) {
            widgets.push_back(new ClipboardWidget(main_widget, api, &entries[idx_vec[i]].value, idx_vec[i]));
        }
    }

    api->processResults(widgets);
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
        if (entries.count() >= limit) {
            return;
        }
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

    if (entries.size() >= limit) {
        entries.pop_front();
    }

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

ClipboardWidget::ClipboardWidget(QWidget* parent, API* api, ClipboardManager::Entry::Content* value, int idx)
    : FuzzyWidget(parent, api),
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
        return api->createListItem(text->text(), api->getIcons()["text"]);
    }

    return api->createListItem(text->text(), api->getFileIcon(std::get<QList<QUrl>>(*content).first().toLocalFile()));
}

void ClipboardManager::clear() {
    QMutexLocker locker(&entries_mutex);
    entries.clear();
}

std::vector<FuzzyWidget*> ClipModeHandler::createMainModeWidgets() {
    const auto& icons = api->getIcons();
    return {
        new ModeWidget(
            parent,
            api,
            "Open Clipboard",
            Mode::CLIP,
            [this]() { api->changeMode(Mode::CLIP); },
            icons.at("clipboard")),

        new ModeWidget(
            parent,
            api,
            "Clear Clipboard",
            Mode::CLIP,
            [this]() {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(parent,
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
            icons.at("clipboard_clear")),
    };
}

void ClipModeHandler::freeWidgets() {
    main_widget->deleteLater();
    widgets.clear();
    main_widget = new QWidget(nullptr);
}

InfoPanelContent* ClipModeHandler::getInfoPanelContent() const {
    if (api->getResultsNum() == 0) {
        return nullptr;
    }

    auto& entries = clipboard_manager.getEntries();

    return new ClipboardInfoPanel(
        main_widget, api, entries[dynamic_cast<ClipboardWidget*>(widgets[api->getCurrentResultIdx()])->getIdx()]);
}

void ClipboardManager::setLimit(int limit) {
    this->limit = limit;
}

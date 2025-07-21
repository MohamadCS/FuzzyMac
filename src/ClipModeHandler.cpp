#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QLabel>
#include <QMimeData>
#include <QStandardPaths>
#include <algorithm>
#include <ranges>
#include <variant>

static ClipboardManager::Entry::Content getClipboardData() {
    QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();

    if (mime->hasUrls()) {
        qDebug() << "url";
        return mime->urls();
    }

    if (mime->hasText()) {
        qDebug() << "text";
        return mime->text();
    }

    return {};
}

ClipModeHandler::ClipModeHandler(MainWindow* win)
    : ModeHandler(win),
      clipboard_count{getClipboardCount()} {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); // ensure directory exists
    path = dataDir + "/clipboard.json";
    qDebug() << path;

    QObject::connect(&timer, &QTimer::timeout, [this,win]() {
        int new_count = getClipboardCount();
        if (clipboard_count != new_count) {
            clipboard_count = new_count;
            clipboard_manager.addEntry(getClipboardData(), "app");
            qDebug() << clipboard_count;
            invokeQuery(win->getQuery());
        }

        timer.start(1000);
    });

    QObject::connect(&save_timer, &QTimer::timeout, [this]() {
        clipboard_manager.saveToFile(path);

        save_timer.start(1000);
    });

    timer.start(1000);
    save_timer.start(1000);

    load();
}

void ClipModeHandler::load() {
    clipboard_manager.loadFromFile(path);
}

void ClipModeHandler::enterHandler() {

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


    clipboard->setMimeData(mime_data);
    clipboard_count++;

    win->sleep();
}

void ClipModeHandler::handleQuickLook() {
}

void ClipModeHandler::invokeQuery(const QString& query) {
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

    widgets.clear();

    if (query.isEmpty()) {
        for (int i = entries.size() - 1; i >= 0; --i) {
            widgets.push_back(new ClipboardWidget(win, main_widget, &entries[i].value));
        }
    } else {
        filter(win, query, list, &idx_vec);
        for (int i = 0; i < idx_vec.size(); ++i) {
            widgets.push_back(new ClipboardWidget(win, main_widget, &entries[idx_vec[i]].value));
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

    entries.clear();
    qDebug() << arr.size();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            entries.append(Entry::fromJson(val.toObject()));
        }
    }
}

void ClipboardManager::saveToFile(const QString& path) const {
    QJsonArray arr;
    for (const auto& entry : entries)
        arr.append(entry.toJson());

    QJsonDocument doc(arr);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(doc.toJson(QJsonDocument::Indented));
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
    entries.push_back(Entry{
        .value = value,
        .app = app,
        .timestamp = QDateTime::currentDateTime(),
    });
}

QList<ClipboardManager::Entry>& ClipboardManager::getEntries() {
    return entries;
}

QString ClipModeHandler::getPrefix() const {
    return "yy";
}

QString ClipModeHandler::handleModeText() {
    return "Clipboard";
}

ClipboardWidget::ClipboardWidget(MainWindow* win, QWidget* parent, ClipboardManager::Entry::Content* value)
    : FuzzyWidget(win, parent),
      content(value) {
    if (std::holds_alternative<QString>(*value)) {
        text = new QLabel(std::get<QString>(*value).left(30).replace('\n', " ⏎ "));
    } else {
        text = new QLabel(std::get<QList<QUrl>>(*value).first().toString().left(30).replace('\n', " ⏎ ") + "...");
    }
}

ClipboardManager::Entry::Content& ClipboardWidget::getContent() const {
    return *content;
}

std::variant<QListWidgetItem*, FuzzyWidget*> ClipboardWidget::getItem() {
    return win->createListItem(text->text());
}

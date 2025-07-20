#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QMimeData>
#include <QStandardPaths>
#include <algorithm>

ClipModeHandler::ClipModeHandler(MainWindow* win)
    : ModeHandler(win),
      clipboard_count{0} {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); // ensure directory exists
    path = dataDir + "/clipboard.json";
    qDebug() << path;
    save_timer.start(1000);

    QObject::connect(&timer, &QTimer::timeout, [this]() {
        int new_count = getClipboardCount();
        if (clipboard_count != new_count) {
            clipboard_count = new_count;
            clipboard_manager.addEntry(QGuiApplication::clipboard()->text(), "app");
        }
        timer.start(500);
    });

    QObject::connect(&save_timer, &QTimer::timeout, [this]() {
        clipboard_manager.saveToFile(path);
        save_timer.start(1000);
    });

    timer.start(500);
    load();
}

void ClipModeHandler::load() {
    clipboard_manager.loadFromFile(path);
}

void ClipModeHandler::enterHandler() {
    QMimeData* mime_data = new QMimeData();

    // Create a list with a single file URL
    QString text = dynamic_cast<TextWidget*>(widgets[win->getCurrentResultIdx()])->getValue();
    QGuiApplication::clipboard()->setText(text);
    win->sleep();
}

void ClipModeHandler::handleQuickLook() {
}

void ClipModeHandler::invokeQuery(const QString& query) {
    auto results = query.isEmpty() ? clipboard_manager.getEntriesValues()
                                   : filter(win, query, clipboard_manager.getEntriesValues());

    widgets.clear();

    for (auto& entry : results) {
        widgets.push_back(new TextWidget(win, main_widget, entry));
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
    obj["value"] = value;
    obj["timestamp"] = timestamp.toString(Qt::ISODate);
    obj["app"] = app;
    return obj;
}

ClipboardManager::Entry ClipboardManager::Entry::fromJson(const QJsonObject& obj) {
    return {
        .value = obj["value"].toString(),
        .app = obj["app"].toString(),
        .timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate),
    };
}

void ClipboardManager::addEntry(const QString& value, const QString& app) {
    entries.push_back(Entry{
        .value = value,
        .app = app,
        .timestamp = QDateTime::currentDateTime(),
    });
}

QStringList ClipboardManager::getEntriesValues() const {
    QStringList res{};
    res.reserve(entries.size());
    for (auto& entry : entries) {
        auto val = entry.value;
        val.replace('\n', " ⏎ ");
        res.push_back(val);
    }
    std::reverse(res.begin(),res.end());
    return res;
}

QString ClipModeHandler::getPrefix() const {
    return "yy";
}

QString ClipModeHandler::handleModeText() {
    return "Clipboard";
}

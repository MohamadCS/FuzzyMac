#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include <QClipboard>
#include <QDir>
#include <QGuiApplication>
#include <QLabel>
#include <QMimeData>
#include <QStandardPaths>
#include <optional>
#include <variant>

static std::optional<ClipboardManager::Entry::Content> getClipboardData() {
    QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mime = clipboard->mimeData();

    // detect similar content

    if (mime->hasUrls()) {
        qDebug() << "url";
        return mime->urls();
    }

    if (mime->hasText()) {
        qDebug() << "text";
        return mime->text();
    }

    return std::nullopt;
}

ClipModeHandler::ClipModeHandler(MainWindow* win)
    : ModeHandler(win),
      clipboard_count{getClipboardCount()},
      suppress_next_change(false) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir); // ensure directory exists
    path = dataDir + "/clipboard.json";
    qDebug() << path;

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
            if (content.has_value()) {
                dirty = true;
                clipboard_manager.addEntry(content.value(), "app");
                win->refreshResults();
            }
        }
    });

    QObject::connect(&save_timer, &QTimer::timeout, [this]() {
        if (dirty) {
            clipboard_manager.saveToFile(path);
        }
        dirty = false;
    });

    timer.start(1000);
    save_timer.start(10000);

    load();
}

void ClipModeHandler::load() {
    freeWidgets();
    clipboard_manager.loadFromFile(path);
}

void ClipModeHandler::enterHandler() {
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
}

void ClipModeHandler::handleQuickLook() {
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
        filter(win, query, list, &idx_vec);
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

ClipboardWidget::ClipboardWidget(MainWindow* win, QWidget* parent, ClipboardManager::Entry::Content* value, int idx)
    : FuzzyWidget(win, parent),
      content(value),
      idx(idx) {
    if (std::holds_alternative<QString>(*value)) {
        text = new QLabel(std::get<QString>(*value).left(30).replace('\n', " ⏎ "), this);
    } else {
        text = new QLabel(std::get<QList<QUrl>>(*value).first().toString().left(30).replace('\n', " ⏎ ") + "...", this);
    }
}

int ClipboardWidget::getIdx() const {
    return idx;
}

ClipboardManager::Entry::Content& ClipboardWidget::getContent() const {
    return *content;
}

std::variant<QListWidgetItem*, FuzzyWidget*> ClipboardWidget::getItem() {
    return win->createListItem(text->text());
}

void ClipboardManager::clear() {
    entries.clear();
}

std::vector<FuzzyWidget*> ClipModeHandler::createMainModeWidgets() {
    const auto& icons = win->getIcons();
    return {
        new ModeWidget(
            win,
            main_widget,
            "Open Clipboard",
            Mode::CLIP,
            [this]() { win->changeMode(Mode::CLIP); },
            icons.at("clipboard")),

        new ModeWidget(
            win,
            main_widget,
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

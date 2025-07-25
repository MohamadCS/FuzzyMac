#include "FuzzyMac/ClipModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
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
#include <iterator>
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
    });

    timer.start(500);
    save_timer.start(10000);

    load();
}

void ClipModeHandler::load() {
    freeWidgets();
    black_list = win->getConfigManager().getList<std::string>({"mode", "clipboard", "blacklist"});
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

    entries.clear();
    qDebug() << arr.size();
    for (const QJsonValue& val : arr) {
        if (val.isObject()) {
            entries.append(Entry::fromJson(val.toObject()));
        }
    }
}

void ClipboardManager::saveToFile(const QString& path) const {
    auto local_entries = entries;

    QThreadPool::globalInstance()->start([&local_entries, path]() -> void {
        QJsonArray arr;
        for (const auto& entry : local_entries)
            arr.append(entry.toJson());

        QJsonDocument doc(arr);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly))
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

QString ClipModeHandler::handleModeText() {
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

ClipboardInfoPanel::ClipboardInfoPanel(QWidget* parent, MainWindow* win, const ClipboardManager::Entry& entry)
    : InfoPanelContent(parent, win) {

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

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(1, 0, 0, 0);

    // creating content area

    QPlainTextEdit* content_text = new QPlainTextEdit(this);

    content_text->setStyleSheet(sheet);
    content_text->setReadOnly(true);
    content_text->setStyleSheet(QString(R"(
                                QPlainTextEdit {
                                    selection-background-color : %1;
                                    selection-color : %2;
                                    color: %3;
                                    background: %4;
                                    border : none;
                                    padding: 12px;
                                    font-size: 15;
                                    font-family: %5;
                                }
                                  QScrollBar:vertical {
                                            border: none;
                                            background: %4;
                                            width: 12px;  
                                            padding: 2px;
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:vertical {
                                            background: %6;  
                                            min-height: 20px; 
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:vertical:hover {
                                            background: %7;  
                                        }
                                        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                                            background: transparent;
                                            height: 0px;           
                                            border: none;
                                        }
                                        QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
                                            background: transparent;
                                        }

                                        QScrollBar:horizontal {
                                            border: none;
                                            background: %4;
                                            width: 12px;  
                                            padding: 2px;
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:horizontal {
                                            background: %6;  
                                            min-height: 20px; 
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:horizontal:hover {
                                            background: %7;  
                                        }
                                        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                                            background: transparent;
                                            height: 0px;           
                                            border: none;
                                        }
                                        QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {
                                            background: transparent;
                                        }
                                    )")
                                    .arg(cfg.get<std::string>({"colors", "query_input", "selection_background"}))
                                    .arg(cfg.get<std::string>({"colors", "query_input", "selection"}))
                                    .arg(cfg.get<std::string>({"colors", "query_input", "text"}))
                                    .arg(cfg.get<std::string>({"colors", "query_input", "background"}))
                                    .arg(cfg.get<std::string>({"font"}))
                                    .arg(cfg.get<std::string>({"colors", "results_list", "scrollbar_color"}))
                                    .arg(cfg.get<std::string>({"colors", "results_list", "scrollbar_hold_color"})));

    if (std::holds_alternative<QString>(entry.value)) {
        content_text->setPlainText(std::get<QString>(entry.value));
    } else {
        QString text{};
        for (const auto& url : std::get<QList<QUrl>>(entry.value)) {
            auto url_str = url.toString();
            text.append(url_str);
            text.append("\n");
        }

        content_text->setPlainText(text);
    }

    content_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    content_text->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    layout->addWidget(content_text, 1);
    // end content area

    // creating info widgets
    std::vector<std::pair<QString, QString>> file_info{
        {"Source", entry.app},
        {"Time", entry.timestamp.toString()},
    };

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
        line->setStyleSheet(QString(R"( color : %1;)").arg(cfg.get<std::string>({"colors", "inner_border"})));

        layout->addWidget(line);
        layout->addLayout(label_layout);
    }

    setLayout(layout);
}

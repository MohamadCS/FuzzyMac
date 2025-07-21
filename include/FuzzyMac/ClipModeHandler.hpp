#pragma once

#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModeHandler.hpp"

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonObject>
#include <variant>

class ClipboardManager {
public:
    struct Entry {
        using Content = std::variant<QString, QList<QUrl>>;
        Content value;
        QString app;
        QDateTime timestamp;
        QJsonObject toJson() const;
        static Entry fromJson(const QJsonObject& obj);
    };

    void loadFromFile(const QString& path);
    void saveToFile(const QString& path) const;
    void addEntry(const Entry::Content& value, const QString& app);
    void clear();

    QList<Entry>& getEntries();


private:
    QList<Entry> entries;
};

class ClipboardWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    ClipboardWidget(MainWindow* win, QWidget* parent, ClipboardManager::Entry::Content* value, int idx);

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    ClipboardManager::Entry::Content& getContent() const;
    int getIdx() const;

private:
    QLabel* text;
    ClipboardManager::Entry::Content* content;
    int idx;
};

class ClipModeHandler : public ModeHandler {
public:
    ClipModeHandler(MainWindow* win);
    void load() override;
    QString getPrefix() const override;
    QString handleModeText() override;
    std::vector<FuzzyWidget*> createMainModeWidgets() override;

    void enterHandler() override;
    void handleQuickLook() override;
    void invokeQuery(const QString&) override;
    void freeWidgets() override;

private:
    bool dirty = false;
    bool suppress_next_change;
    int clipboard_count;
    QString path;
    std::vector<FuzzyWidget*> widgets;
    ClipboardManager clipboard_manager;
    QTimer timer;
    QTimer save_timer;
};

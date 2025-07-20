#pragma once

#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModeHandler.hpp"

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonObject>

class ClipboardManager {
public:
    struct Entry {
        QString value;
        QString app;
        QDateTime timestamp;
        QJsonObject toJson() const;
        static Entry fromJson(const QJsonObject& obj);
    };

    void loadFromFile(const QString& path);
    void saveToFile(const QString& path) const;
    void addEntry(const QString& value, const QString& app);

    QStringList getEntriesValues() const;

private:
    QList<Entry> entries;
};

class ClipModeHandler : public ModeHandler {
public:
    ClipModeHandler(MainWindow* win);
    void load() override;
    QString getPrefix() const override;
    QString handleModeText() override;

    void enterHandler() override;
    void handleQuickLook() override;
    void invokeQuery(const QString&) override;

private:
    int clipboard_count;
    QString path;
    std::vector<FuzzyWidget*> widgets;
    ClipboardManager clipboard_manager;
    QFileSystemWatcher file_watcher;
    QTimer timer;
    QTimer save_timer;
};

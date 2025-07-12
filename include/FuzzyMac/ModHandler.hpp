#pragma once
#include "FuzzyMac/MainWindow.hpp"

#include <QDrag>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QVBoxLayout>

#include <string>
#include <toml++/toml.h>

class ModeHandler {
protected:
    MainWindow* win;
    QListWidgetItem* createListItem();

public:
    ModeHandler(MainWindow* win);
    virtual void load() = 0;
    virtual void enterHandler() = 0;
    virtual void handleQuickLock() = 0;
    virtual void handleCopy();
    virtual std::string handleModeText() {
        return "empty";
    }
    virtual void handleDragAndDrop(QDrag*);
    virtual void handlePathCopy();
    virtual std::vector<QListWidgetItem*> getResults(const QString& query_) = 0;
    virtual ~ModeHandler() = default;
};

class AppModeHandler : public ModeHandler {
    std::vector<std::string> apps;
    std::vector<std::string> app_dirs;
    std::vector<int> results_indices;
    QFileSystemWatcher* app_watcher;

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;

    virtual std::string handleModeText() override {
        return "Applications";
    }

    void enterHandler() override;
    void handleQuickLock() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
};

class CLIModeHandler : public ModeHandler {

    std::vector<std::string> entries;
    std::vector<int> results_indices;
    bool loaded = false;

public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void enterHandler() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
    std::string handleModeText() override {
        return std::format("Results");
    }
    void handleQuickLock() override;
};

class FileModeHandler : public ModeHandler {
    std::vector<std::string> paths;
    std::vector<std::string> abs_results;

public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override = default;
    void enterHandler() override;
    virtual std::string handleModeText() override {
        return "Files";
    }
    void handleCopy() override;
    void handleDragAndDrop(QDrag*) override;
    void load() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
    void handleQuickLock() override;
};

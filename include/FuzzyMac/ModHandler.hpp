#pragma once
#include "FuzzyMac/FuzzyWidget.hpp"
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
    virtual void unload() {};

    virtual void enterHandler() = 0;
    virtual void handleQuickLock() = 0;
    virtual void handleCopy();
    virtual std::string handleModeText();
    virtual void handleDragAndDrop(QDrag*);
    virtual void handlePathCopy();
    virtual void invokeQuery(const QString& query_) = 0;
    virtual ~ModeHandler() = default;
};

class AppModeHandler : public ModeHandler {
    bool math_mode = false;
    std::vector<std::string> apps;
    std::vector<std::string> app_dirs;
    std::vector<FuzzyWidget*> widgets;
    QFileSystemWatcher* app_watcher;

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;
    std::string handleModeText() override;

    void enterHandler() override;
    void handleQuickLock() override;
    void invokeQuery(const QString& query_) override;
};

class CLIModeHandler : public ModeHandler {
    std::vector<std::string> entries;
    std::vector<TextWidget*> widgets;
    bool loaded = false;

public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void enterHandler() override;
    void invokeQuery(const QString& query_) override;


    std::string handleModeText() override;
    void handleQuickLock() override;
};

class FileModeHandler : public ModeHandler {
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<std::vector<std::string>>* watcher;
    std::vector<std::string> paths;

public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override = default;
    void enterHandler() override;
    std::string handleModeText() override;
    void handleCopy() override;
    void handleDragAndDrop(QDrag*) override;
    void load() override;
    void invokeQuery(const QString& query_) override;
    void handleQuickLock() override;
};

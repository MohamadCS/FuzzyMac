#pragma once
#include "FuzzyMac/MainWindow.hpp"

#include <QFileIconProvider>
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
    ModeHandler(MainWindow* win)
        : win(win) {
    }
    virtual void enterHandler() = 0;
    virtual void fillData() = 0;
    virtual void handleQuickLock() = 0;
    virtual std::vector<QListWidgetItem*> getResults(const QString& query_) = 0;
    virtual ~ModeHandler() = default;
};

class AppModeHandler : public ModeHandler {
    std::vector<std::string> apps;
    std::vector<int> results_indices;

public:
    AppModeHandler(MainWindow* win)
        : ModeHandler(win) {
    }
    ~AppModeHandler() override = default;
    void enterHandler() override;
    void fillData() override;
    void handleQuickLock() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
};

class CLIModeHandler : public ModeHandler {

    std::vector<std::string> entries;
    std::vector<int> results_indices;

public:
    CLIModeHandler(MainWindow* win)
        : ModeHandler(win) {
    }
    ~CLIModeHandler() override = default;
    void enterHandler() override;
    void fillData() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
    void handleQuickLock() override;
};

class FileModeHandler : public ModeHandler {
    std::vector<std::string> paths;
    std::vector<std::string> abs_results;

public:
    FileModeHandler(MainWindow* win)
        : ModeHandler(win) {
    }
    ~FileModeHandler() override = default;
    void enterHandler() override;
    void fillData() override;
    std::vector<QListWidgetItem*> getResults(const QString& query_) override;
    void handleQuickLock() override;
};

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
    virtual bool needsAsyncFetch() = 0;
    virtual std::string handleModeText();
    virtual void handleDragAndDrop(QDrag*);
    virtual void handlePathCopy();
    virtual void beforeFetch() = 0;
    virtual ResultsVec fetch(const QString& query_) = 0;
    virtual void afterFetch() = 0;
    virtual ~ModeHandler() = default;
};

class CalculatorWidget : public QWidget {
    Q_OBJECT;

public:
    CalculatorWidget(MainWindow* win);
    MainWindow* win;
    QLabel* title_label;
    QLabel* answer_label;
};

class AppModeHandler : public ModeHandler {

    bool math_mode = false;
    std::vector<std::string> apps;
    std::vector<std::string> app_dirs;
    std::vector<int> results_indices;
    QFileSystemWatcher* app_watcher;
    CalculatorWidget* calc_widget;

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;

    std::string handleModeText() override;

    bool needsAsyncFetch() override;
    void enterHandler() override;
    void handleQuickLock() override;
    ResultsVec fetch(const QString& query_) override;
    void beforeFetch() override;
    void afterFetch() override;
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
    ResultsVec fetch(const QString& query_) override;

    bool needsAsyncFetch() override;

    std::string handleModeText() override;
    void handleQuickLock() override;
    void beforeFetch() override;
    void afterFetch() override;
};

class FileModeHandler : public ModeHandler {
    std::vector<std::string> paths;
    std::vector<std::string> abs_results;

public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override = default;
    void enterHandler() override;
    std::string handleModeText() override;
    void handleCopy() override;
    void handleDragAndDrop(QDrag*) override;
    bool needsAsyncFetch() override;
    void load() override;
    ResultsVec fetch(const QString& query_) override;
    void handleQuickLock() override;
    void beforeFetch() override;
    void afterFetch() override;
};

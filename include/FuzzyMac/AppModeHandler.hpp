#pragma once
#include "FuzzyMac/ModeHandler.hpp"

class AppModeHandler : public ModeHandler {

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;
    QString handleModeText() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;

private:
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* fs_watcher; // For deltas in our sub-fs;
    QStringList app_dirs; // directory containing apps;
    QStringList special_apps; // apps with no root dir.
    QStringList loaded_apps; 
    std::vector<FuzzyWidget*> widgets;

    bool math_mode = false;

    void createBindings();
    void reloadEntries();
    void setupCalcWidget(const QString& query);
};

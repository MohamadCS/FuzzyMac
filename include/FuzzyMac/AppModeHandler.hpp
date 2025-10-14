#pragma once
#include "FuzzyMac/ModeHandler.hpp"

class AppModeHandler : public ModeHandler {

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;
    QString getModeText() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;

private:
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* fs_watcher; // For deltas in our sub-fs;
    QFileSystemWatcher* scripts_dir_watcher; // For deltas in our sub-fs;
    QStringList app_dirs; // directory containing apps;
    QStringList special_apps; // apps with no root dir.
    QStringList loaded_apps; 
    std::vector<FuzzyWidget*> widgets;
    QStringList scripts_dir_paths;
    QStringList scripts;

    bool math_mode = false;

    void createBindings();
    void reloadEntries();
    void reloadScripts();
    void setupActions(const QString& query);
    void setupCalcWidget(const QString& query);
    void setupBluetoothWidgets(const QString& query);
};

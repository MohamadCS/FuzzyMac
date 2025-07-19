#pragma once
#include "FuzzyMac/ModeHandler.hpp"

class AppModeHandler : public ModeHandler {

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;
    QString handleModeText() override;

    void enterHandler() override;
    void handleQuickLook() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;

private:
    bool math_mode = false;
    QStringList apps;
    std::map<QString, Mode> modes;
    QStringList app_dirs;
    std::vector<FuzzyWidget*> widgets;
    QFileSystemWatcher* app_watcher;
};

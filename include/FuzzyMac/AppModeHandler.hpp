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
    QStringList app_paths;
    QStringList app_dirs;
    std::vector<FuzzyWidget*> widgets;

    bool math_mode = false;

    void createBindings();
    void setupCalcWidget(const QString& query);
};

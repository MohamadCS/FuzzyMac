#pragma once

#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ModeHandler.hpp"

class CLIModeHandler : public ModeHandler {
public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void enterHandler() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;
    QString handleModeText() override;
    void handleQuickLook() override;

private:
    QStringList entries;
    std::vector<FuzzyWidget*> widgets;
    bool loaded = false;
};

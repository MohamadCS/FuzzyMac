#pragma once

#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"

class CLIModeHandler : public ModeHandler {
public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void enterHandler() override;
    void invokeQuery(const QString& query_) override;
    QString handleModeText() override;
    void handleQuickLook() override;

private:
    QStringList entries;
    std::vector<TextWidget*> widgets;
    bool loaded = false;
};


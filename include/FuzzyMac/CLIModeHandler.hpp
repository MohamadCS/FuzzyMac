#pragma once

#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ModeHandler.hpp"

class CLIModeHandler : public ModeHandler {
public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;
    void handleRequest(const QJsonObject&) override;
    QString handleModeText() override;

private:
    QStringList entries;
    std::vector<FuzzyWidget*> widgets;
    bool loaded = false;

    void createKeymaps();
};

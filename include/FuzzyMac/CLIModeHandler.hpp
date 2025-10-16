#pragma once

#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/Server.hpp"

#include "CLI/CLI11.hpp"
#include <QMutexLocker>
#include <iostream>
#include <string>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct ClientData {
    QString std_in;
    QString sep;
    QString title;
    QString mode;
    bool preview;
};

class CLIModeHandler : public ModeHandler {
public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;
    void onModeExit() override;
    QString getModeText() override;
    InfoPanelContent* getInfoPanelContent() const override;

private:
    QStringList entries;
    std::vector<FuzzyWidget*> widgets;
    Server* server;
    ClientData client_data;
    bool loaded = false;
    void createKeymaps();
    void setupServer();
};


#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FileInfoPanel.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/Utils.hpp"
#include "shared/Data.hpp"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

CLIModeHandler::CLIModeHandler(QWidget* parent, API* api)
    : ModeHandler(parent, api) {

    setupServer();
    createKeymaps();
}

void CLIModeHandler::setupServer() {
    server = new Server(parent, api, [this]() { api->sleep(); });
    server->startServer(server_path.c_str());
}

void CLIModeHandler::createKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        QLocalSocket* client = server->getCurrentClient();

        if (!client) {
            return;
        }

        if (client_data.mode == "find") {
            if (api->getResultsNum() == 0) {
                api->sleep();
                return;
            }

            int i = std::max(api->getCurrentResultIdx(), 0);
            spdlog::info("Currently in find mode");
            client->write(widgets[i]->getSearchPhrase().toLocal8Bit());
        } else {
            spdlog::info("About to write {}", api->getQuery().toStdString());
            client->write(api->getQuery().toLocal8Bit());
        }

        client->flush();
        spdlog::info("about to drop");

        api->sleep();
    });
}

void CLIModeHandler::load() {

    freeWidgets();

    if (!server) {
        spdlog::warn("CLI: Server not available.");
        return;
    }

    QLocalSocket* client = server->getCurrentClient();
    if (!client) {
        spdlog::warn("CLI: No active client connection.");
        return;
    }

    spdlog::info("CLI mode reading input...");

    // read client data
    QByteArray buffer = client->readAll(); // ensure to read the current data first otherwise it
                                           // might be missed
    while (client->waitForReadyRead(50)) {
        buffer += client->readAll();
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(buffer, &err);
    if (err.error != QJsonParseError::NoError) {
        spdlog::error("Could not parse client's data: {}", err.errorString().toStdString());
        server->dropConnection();
        return;
    }

    QJsonObject root = doc.object();
    QJsonObject args = root["args"].toObject();

    client_data = ClientData{
        .std_in = root["stdin"].toString(),
        .sep = args["separator"].toString(),
        .title = args["title"].toString(),
        .mode = args["mode"].toString(),
        .format = args["format"].toString(),
        .preview = args["preview"].toBool(),
    };

    spdlog::info("{}", client_data.std_in.toStdString());

    entries.clear();

    // PERF:  Use a regular vec if there is no fomratting
    for (auto raw_value : client_data.std_in.split(client_data.sep, Qt::SkipEmptyParts)) {
        if (!client_data.format.isEmpty()) {
            auto formatted =
                QString::fromStdString(formatRegex(raw_value.toStdString(), client_data.format.toStdString()));
            entries[formatted] = raw_value;
        } else {
            entries[raw_value] = raw_value;
        }
    }

    for (auto& k : entries.keys()) {
        spdlog::info("{} -- {}", k.toStdString(), entries[k].toStdString());
    }
}

void CLIModeHandler::freeWidgets() {
    widgets.clear();
    main_widget->deleteLater();
    main_widget = new QWidget(nullptr);
}

void CLIModeHandler::onModeExit() {
    server->dropConnection();
}

void CLIModeHandler::invokeQuery(const QString& query) {
    freeWidgets();

    if (client_data.mode != "find") {
        api->processResults({});
        return;
    }

    auto results = query.isEmpty() ? entries.keys() : filter(query, entries.keys());

    for (auto& entry : results) {
        widgets.push_back(new CLIWidget(main_widget, api, entry, entries[entry]));
    }

    api->processResults(widgets);
}

QString CLIModeHandler::getModeText() {
    return client_data.title;
}

InfoPanelContent* CLIModeHandler::getInfoPanelContent() const {
    if (api->getResultsNum() == 0) {
        return nullptr;
    }

    if (!client_data.preview) {
        return nullptr;
    }

    int i = std::max(api->getCurrentResultIdx(), 0);
    auto* widget = dynamic_cast<CLIWidget*>(widgets[i]);
    if (widget == nullptr) {
        return nullptr;
    }

    if (!QFileInfo(widget->getSearchPhrase()).exists()) {
        return nullptr;
    }

    return new FileInfoPanel(main_widget, api, widget->getSearchPhrase());
}

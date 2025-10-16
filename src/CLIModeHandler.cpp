
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "FuzzyMac/FileInfoPanel.hpp"
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

CLIModeHandler::CLIModeHandler(MainWindow* win)
    : ModeHandler(win) {

    setupServer();
    createKeymaps();
}

void CLIModeHandler::setupServer() {
    server = new Server(win, [this]() { win->sleep(); });
    server->startServer("/tmp/fuzzymac_socket");
}

void CLIModeHandler::createKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        QLocalSocket* client = server->getCurrentClient();

        if (!client) {
            return;
        }

        if (client_data.mode == "find") {
            if (win->getResultsNum() == 0) {
                win->sleep();
                return;
            }

            int i = std::max(win->getCurrentResultIdx(), 0);
            spdlog::info("Currently in find mode");
            client->write(widgets[i]->getSearchPhrase().toLocal8Bit());
        } else {
            spdlog::info("About to write {}", win->getQuery().toStdString());
            client->write(win->getQuery().toLocal8Bit());
        }

        client->flush();
        spdlog::info("about to drop");

        win->sleep();
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

    spdlog::info("format  = {}", client_data.format.toStdString());

    entries = client_data.std_in.split(client_data.sep);
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
        win->processResults({});
        return;
    }

    auto results = query.isEmpty() ? entries : filter(query, entries);

    for (auto& entry : results) {
        widgets.push_back(new TextWidget(win, main_widget, entry, client_data.format));
    }

    win->processResults(widgets);
}

QString CLIModeHandler::getModeText() {
    return client_data.title;
}

InfoPanelContent* CLIModeHandler::getInfoPanelContent() const {
    if (win->getResultsNum() == 0) {
        return nullptr;
    }

    if (!client_data.preview) {
        return nullptr;
    }

    int i = std::max(win->getCurrentResultIdx(), 0);
    auto path = dynamic_cast<TextWidget*>(widgets[i])->getSearchPhrase();
    if (!QFileInfo(path).exists()) {
        return nullptr;
    }

    return new FileInfoPanel(main_widget, win, dynamic_cast<TextWidget*>(widgets[i])->getSearchPhrase());
}

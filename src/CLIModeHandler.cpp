
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"
#include "spdlog/spdlog.h"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

CLIModeHandler::CLIModeHandler(MainWindow* win)
    : ModeHandler(win) {

    createKeymaps();
}

void CLIModeHandler::createKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        auto* server = win->getServer();
        QLocalSocket* client = server->getCurrentClient();
        if (!client) {
            return;
        }
        client->write(widgets[i]->getSearchPhrase().toLocal8Bit());
        client->flush();
        server->dropConnection();

        win->sleep();
    });
}

void CLIModeHandler::load() {
    auto* server = win->getServer();

    if (!server) {
        return;
    }

    QLocalSocket* client = server->getCurrentClient();

    if (!client) {
        return;
    }

    spdlog::info("CLI mode is reading");

    QByteArray data = client->readAll();
    auto str = QString::fromLocal8Bit(data);
    entries = str.split('\n', Qt::SkipEmptyParts);
}

void CLIModeHandler::freeWidgets() {
    widgets.clear();
    main_widget->deleteLater();
    main_widget = new QWidget(nullptr);
}

void CLIModeHandler::onModeExit() {
    win->getServer()->dropConnection();
}

void CLIModeHandler::invokeQuery(const QString& query_) {

    freeWidgets();

    auto results = filter(query_, entries);

    for (auto& entry : results) {
        widgets.push_back(new TextWidget(win, main_widget, entry));
    }

    win->processResults(widgets);
}

QString CLIModeHandler::getModeText() {
    return "Results";
}

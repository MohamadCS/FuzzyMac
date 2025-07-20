
#include "FuzzyMac/CLIModeHandler.hpp"
#include "FuzzyMac/Algorithms.hpp"

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
}

void CLIModeHandler::load() {
    if (loaded) {
        return;
    }

    loaded = true;

    entries = {};
    std::string line;

    while (std::getline(std::cin, line)) {
        entries.push_back(line.c_str());
        widgets.push_back(new TextWidget(win, main_widget, line.c_str()));
    }

    qDebug() << entries.size();
}

void CLIModeHandler::enterHandler() {
    if (win->getResultsNum() == 0) {
        return;
    }

    int i = std::max(win->getCurrentResultIdx(), 0);
    std::cout << widgets[i]->getValue().toStdString();
    exit(0);
}

void CLIModeHandler::invokeQuery(const QString& query_) {
    auto results = filter(win, query_, entries);

    ResultsVec res{};
    res.reserve(entries.size());

    for (auto& entry : results) {
        res.push_back(new TextWidget(win, main_widget, entry));
    }

    win->processResults(res);
}

void CLIModeHandler::handleQuickLook() {
}

QString CLIModeHandler::handleModeText() {
    return "Results";
}

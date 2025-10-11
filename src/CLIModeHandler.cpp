
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

        createKeymaps();
}

void CLIModeHandler::createKeymaps() {
    keymap.bind(QKeySequence(Qt::Key_Return), [this]() {
        if (win->getResultsNum() == 0) {
            return;
        }

        int i = std::max(win->getCurrentResultIdx(), 0);
        std::cout << widgets[i]->getSearchPhrase().toStdString();
        exit(0);
    });
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
}

void CLIModeHandler::freeWidgets() {
    widgets.clear();
    main_widget->deleteLater();
    main_widget = new QWidget(nullptr);
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

#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/MainWindow.hpp"

#include <QDrag>
#include <QMimeData>
#include <QtConcurrent>
#include <cstdlib>
#include <optional>
#include <wordexp.h>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QFileIconProvider>
#include <QGuiApplication>
#include <QLabel>

ModeHandler::ModeHandler(MainWindow* win)
    : win(win),
      main_widget(new QWidget(nullptr)) {
}

void ModeHandler::freeWidgets() {
}

QString ModeHandler::handleModeText() {
    return "empty";
}

std::optional<QIcon> ModeHandler::getIcon() const {
    return {};
}
void ModeHandler::handleCopy() {
}

void ModeHandler::handlePathCopy() {
}

void ModeHandler::handleDragAndDrop(QDrag* drag) const {
}

QString ModeHandler::getPrefix() const {
    return "";
}

InfoPanelContent* ModeHandler::getInfoPanelContent() const {
    return nullptr;
}

void ModeHandler::handleQuickLook() {
}

std::vector<FuzzyWidget*> ModeHandler::createMainModeWidgets() {
    return {};
}

ModeHandler::~ModeHandler() {
    delete main_widget;
};

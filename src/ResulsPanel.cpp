#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/ResultsPanel.hpp"

#include <QDrag>
#include <QMimeData>
#include <QString>
#include <QUrl>

void ResultsPanel::startDrag(Qt::DropActions supportedActions) {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    ModeHandler* mod_handler = win->getModeHandler();
    QDrag* drag = new QDrag(this);
    mod_handler->handleDragAndDrop(drag);
}

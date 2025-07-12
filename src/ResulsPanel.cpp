#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/ParseConfig.hpp"
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

void ResultsPanel::loadConfig() {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    auto& config = win->getConfig();
    setIconSize(QSize(40, 40));
    setStyleSheet(QString(R"(
                                    QListWidget {
                                        background: %4;
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        border: none;
                                        padding: 0px;
                                        font-size: 15px;
                                        font-family: %5;
                                    })")
                      .arg(get<std::string>(config, {"colors", "results_list", "selection_background"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "selection"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "text"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "background"}))
                      .arg(get<std::string>(config, {"font"})));
}

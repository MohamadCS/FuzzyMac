#include "FuzzyMac/ResultsPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModeHandler.hpp"

#include <QDrag>
#include <QMimeData>
#include <QString>
#include <QUrl>

ResultsPanel::ResultsPanel(QWidget* parent)
    : QListWidget(parent) {
    setDragEnabled(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setObjectName("ResultList");
}

void ResultsPanel::startDrag(Qt::DropActions supportedActions) {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    const ModeHandler* mode_handler = win->getAPI()->getCurrentModeHandler();
    QDrag* drag = new QDrag(this);
    mode_handler->handleDragAndDrop(drag);
}

void ResultsPanel::loadConfig() {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    auto* api = win->getAPI();
    auto& config = api->getConfigManager();
    setIconSize(QSize(45, 45));

    QPalette p = palette();
    p.setColor(QPalette::Text, "#575279");
    setPalette(p);
    setStyleSheet(QString(R"(
        QListWidget {
            selection-background-color : %1;
            selection-color : %2;
            color: %3;
            border-radius: 0px;
            margin: 0px;
            background: %4;
            padding: 0px;
            border : none;
            font-size: 15px;
            font-family: %5;
        }
        QScrollBar:vertical {
            border: none;
            background:  %4;
            margin: 0px;
            width: 12px;  
            padding: 2px;
            border-radius: 4px;
        }
          QListWidget::item {
                    border: 0px solid #ccc;
                    border-radius: 8px;      
                    font-weight : normal;
                    padding: 5px;
                    margin: 4px 8px;         
                    background-color: %4;
            }
            QListWidget::item:selected {
                background-color: %1;
                font-weight : bold;
                color: %3;
            }
            QScrollBar::handle:vertical {
                background: %6;  
                min-height: 20px; 
                border-radius: 4px;
            }
            QScrollBar::handle:vertical:hover {
                margin: 0px;
                background: %7;  
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                background: transparent;
                height: 0px;           
                border: none;
            }
            QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
                background: transparent;
            }
            )")
                      .arg(config.get<std::string>({"colors", "results_list", "selection_background"}))
                      .arg(config.get<std::string>({"colors", "results_list", "selection"}))
                      .arg(config.get<std::string>({"colors", "results_list", "text"}))
                      .arg(config.get<std::string>({"colors", "results_list", "background"}))
                      .arg(config.get<std::string>({"font"}))
                      .arg(config.get<std::string>({"colors", "results_list", "scrollbar_color"}))
                      .arg(config.get<std::string>({"colors", "results_list", "scrollbar_hold_color"})));
}

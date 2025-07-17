#include "FuzzyMac/ResultsPanel.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/ParseConfig.hpp"

#include <QDrag>
#include <QMimeData>
#include <QString>
#include <QUrl>

void ResultsPanel::startDrag(Qt::DropActions supportedActions) {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    const ModeHandler* mod_handler = win->getCurrentModeHandler();
    QDrag* drag = new QDrag(this);
    mod_handler->handleDragAndDrop(drag);
}

void ResultsPanel::loadConfig() {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    auto& config = win->getConfig();
    setIconSize(QSize(40, 40));

    QPalette p = palette();
    p.setColor(QPalette::Text, "#575279");
    setPalette(p);
    setStyleSheet(QString(R"(
        QListWidget {
            selection-background-color : %1;
            selection-color : %2;
            color: %3;
            background: %4;
            border: none;
            padding: 0px;
            font-size: 15px;
            font-weight: 300;
            font-family: %5;
        }
        QScrollBar:vertical {
            border: none;
            background: transparent;
            width: 12px;  
            padding: 2px;
            border-radius: 4px;
        }
          QListWidget::item {
                    border: 0px solid #ccc;
                    border-radius: 10px;      
                    margin: 4px 3px;         
                    background-color: %4;
            }
            QListWidget::item:selected {
                background-color: %1;
                color: %3;
            }
            QScrollBar::handle:vertical {
                background: %6;  
                min-height: 20px; 
                border-radius: 4px;
            }
            QScrollBar::handle:vertical:hover {
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
                      .arg(get<std::string>(config, {"colors", "results_list", "selection_background"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "selection"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "text"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "background"}))
                      .arg(get<std::string>(config, {"font"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "scrollbar_color"}))
                      .arg(get<std::string>(config, {"colors", "results_list", "scrollbar_hold_color"})));
}

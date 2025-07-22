#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/MainWindow.hpp"

#include <QApplication>

void QueryEdit::keyPressEvent(QKeyEvent* event) {

    if (QKeySequence(event->modifiers() | event->key()) == QKeySequence(Qt::ControlModifier | Qt::Key_Backspace)) {
        setText("");
        return;
    }

    if (QKeySequence(event->modifiers() | event->key()) == QKeySequence::Copy) {
        emit requestAppCopy();
        return;
    }

#ifndef CLI_TOOL
    if (text().isEmpty() && event->key() == Qt::Key_Backspace) {
        MainWindow* win = qobject_cast<MainWindow*>(window());
        win->changeMode(Mode::APP);
    }

#endif

    // Default behavior for all other keys
    QLineEdit::keyPressEvent(event);
}

void QueryEdit::loadConfig() {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    const auto& config = win->getConfigManager();
    setStyleSheet(QString(R"(
                                    QLineEdit {
                                        margin : 0px;
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        background: %4;
                                        border : none;
                                        padding: 12px;
                                        font-size: 30px;
                                        font-family: %5;
                                    })")
                      .arg(config.get<std::string>({"colors", "query_input", "selection_background"}))
                      .arg(config.get<std::string>({"colors", "query_input", "selection"}))
                      .arg(config.get<std::string>({"colors", "query_input", "text"}))
                      .arg(config.get<std::string>({"colors", "query_input", "background"}))
                      .arg(config.get<std::string>({"font"})));
}

#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ParseConfig.hpp"

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

    // Default behavior for all other keys
    QLineEdit::keyPressEvent(event);
}

void QueryEdit::loadConfig() {
    MainWindow* win = qobject_cast<MainWindow*>(window());
    const auto& config = win->getConfig();
    setStyleSheet(QString(R"(
                                    QLineEdit {
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        background: %4;
                                        border : none;
                                        padding: 12px;
                                        font-size: 30px;
                                        font-family: %5;
                                    })")
                      .arg(get<std::string>(config, {"colors", "query_input", "selection_background"}))
                      .arg(get<std::string>(config, {"colors", "query_input", "selection"}))
                      .arg(get<std::string>(config, {"colors", "query_input", "text"}))
                      .arg(get<std::string>(config, {"colors", "query_input", "background"}))
                      .arg(get<std::string>(config, {"font"})));
}

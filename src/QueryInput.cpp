#include "FuzzyMac/QueryInput.hpp"

#include <QApplication>
void QueryInput::keyPressEvent(QKeyEvent* event) {
    if (QKeySequence(event->modifiers() | event->key()) == QKeySequence::Copy) {
        emit requestAppCopy();
        return;
    }

    // Default behavior for all other keys
    QLineEdit::keyPressEvent(event);
}

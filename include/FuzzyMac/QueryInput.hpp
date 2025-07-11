#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class QueryInput : public QLineEdit {
    Q_OBJECT;

public:
    using QLineEdit::QLineEdit;

signals:
    void requestAppCopy();

private:
    void keyPressEvent(QKeyEvent* event) override;
};

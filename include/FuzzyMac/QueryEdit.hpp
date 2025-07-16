#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class QueryEdit : public QLineEdit {
    Q_OBJECT;

public:
    using QLineEdit::QLineEdit;

    void loadConfig();

signals:
    void requestAppCopy();
    void requestMainMode();

private:
    void keyPressEvent(QKeyEvent* event) override;
};

#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class QueryEdit : public QLineEdit {
    Q_OBJECT;

public:
    QueryEdit(QWidget* parent);
    void loadConfig();


signals:
    void requestAppCopy();
    void requestMainMode();

private:
    void keyPressEvent(QKeyEvent* event) override;
};

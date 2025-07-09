#pragma once

#include <QPainter>
#include <QStaticText>
#include <QWidget>

class ResultWidget : public QWidget {
public:
    ResultWidget(QWidget* parent, const QString& value);

protected:
    void paintEvent(QPaintEvent*);

private:
    QString value;
    QStaticText label;
};

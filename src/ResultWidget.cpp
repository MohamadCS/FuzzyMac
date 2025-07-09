#include "../include/FuzzyMac/ResultWidget.hpp"

ResultWidget::ResultWidget(QWidget* parent, const QString& value)
    : QWidget(parent),
      value(value) {
    label = QStaticText(value);
}

void ResultWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawStaticText(QPointF(0, 0), label);
}

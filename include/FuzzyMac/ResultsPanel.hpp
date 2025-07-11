#pragma once
#include <QListWidget>


class ResultsPanel : public QListWidget {
    Q_OBJECT
public:
    using QListWidget::QListWidget;

protected:
    void startDrag(Qt::DropActions supportedActions) override;
};

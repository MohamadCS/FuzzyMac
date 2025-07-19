#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include <QWidget>

class InfoPanelContent : public QWidget {
    Q_OBJECT;

public:
    using QWidget::QWidget;
    InfoPanelContent(QWidget* parent, MainWindow* win)
        : QWidget(parent),
          win(win) {
    }

protected:
    MainWindow* win;
};

class InfoPanel : public QWidget {
    Q_OBJECT;

public:
    using QWidget::QWidget;
    InfoPanel(QWidget* parent, MainWindow* win);

    void setContent(InfoPanelContent* content);

private:
    QHBoxLayout* layout;
    MainWindow* win;
    InfoPanelContent* content;
};

#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include <QWidget>

class InfoPanelContent : public QWidget {
    Q_OBJECT;

public:
    using QWidget::QWidget;
    InfoPanelContent(QWidget* parent, API* win)
        : QWidget(parent),
          api(win) {
    }

protected:
    API* api;
};

class InfoPanel : public QWidget {
    Q_OBJECT;

public:
    using QWidget::QWidget;
    InfoPanel(QWidget* parent, API* api);

    void setContent(InfoPanelContent* content);

private:
    QHBoxLayout* layout;
    API* api;
    InfoPanelContent* content;
};

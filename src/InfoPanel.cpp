#include "FuzzyMac/InfoPanel.hpp"

#include <QStyle>

InfoPanel::InfoPanel(QWidget* parent, MainWindow* win)
    : QWidget(parent),
      win(win),
      content(nullptr) {
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
}

void InfoPanel::setContent(InfoPanelContent* new_content) {

    if (content) {
        layout->removeWidget(content);
        content->deleteLater();
        content = nullptr;
    }

    if (!new_content) {
        return;
    }

    content = new_content;
    layout->addWidget(new_content);
}

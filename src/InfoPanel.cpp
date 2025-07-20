#include "FuzzyMac/InfoPanel.hpp"

#include <QStyle>

InfoPanel::InfoPanel(QWidget* parent, MainWindow* win)
    : QWidget(parent),
      win(win),
      content(nullptr) {
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

void InfoPanel::setContent(InfoPanelContent* new_content) {
    auto& cfg = win->getConfigManager();
    QString sheet = QString(R"(
    QWidget {
            color : %1;
            background-color: %2;
            border-left: 2px solid %3;
    }
    )")
                        .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                        .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                        .arg(cfg.get<std::string>({"colors", "inner_border_color"}));
    setStyleSheet(sheet);

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

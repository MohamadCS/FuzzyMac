
#include "FuzzyMac/ClipInfoPanel.hpp"
#include "FuzzyMac/ConfigManager.hpp"

#include <QPlainTextEdit>

static QString applyTextStyleSheet(const ConfigManager& cfg) {
    return QString(R"(
                                QPlainTextEdit {
                                    selection-background-color : %1;
                                    selection-color : %2;
                                    color: %3;
                                    background: %4;
                                    margin: 8px;
                                    border-radius: 8px;
                                    border : none;
                                    padding: 12px;
                                    font-size: 15;
                                    font-family: %5;
                                }
                                  QScrollBar:vertical {
                                            border: none;
                                            background: %4;
                                            width: 12px;  
                                            padding: 2px;
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:vertical {
                                            background: %6;  
                                            min-height: 20px; 
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:vertical:hover {
                                            background: %7;  
                                        }
                                        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                                            background: transparent;
                                            height: 0px;           
                                            border: none;
                                        }
                                        QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
                                            background: transparent;
                                        }

                                        QScrollBar:horizontal {
                                            border: none;
                                            background: %4;
                                            width: 12px;  
                                            padding: 2px;
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:horizontal {
                                            background: %6;  
                                            min-height: 20px; 
                                            border-radius: 4px;
                                        }
                                        QScrollBar::handle:horizontal:hover {
                                            background: %7;  
                                        }
                                        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                                            background: transparent;
                                            height: 0px;           
                                            border: none;
                                        }
                                        QScrollBar::up-arrow:horizontal, QScrollBar::down-arrow:horizontal {
                                            background: transparent;
                                        }
                                    )")
                             .arg(cfg.get<std::string>({"colors", "query_input", "selection_background"}))
                             .arg(cfg.get<std::string>({"colors", "query_input", "selection"}))
                             .arg(cfg.get<std::string>({"colors", "query_input", "text"}))
                             .arg(cfg.get<std::string>({"colors", "query_input", "background"}))
                             .arg(cfg.get<std::string>({"font"}))
                             .arg(cfg.get<std::string>({"colors", "results_list", "scrollbar_color"}))
                             .arg(cfg.get<std::string>({"colors", "results_list", "scrollbar_hold_color"}));
}

ClipboardInfoPanel::ClipboardInfoPanel(QWidget* parent, MainWindow* win, const ClipboardManager::Entry& entry)
    : InfoPanelContent(parent, win) {

    auto& cfg = win->getConfigManager();

    setAutoFillBackground(true);
    setStyleSheet(QString(R"(
            color : %1;
            background-color: %1;
            border-left: 2 solid %2;
    )")
                      .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                      .arg(cfg.get<std::string>({"colors", "inner_border"})));

    QString sheet = QString(R"(
            color : %1;
            background: %2;
            font-weight: 300;
            font-family: %3;
            font-size: 12px;
            padding: 5px;
            border: transparent;
    )")
                        .arg(cfg.get<std::string>({"colors", "mode_label", "text"}))
                        .arg(cfg.get<std::string>({"colors", "mode_label", "background"}))
                        .arg(cfg.get<std::string>({"font"}));

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(1, 0, 0, 0);

    // creating content area

    QPlainTextEdit* content_text = new QPlainTextEdit(this);

    content_text->setReadOnly(true);

    if (std::holds_alternative<QString>(entry.value)) {
        content_text->setPlainText(std::get<QString>(entry.value));
    } else {
        QString text{};
        for (const auto& url : std::get<QList<QUrl>>(entry.value)) {
            auto url_str = url.toString();
            text.append(url_str);
            text.append("\n");
        }

        content_text->setPlainText(text);
    }

    content_text->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    content_text->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    content_text->setStyleSheet(applyTextStyleSheet(win->getConfigManager()));
    layout->addWidget(content_text, 1);

    // creating info widgets
    std::vector<std::pair<QString, QString>> file_info{
        {"Source", entry.app},
        {"Time", entry.timestamp.toString()},
    };

    for (auto [name, data] : file_info) {
        auto* label_layout = new QHBoxLayout;
        label_layout->setSpacing(0);
        label_layout->setContentsMargins(0, 0, 0, 0);

        QLabel* name_label = new QLabel(name, this);
        QLabel* data_label = new QLabel(data, this);

        data_label->setWordWrap(true);
        data_label->setStyleSheet(sheet);
        data_label->setAlignment(Qt::AlignRight);

        name_label->setWordWrap(true);
        name_label->setStyleSheet(sheet);

        label_layout->addWidget(name_label);
        label_layout->addWidget(data_label, 1);

        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Plain);
        line->setStyleSheet(QString(R"( color : %1;)").arg(cfg.get<std::string>({"colors", "inner_border"})));

        layout->addWidget(line);
        layout->addLayout(label_layout);
    }

    setLayout(layout);
}

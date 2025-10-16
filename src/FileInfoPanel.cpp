#include "FuzzyMac/FileInfoPanel.hpp"
#include "FuzzyMac/Utils.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"

#include <QLabel>
#include <QFuture>
#include <QtConcurrent>


FileInfoPanel::FileInfoPanel(QWidget* parent, MainWindow* win, QString path)
    : InfoPanelContent(parent, win) {


    image_watcher = new QFutureWatcher<QImage>(this);

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

    QFileInfo info{path};

    if(!info.exists()) {
        return;
    }

    std::vector<std::pair<QString, QString>> file_info{
        {"File Name", info.fileName()},
        {"Path", info.path()},
        {"Size", convertToReadableFileSize(info.size())},
        {"Modified", info.lastModified().toString()},
    };

    auto* layout = new QVBoxLayout(this);

    auto* thumb = new QLabel(this);
    connect(image_watcher, &QFutureWatcher<QImage>::finished, [this, thumb]() {
        const auto& image = image_watcher->result();
        if (image.isNull()) {
            return;
        } else {
            thumb->setPixmap(QPixmap::fromImage(image));
        }
    });

    thumb->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    thumb->setStyleSheet(sheet);

    layout->setSpacing(0);
    layout->setContentsMargins(1, 0, 0, 0);
    layout->addWidget(thumb, 1);

    auto future = QtConcurrent::run([this, path]() -> QImage { return getThumbnailImage(path, 128, 128); });

    image_watcher->setFuture(future);

    for (int i = 0; i < file_info.size(); ++i) {
        auto [name, data] = file_info[i];
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

#include "FuzzyMac/ImageViewerInfoPanel.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/Utils.hpp"
#include <QImageReader>
#include <QLabel>
#include <QtConcurrent>

ImageViewerInfoPanel::ImageViewerInfoPanel(QWidget* parent, API* api, QString path)
    : InfoPanelContent(parent, api) {

    image_watcher = new QFutureWatcher<QPixmap>(this);

    auto& cfg = api->getConfigManager();

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

    auto* layout = new QVBoxLayout(this);
    auto* thumb = new QLabel(this);

    thumb->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    thumb->setStyleSheet(sheet);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(thumb);

    connect(image_watcher, &QFutureWatcher<QPixmap>::finished, [this, thumb]() {
        thumb->setPixmap(image_watcher->result());
    });

    auto future = QtConcurrent::run([this, path]() -> QPixmap {
        QImage img = getThumbnailImage(path, 224, 126);
        return QPixmap::fromImage(img.scaled(320, 180, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    });

    image_watcher->setFuture(future);
    setLayout(layout);
}

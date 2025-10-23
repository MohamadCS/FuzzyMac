#pragma once

#include <QPixmap>

#include "FuzzyMac/InfoPanel.hpp"

class ImageViewerInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    ImageViewerInfoPanel(QWidget* parent, API* api, QString path);

private:
    QFutureWatcher<QPixmap>* image_watcher;
};

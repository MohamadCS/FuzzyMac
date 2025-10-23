#pragma once

#include "FuzzyMac/InfoPanel.hpp"

class FileInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    FileInfoPanel(QWidget* parent, API* api, QString path);

private:
    QFutureWatcher<QImage>* image_watcher;
};

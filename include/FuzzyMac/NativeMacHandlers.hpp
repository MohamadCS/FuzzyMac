#pragma once

#include <string>
#include <QString>
#include <QImage>
#include <QPixmap>
#include <QTimer>

extern "C" void deactivateApp();

extern "C" void centerWindow(QWidget* widget);

extern "C" void makeWindowFloating(QWidget* widget);

extern "C" void disableCmdQ();

extern "C++" std::vector<std::string> spotlightSearch(const std::vector<std::string>& dirs, const std::string& query);

extern "C++" void quickLook(const std::string& filePath);

extern "C++" QImage getThumbnailImage(const QString& filePath, int width, int height);

extern "C++" int getClipboardCount();




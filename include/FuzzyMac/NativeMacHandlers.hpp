#pragma once

#include <QImage>
#include <QPixmap>
#include <QString>
#include <QTimer>
#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include <string>

extern "C" void deactivateApp();

extern "C" void centerWindow(QWidget* widget);

extern "C" void setupWindowDecoration(MainWindow *widget, ConfigManager*);


extern "C" void addMaterial(QWidget* widget);

extern "C" void disableCmdQ();

extern "C++" std::vector<std::string> spotlightSearch(const std::vector<std::string>& dirs, const std::string& query);

extern "C++" void quickLook(const std::string& filePath);

extern "C++" QImage getThumbnailImage(const QString& filePath, int width, int height);

extern "C++" int getClipboardCount();

extern "C++" bool authenticateWithTouchID();

extern "C++" std::string getFrontmostAppName();


extern "C++" void showQuickLookPanel(const QString &filePath);

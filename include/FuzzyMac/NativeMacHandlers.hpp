#pragma once

#include <QWidget>
#include <string>

extern "C" void deactivateApp();

extern "C" void centerWindow(QWidget* widget);

extern "C" void makeWindowFloating(QWidget* widget);

extern "C" void disableCmdQ();

extern "C++" std::vector<std::string> spotlightSearch(const std::vector<std::string>& dirs, const std::string& query);

extern "C++" void quickLock(const std::string& filePath);

extern "C" void closeQuickLook();

#pragma once

#include <string>
#include <vector>
#include <QWidget>

extern "C" void deactivateApp();
extern "C" void makeWindowFloating(QWidget *widget);

extern "C++" std::vector<std::string> spotlightSearch(const std::string& query,
                                                      const std::vector<std::string>& folders);

extern "C++" void quickLock(const std::string& filePath);

extern "C" void closeQuickLook();

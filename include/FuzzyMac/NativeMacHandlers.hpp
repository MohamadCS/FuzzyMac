#pragma once

#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QTimer>
#include <string>

struct BluetoothDevice {
    QString addr;
    QString name;
    bool is_connected;
};

extern "C" void deactivateApp();

extern "C" void centerWindow(QWidget* widget);

extern "C" void setupWindowDecoration(MainWindow* widget, ConfigManager*);

extern "C" void setupWindowSettings(MainWindow* widget);

extern "C++" QStringList spotlightSearch(const QStringList& dirs, const QString& arg);

extern "C" void addMaterial(QWidget* widget);

extern "C" void disableCmdQ();

extern "C++" QStringList spotlightSearch(const QStringList& dirs, const QString& arg);

extern "C++" void quickLook(const std::string& filePath);

extern "C++" QImage getThumbnailImage(const QString& filePath, int width, int height);

extern "C++" int getClipboardCount();

extern "C++" bool authenticateWithTouchID();

extern "C++" std::string getFrontmostAppName();

extern "C++" void showQuickLookPanel(const QString& filePath);

extern "C++" void connectToBTDevice(const QString& mac_addr, bool connect);

extern "C++" std::vector<BluetoothDevice> getPairedBluetoothDevices();


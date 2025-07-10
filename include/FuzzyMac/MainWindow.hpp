#pragma once

#include "FuzzyMac/ModHandler.hpp"
#include "toml++/toml.h"

#include <QFutureWatcher>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>

#include <memory>

namespace fs = std::filesystem;

enum class Mode {
    CLI,
    APP,
    FILE
};

struct MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(Mode mode = Mode::APP, QWidget* parent = nullptr);
    ~MainWindow();

    void wakeup();
    void sleep();

public slots:
    void nextItem();
    void prevItem();
    void openItem();
#ifndef CLI_TOOL
    void onApplicationStateChanged(Qt::ApplicationState state);
#endif

    const toml::table& getConfig() const;

private:
    QWidget* central;
    QLineEdit* query_input;
    QListWidget* results_list;
    QVBoxLayout* layout;
    QTimer* search_refresh_timer;
    QFutureWatcher<QStringList>* results_watcher;
    toml::table config;

    Mode mode;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;

    void processConfigFile();
    void createWidgets();
    void setupLayout();
    void setupStyles();
    void createKeybinds();
    void processStdIn();
    void fillData();
    void connectEventHandlers();
};

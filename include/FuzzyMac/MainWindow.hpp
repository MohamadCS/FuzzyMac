#pragma once

#include "FuzzyMac/ModHandler.hpp"

#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

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

public slots:
    void nextItem();
    void prevItem();
    void openItem();
    void onApplicationStateChanged(Qt::ApplicationState state);

private:
    QWidget* central;
    QLineEdit* query_input;
    QListWidget* results_list;
    QVBoxLayout* layout;
    QTimer* search_refresh_timer;

    Mode mode;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;

    void createWidgets();
    void setupLayout();
    void setupStyles();
    void createKeybinds();
    void processStdIn();
    void fillData();
    void connectEventHandlers();

    void customSearchDataFill(const QString& query);
    void spotlightSearchDataFill(const QString& query);
};

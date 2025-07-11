#pragma once

#include "toml++/toml.h"
#include "FuzzyMac/QueryInput.hpp"
#include "FuzzyMac/ResultsPanel.hpp"

#include <QFileIconProvider>
#include <QFileSystemWatcher>
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
#include <optional>

namespace fs = std::filesystem;

class ModeHandler;

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
    const toml::table& getConfig() const;
    void refreshResults();
    void addToResultList(const std::string& name, std::optional<fs::path> path = std::nullopt);
    QIcon getFileIcon(const std::string& path) const;
    QListWidgetItem* createListItem(const std::string& name, std::optional<fs::path> path = std::nullopt);
    void clearResultList();
    int getCurrentResultIdx() const;
    int resultsNum() const;
    ModeHandler* getModeHandler() const;
private slots:
    void nextItem();
    void prevItem();
    void copyToClipboard();
    void copyPathToClipboard();
    void openItem();
    void quickLock();
    void onTextChange(const QString& text);
#ifndef CLI_TOOL
    void onApplicationStateChanged(Qt::ApplicationState state);
#endif

protected:
private:
    QWidget* central;
    QueryInput* query_input;
    ResultsPanel* results_list;
    QVBoxLayout* layout;
    QFileIconProvider icon_provider;
    QFileSystemWatcher* config_file_watcher;
    QTimer* search_refresh_timer;
    QFutureWatcher<std::vector<QListWidgetItem*>>* results_watcher;
    toml::table config;

    Mode mode;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;

    void processConfigFile();
    void createWidgets();
    void setupLayout();
    void setupStyles();
    void createKeybinds();
    void loadConfig();
    void connectEventHandlers();
};

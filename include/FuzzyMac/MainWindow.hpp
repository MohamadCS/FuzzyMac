#pragma once

#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/ResultsPanel.hpp"
#include "toml++/toml.h"

#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainterPath>
#include <QProcess>
#include <QString>
#include <QVBoxLayout>

#include <memory>
#include <optional>
#include <variant>

namespace fs = std::filesystem;

class ModeHandler;
class FuzzyWidget;

using ResultsVec = std::vector<FuzzyWidget*>;

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

    void refreshResults();
    void addItemToResultsList(const std::string& name, std::optional<fs::path> path = std::nullopt);
    QListWidgetItem* createListItem(const std::string& name, std::optional<fs::path> path = std::nullopt);
    QListWidgetItem* createListItem(QWidget* widget);
    void clearResultList();

    bool isWidgetCurrentSelection(QWidget* widget) const;
    const toml::table& getConfig() const;
    std::string getQuery() const;
    QIcon getFileIcon(const std::string& path) const;
    int getCurrentResultIdx() const;
    int getResultsNum() const;
    void processResults(const ResultsVec&);
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
    QueryEdit* query_edit;
    QLabel* mode_label;
    ResultsPanel* results_list;
    QVBoxLayout* layout;
    QFileIconProvider icon_provider;
    QFileSystemWatcher* config_file_watcher;
    QFutureWatcher<ResultsVec>* results_watcher;
    toml::table config;

    Mode mode;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;

    void loadStyle();
    void createWidgets();
    void setupLayout();
    void createKeybinds();
    void loadConfig();
    void connectEventHandlers();
};

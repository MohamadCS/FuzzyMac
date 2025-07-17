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
class InfoPanel;
class InfoPanelContent;

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
    QListWidgetItem* createListItem(const QString& name, const std::optional<QIcon>& icon = std::nullopt);
    QListWidgetItem* createListItem(QWidget* widget);
    void clearResultList();

    void changeMode(Mode mode);

    bool isWidgetCurrentSelection(QWidget* widget) const;
    const toml::table& getConfig() const;
    QString getQuery() const;
    QIcon getFileIcon(const QString& path) const;
    QIcon createIcon(const QString& path, const QColor& color) const;
    void setInfoPanelContent(InfoPanelContent* content);
    int getCurrentResultIdx() const;
    int getResultsNum() const;
    void processResults(const ResultsVec&);
    const ModeHandler* getCurrentModeHandler() const;
    const ModeHandler* getModeHandler(Mode mode) const;

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
    InfoPanel* info_panel;
    QFileIconProvider icon_provider;
    QFileSystemWatcher* config_file_watcher;
    toml::table config;

    Mode mode;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;

    void loadStyle();
    void createWidgets();
    void setupLayout();
    void createKeybinds();
    void loadConfig();
    void connectEventHandlers();
    void matchModeShortcut(const QString&);
};

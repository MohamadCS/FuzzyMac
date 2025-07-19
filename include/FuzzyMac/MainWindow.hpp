#pragma once

#include "ConfigManager.hpp"
#include "FuzzyMac/ConfigManager.hpp"
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

class ModeHandler;
class ModeHandlerFactory;
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

    const ConfigManager& getConfigManager() const;
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
    // for layout
    QWidget* central;
    QVBoxLayout* layout;

    // main widgets, life time is managed by MainWindow
    QueryEdit* query_edit;
    QLabel* mode_label;
    ResultsPanel* results_list;
    InfoPanel* info_panel;

    // Helper QStructs, Life time is managed by MainWindow
    QFileIconProvider icon_provider;

    // Mode handling
    ModeHandlerFactory* mode_factory;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handler;
    Mode mode;

    ConfigManager* config_manager;

    void loadStyle();
    void createWidgets();
    void setupLayout();
    void createKeybinds();
    void loadConfig();
    void connectEventHandlers();
    void matchModeShortcut(const QString&);
};

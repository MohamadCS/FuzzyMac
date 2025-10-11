#pragma once

#include "ConfigManager.hpp"
#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/KeyMap.hpp"
#include "FuzzyMac/ResultsPanel.hpp"
#include "toml++/toml.h"

#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QGraphicsBlurEffect>
#include <QLineEdit>
#include <QLocalSocket>
#include <QListWidget>
#include <QLocalServer>
#include <QMainWindow>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QString>
#include <QTimer>
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
    CLI = 0,
    APP,
    FILE,
    CLIP,
    WALLPAPER,
    COUNT,
};

struct MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(Mode mode = Mode::APP, QWidget* parent = nullptr);
    ~MainWindow();


    void wakeup();
    void sleep();

    void refreshResults();
    void clearResultList();
    void selectItem(int item);
    QListWidgetItem* createListItem(const QString& name, const std::optional<QIcon>& icon = std::nullopt);
    QListWidgetItem* createListItem(QWidget* widget);
    int getCurrentResultIdx() const;
    int getResultsNum() const;
    void processResults(const ResultsVec&);

    void changeMode(Mode mode);
    void exitMode();

    void setInfoPanelContent(InfoPanelContent* content);
    void toggleInfoPanel();

    const ConfigManager& getConfigManager() const;
    QString getQuery() const;
    QIcon getFileIcon(const QString& path) const;
    QIcon createIcon(const QString& path, const QColor& color) const;
    void clearQuery();

    bool keymapDefined(QKeyEvent* ev) const;
    bool keymapOverides(QKeyEvent* ev) const;


    std::map<QString, QIcon> getIcons();

    const ModeHandler* getCurrentModeHandler() const;
    const ModeHandler* getModeHandler(Mode mode) const;
    std::vector<FuzzyWidget*> getModesWidgets() const;



    void keyPressEvent(QKeyEvent* ev) override;

private slots:
    void onTextChange(const QString& text);
    void onApplicationStateChanged(Qt::ApplicationState state);

    void onResultsListChanged();


private:
    // for layout
    QWidget* border_widget;
    QWidget* main_widget;
    QVBoxLayout* layout;
    Keymap keymap;

    // main widgets, life time is managed by MainWindow
    QueryEdit* query_edit;
    QLabel* mode_label;
    ResultsPanel* results_list;
    InfoPanel* info_panel;
    std::map<QString, QIcon> icons;

    // Helper QStructs, Life time is managed by MainWindow
    QFileIconProvider icon_provider;

    // Mode handling
    ModeHandlerFactory* mode_factory;
    std::map<Mode, std::unique_ptr<ModeHandler>> mode_handlers;
    Mode mode;

    ConfigManager* config_manager;
    bool show_info_panel;

    // loads the MainWindow Style
    void loadStyle();

    // creates all main widgets
    void createWidgets();
    // creates the application shortcuts
    void createKeybinds();

    void setupServer();

    // reloads the config, and the current mod
    void loadConfig();

    void connectEventHandlers();

    // finds the first mode that defines a shortcut
    // and switches to it
    void matchModeShortcut(const QString&);
};

#pragma once

#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/KeyMap.hpp"
#include "toml++/toml.h"

#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QGraphicsBlurEffect>
#include <QLineEdit>
#include <QListWidget>
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
class QueryEdit;
class ResultsPanel;

using ResultsVec = std::vector<FuzzyWidget*>;

enum class Mode {
    CLI = 0,
    APP,
    FILE,
    CLIP,
    WALLPAPER,
    COUNT,
};

struct API {
    // Lifecycle
    std::function<void()> wakeup;
    std::function<void()> sleep;

    // Results
    std::function<void()> refreshResults;
    std::function<void()> clearResultList;
    std::function<void(int)> selectItem;
    std::function<QListWidgetItem*(const QString&, const std::optional<QIcon>&)> createListItem;
    std::function<QListWidgetItem*(QWidget*)> createListItemWidget;
    std::function<void(QListView::ViewMode)> setResultsListView;
    std::function<int()> getCurrentResultIdx;
    std::function<int()> getResultsNum;
    std::function<void(const ResultsVec&)> processResults;

    // Mode handling
    std::function<void(Mode)> changeMode;
    std::function<void()> exitMode;

    // Info panel
    std::function<void(InfoPanelContent*)> setInfoPanelContent;
    std::function<void()> toggleInfoPanel;

    // Config / query
    std::function<const ConfigManager&()> getConfigManager;
    std::function<QString()> getQuery;
    std::function<void()> clearQuery;

    // Icons
    std::function<QIcon(const QString&)> getFileIcon;
    std::function<QIcon(const QString&, const QColor&)> createIcon;
    std::function<std::map<QString, QIcon>()> getIcons;

    // Keymap
    std::function<bool(QKeyEvent*)> keymapDefined;
    std::function<bool(QKeyEvent*)> keymapOverides;

    // Mode handlers / widgets
    std::function<const ModeHandler*()> getCurrentModeHandler;
    std::function<const ModeHandler*(Mode)> getModeHandler;
    std::function<std::vector<FuzzyWidget*>()> getModesWidgets;

    // Event handling
    std::function<void(QKeyEvent*)> keyPressEvent;
    std::function<void()> handleNewRequest;
};

struct MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(Mode mode = Mode::APP, QWidget* parent = nullptr);
    ~MainWindow();

    API* getAPI() const;

private slots:
    void onTextChange(const QString& text);
    void onApplicationStateChanged(Qt::ApplicationState state);
    void onResultsListChanged();

private:
    // for layout
    std::unique_ptr<API> api;
    QWidget* border_widget;
    QWidget* main_widget;
    QVBoxLayout* layout;
    Keymap keymap;

    // main widgets, life time is managed by MainWindow
    QueryEdit* query_edit;
    ResultsPanel* results_list;
    QLabel* mode_label;
    InfoPanel* info_panel;

    // icons
    std::map<QString, QIcon> icons;
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

    // reloads the config, and the current mod
    void loadConfig();

    void connectEventHandlers();

    // finds the first mode that defines a shortcut
    // and switches to it
    void matchModeShortcut(const QString&);

    // private API
    void wakeup();
    void sleep();

    void refreshResults();
    void clearResultList();
    void selectItem(int item);
    QListWidgetItem* createListItem(const QString& name, const std::optional<QIcon>& icon = std::nullopt);
    QListWidgetItem* createListItem(QWidget* widget);
    void setResultsListView(QListView::ViewMode view_mode);
    int getCurrentResultIdx() const;
    int getResultsNum() const;
    void processResults(const ResultsVec&);

    void changeMode(Mode mode);

    void setInfoPanelContent(InfoPanelContent* content);
    void toggleInfoPanel();

    const ConfigManager& getConfigManager() const;
    QString getQuery() const;
    QIcon getFileIcon(const QString& path) const;
    QIcon createIcon(const QString& path, const QColor& color) const;
    void clearQuery();

    std::map<QString, QIcon> getIcons();

    const ModeHandler* getCurrentModeHandler() const;
    const ModeHandler* getModeHandler(Mode mode) const;
    std::vector<FuzzyWidget*> getModesWidgets() const;

    void handleNewRequest();

    void keyPressEvent(QKeyEvent* ev) override;

    bool keymapDefined(QKeyEvent* ev) const;
    bool keymapOverides(QKeyEvent* ev) const;

    void createAPI();
};

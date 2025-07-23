#pragma once
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"

#include <QDrag>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QVBoxLayout>

#include <optional>
#include <toml++/toml.h>

class ModeHandler {
public:
    ModeHandler(MainWindow* win);
    virtual ~ModeHandler();

    // Called on startup and when config file changes
    virtual void load() = 0;

    // no use case yet
    virtual void unload() {};

    // called whenever the user pressed enter when the mode is activated
    virtual void enterHandler() = 0;

    // Prefix that the user types for to switch to the mode automatically
    virtual QString getPrefix() const;

    // cmd-y handle  
    virtual void handleQuickLook();

    virtual std::optional<QIcon> getIcon() const;

    // returns the current content to be displayed on info panel
    // or nullptr if the mode does not want to show content
    virtual InfoPanelContent* getInfoPanelContent() const;

    // The text that be shown below the search bar
    virtual QString handleModeText();

    // called when the text on searchbar changes
    virtual void invokeQuery(const QString& query_) = 0;

    virtual std::vector<FuzzyWidget*> createMainModeWidgets(); 

    virtual void handleCopy();

    virtual void handleDragAndDrop(QDrag*) const;

    virtual void handlePathCopy();

protected:
    MainWindow* win;
    QWidget* parent;
    QWidget* main_widget; // used for cleanup

    virtual void freeWidgets();
    QListWidgetItem* createListItem();
};

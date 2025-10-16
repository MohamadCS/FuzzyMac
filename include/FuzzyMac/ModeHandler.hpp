#pragma once
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/KeyMap.hpp"
#include "FuzzyMac/MainWindow.hpp"

#include <QDrag>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QJsonObject>
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
    //
    // Prefix that the user types for to switch to the mode automatically
    virtual QString getPrefix() const;

    virtual std::optional<QIcon> getIcon() const;

    // returns the current content to be displayed on info panel
    // or nullptr if the mode does not want to show content
    virtual InfoPanelContent* getInfoPanelContent() const;

    // The text that be shown below the search bar
    virtual QString getModeText();

    // Called when the text on searchbar changes
    virtual void invokeQuery(const QString& query_) = 0;

    virtual std::vector<FuzzyWidget*> createMainModeWidgets();

    virtual void onModeExit();

    virtual void handleDragAndDrop(QDrag*) const;

    const Keymap& getKeymap() const;

protected:
    MainWindow* win;
    QWidget* parent;
    QWidget* main_widget; // used for cleanup
    Keymap keymap;

    virtual void freeWidgets();
    QListWidgetItem* createListItem();
};

#pragma once
#include "FuzzyMac/FileInfoPanel.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ModeHandler.hpp"

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

class FileModeHandler : public ModeHandler {
public:
    FileModeHandler(MainWindow* win);

    ~FileModeHandler() override;

    // Called on startup and on config change.
    void load() override;

    // Returns the widget of the info panel.
    InfoPanelContent* getInfoPanelContent() const override;

    // Creates widgets based on current config and query.
    void invokeQuery(const QString& query_) override;

    // Creates special mode widgets to display on main mode.
    std::vector<FuzzyWidget*> createMainModeWidgets() override;

    // The prefix which triggers the mode from the main mode.
    QString getPrefix() const override;

    void handleDragAndDrop(QDrag*) const override;

    // Retruns the text to be displayed on mode label.
    QString getModeText() override;

    // Calls when the applications changed to another mode from this mode
    void onModeExit() override;

    // frees all widgets created by the mode until this point.
    void freeWidgets() override;

private:
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* fs_watcher;
    QStringList paths;
    QStringList entries;
    std::optional<QString> curr_path;

    void reloadEntries();
    bool isRelativeFileSearch() const;
    void setupKeymaps();
    void connectHandlers();
};

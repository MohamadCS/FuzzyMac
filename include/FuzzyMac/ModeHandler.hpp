#pragma once
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

    virtual void load() = 0;
    virtual void unload() {};
    virtual void enterHandler() = 0;
    virtual QString getPrefix() const;
    virtual void handleQuickLook() = 0;
    virtual std::optional<QIcon> getIcon() const;
    virtual InfoPanelContent* getInfoPanelContent() const;
    virtual void handleCopy();
    virtual void freeWidgets();
    virtual QString handleModeText();
    virtual void handleDragAndDrop(QDrag*) const;
    virtual void handlePathCopy();
    virtual void invokeQuery(const QString& query_) = 0;
    virtual ~ModeHandler();

protected:
    MainWindow* win;
    QWidget* parent;
    QWidget* main_widget; // used for cleanup

    QListWidgetItem* createListItem();
};

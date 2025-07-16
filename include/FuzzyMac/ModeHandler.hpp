#pragma once
#include "FuzzyMac/FuzzyWidget.hpp"
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
    virtual void handleCopy();
    virtual void freeWidgets();
    virtual QString handleModeText();
    virtual void handleDragAndDrop(QDrag*) const;
    virtual void handlePathCopy();
    virtual void invokeQuery(const QString& query_) = 0;
    virtual ~ModeHandler() {
        delete main_widget;
    };

protected:
    MainWindow* win;
    QWidget* parent;
    QWidget* main_widget; // used for cleanup

    QListWidgetItem* createListItem();
};

class AppModeHandler : public ModeHandler {

public:
    AppModeHandler(MainWindow* win);
    ~AppModeHandler() override;
    void load() override;
    QString handleModeText() override;

    void enterHandler() override;
    void handleQuickLook() override;
    void invokeQuery(const QString& query_) override;
    void freeWidgets() override;

private:
    bool math_mode = false;
    QStringList apps;
    std::map<QString, Mode> modes;
    QStringList app_dirs;
    std::vector<FuzzyWidget*> widgets;
    QFileSystemWatcher* app_watcher;
};

class CLIModeHandler : public ModeHandler {
public:
    CLIModeHandler(MainWindow* win);
    ~CLIModeHandler() override = default;
    void load() override;
    void enterHandler() override;
    void invokeQuery(const QString& query_) override;
    QString handleModeText() override;
    void handleQuickLook() override;

private:
    QStringList entries;
    std::vector<TextWidget*> widgets;
    bool loaded = false;
};

class FileModeHandler : public ModeHandler {
public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override;

    void load() override;
    void enterHandler() override;
    QString handleModeText() override;
    void handleCopy() override;
    void handleDragAndDrop(QDrag*) const override;
    void invokeQuery(const QString& query_) override;
    void handleQuickLook() override;
    std::optional<QIcon> getIcon() const override;
    QString getPrefix() const override;
    void freeWidgets() override;

private:
    QIcon icon;
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* dir_watcher;
    QStringList paths;
    QStringList entries;
    QMutex mutex;
};

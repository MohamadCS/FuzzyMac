#pragma once
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
#include <stack>
#include <toml++/toml.h>

class FileModeHandler : public ModeHandler {
public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override;

    void load() override;
    void enterHandler() override;
    QString handleModeText() override;
    void handleCopy() override;
    InfoPanelContent* getInfoPanelContent() const override;
    void handleDragAndDrop(QDrag*) const override;
    void invokeQuery(const QString& query_) override;
    void handleQuickLook() override;
    void handleComplete() override;
    bool handleBackspace() override;

    std::vector<FuzzyWidget*> createMainModeWidgets() override;
    QString getPrefix() const override;
    void freeWidgets() override;
    bool isRelativeFileSearch() const;
    void handleLeftBracket() override;

    void onModeExit() override;

private:
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* dir_watcher;
    QStringList paths;
    QStringList entries;
    std::optional<QString> curr_path;
    QMutex mutex;
};

class FileInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    FileInfoPanel(QWidget* parent, MainWindow* win, QString path);

private:
    QFutureWatcher<QImage>* image_watcher;
};

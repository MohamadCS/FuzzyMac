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
    QString handleModeText() override;
    InfoPanelContent* getInfoPanelContent() const override;
    void handleDragAndDrop(QDrag*) const override;
    void invokeQuery(const QString& query_) override;

    std::vector<FuzzyWidget*> createMainModeWidgets() override;
    QString getPrefix() const override;
    void freeWidgets() override;
    bool isRelativeFileSearch() const;


    void onModeExit() override;

private:
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<QStringList>* future_watcher;
    QFileSystemWatcher* dir_watcher;
    QStringList paths;
    QStringList entries;
    std::optional<QString> curr_path;
    QMutex mutex;

    void createKeyMaps();
};

class FileInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    FileInfoPanel(QWidget* parent, MainWindow* win, QString path);

private:

    QFutureWatcher<QImage>* image_watcher;
};

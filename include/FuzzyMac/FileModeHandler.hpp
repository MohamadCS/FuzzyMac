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
#include <toml++/toml.h>

class FileModeHandler : public ModeHandler {
public:
    FileModeHandler(MainWindow* win);
    ~FileModeHandler() override;

    void load() override;
    InfoPanelContent* getInfoPanelContent() const override;
    void invokeQuery(const QString& query_) override;

    std::vector<FuzzyWidget*> createMainModeWidgets() override;
    QString getPrefix() const override;

    // handlers
    void handleDragAndDrop(QDrag*) const override;
    QString handleModeText() override;
    void onModeExit() override;
    void createKeyMaps();
    void freeWidgets() override;

private:
    std::vector<FuzzyWidget*> widgets;
    QFutureWatcher<QStringList>* future_watcher;
    QStringList paths;
    QStringList entries;
    std::optional<QString> curr_path;

    bool isRelativeFileSearch() const;
};

class FileInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    FileInfoPanel(QWidget* parent, MainWindow* win, QString path);

private:
    QFutureWatcher<QImage>* image_watcher;
};

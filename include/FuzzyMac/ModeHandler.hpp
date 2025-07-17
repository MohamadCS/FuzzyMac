#pragma once
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/Utils.hpp"

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
    virtual InfoPanelContent* getInfoPanelContent() const {
        return nullptr;
    }

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

class FileInfoPanel : public InfoPanelContent {
    Q_OBJECT;

public:
    FileInfoPanel(QWidget* parent, MainWindow* win, QString path)
        : InfoPanelContent(parent, win) {


        QString sheet = QString(R"(
            color : %1;
            background: %2;
            font-weight: 300;
            font-family: %3;
            font-size: 12px;
            padding: 5px;
            border: red;
    )")
                            .arg(get<std::string>(win->getConfig(), {"colors", "mode_label", "text"}))
                            .arg(get<std::string>(win->getConfig(), {"colors", "mode_label", "background"}))
                            .arg(get<std::string>(win->getConfig(), {"font"}));

        QFileInfo info{path};

        std::vector<std::pair<QString, QString>> file_info{
            {"File Name", info.fileName()},
            {"Path", info.path()},
            {"Size", convertToReadableFileSize(info.size())},
            {"Modified", info.lastModified().toString()},
        };

        auto* layout = new QVBoxLayout(this);

        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        for (auto [name, data] : file_info) {
            auto* label_layout = new QHBoxLayout();
            label_layout->setSpacing(0);
            label_layout->setContentsMargins(0, 0, 0, 0);

            QLabel* name_label = new QLabel(name, this);
            QLabel* data_label = new QLabel(data, this);

            data_label->setWordWrap(true);
            data_label->setStyleSheet(sheet);

            name_label->setWordWrap(true);
            name_label->setStyleSheet(sheet);

            label_layout->addWidget(name_label);
            label_layout->addWidget(data_label);

            QFrame* line = new QFrame(this);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Plain);
            line->setStyleSheet(
                QString(R"( color : %1;)").arg(get<std::string>(win->getConfig(), {"colors", "inner_border_color"})));

            layout->addLayout(label_layout);
            layout->addWidget(line);
        }

        auto* place_holder = new QWidget(this);
        place_holder->setStyleSheet(sheet);
        layout->addWidget(place_holder, 1);
        setLayout(layout);

        setAutoFillBackground(true);
        setStyleSheet(QString(R"(
            color : %1;
            background-color: %2;
            border-left: 2 solid %3;
    )")
                          .arg(get<std::string>(win->getConfig(), {"colors", "mode_label", "background"}))
                          .arg(get<std::string>(win->getConfig(), {"colors", "mode_label", "background"}))
                          .arg(get<std::string>(win->getConfig(), {"colors", "inner_border_color"})));
    }

private:
    QLabel* path;
};

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

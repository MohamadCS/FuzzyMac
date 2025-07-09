#pragma once

#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QString>
#include <QVBoxLayout>

#include <string>

class ModeHandler {
public:
    virtual void enterHandler(QListWidget* results_list) = 0;
    virtual void fillData(QListWidget* results_list) = 0;
    virtual QStringList getResults(const QString& query_, QListWidget* results_list) = 0;
    virtual ~ModeHandler() = default;
};

class AppModeHandler : public ModeHandler {
    std::vector<std::string> apps;
    std::vector<int> results_indices;

public:
    ~AppModeHandler() override = default;
    void enterHandler(QListWidget* results_list) override;
    void fillData(QListWidget* results_list) override;
    QStringList getResults(const QString& query_, QListWidget* results_list) override;
};

class CLIModeHandler : public ModeHandler {
    std::vector<std::string> entries;
    std::vector<int> results_indices;

public:
    ~CLIModeHandler() override = default;
    void enterHandler(QListWidget* results_list) override;
    void fillData(QListWidget* results_list) override;
    QStringList getResults(const QString& query_, QListWidget* results_list) override;
};

class FileModeHandler : public ModeHandler {
    std::vector<std::string> abs_results;

public:
    ~FileModeHandler() override = default;
    void enterHandler(QListWidget* results_list) override;
    void fillData(QListWidget* results_list) override;
    QStringList getResults(const QString& query_, QListWidget* results_list) override;
};

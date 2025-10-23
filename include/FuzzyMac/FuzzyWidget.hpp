#pragma once

#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"

#include <QLabel>
#include <QWidget>
#include <functional>
#include <variant>

class FuzzyWidget : public QWidget {
    Q_OBJECT;

protected:
    API* api;

public:
    FuzzyWidget(QWidget* parent, API* api);
    virtual ~FuzzyWidget() = default;
    virtual std::variant<QListWidgetItem*, FuzzyWidget*> getItem() = 0;
    virtual QString getSearchPhrase() const {
        return "";
    }
    virtual void enterHandler() {};
};

class TextWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    TextWidget(QWidget* parent, API* api, const QString& value);

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    QString getSearchPhrase() const override;

private:
    QString path;
    QLabel* text;
};

class CLIWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    CLIWidget(QWidget* parent, API* api, const QString& display_value, const QString& value);

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    QString getSearchPhrase() const override;

private:
    QString value;
    QLabel* text;
};

class FileWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    FileWidget(QWidget* parent, API* api, const QString& path, bool show_icon);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    QString getPath() const;
    ~FileWidget() {
    }

private:
    QString path;
    bool show_icon;
};

class ModeWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    ModeWidget(QWidget* parent, API* api, const QString& value, Mode mode, std::function<void()> enter_handler,
               const std::optional<QIcon>& icon = std::nullopt);
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    void enterHandler() override;

    QString getSearchPhrase() const override {
        return name;
    }

private:
    QString name;
    std::optional<QIcon> icon;
    std::function<void()> customeEnterHandler;
    Mode mode;
};

class CalculatorWidget : public FuzzyWidget {
    Q_OBJECT;

    void enterHandler() override;

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;

public:
    CalculatorWidget(QWidget* parent, API* api);
    QLabel* title_label;
    QLabel* answer_label;
};

class BluetoothDeviceWidget : public FuzzyWidget {
    Q_OBJECT;
    BluetoothDevice device;

public:
    BluetoothDeviceWidget(QWidget* parent, API* api, const BluetoothDevice& device);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
};

class ActionWidget : public FuzzyWidget {
    Q_OBJECT;

    QString desc;
    QString script_path;

public:
    ActionWidget(QWidget* parent, API* api, const QString& desc, const QString& script_path);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
};

class ImageWidget : public FuzzyWidget {

    Q_OBJECT;
    QString path;
    QFutureWatcher<QPixmap>* img_watcher;

public:
    ImageWidget(QWidget* parent, API* api, const QString& path);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    QString getPath() const;
};

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
    MainWindow* win;

public:
    FuzzyWidget(MainWindow* win, QWidget* parent);
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
    TextWidget(MainWindow* win, QWidget* parent, const QString& value);

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    QString getSearchPhrase() const override;

private:
    QLabel* text;
};

class FileWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    FileWidget(MainWindow* win, QWidget* parent, const QString& path, bool show_icon);
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
    ModeWidget(MainWindow* win, QWidget* parent, const QString& value, Mode mode, std::function<void()> enter_handler,
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
    CalculatorWidget(MainWindow* win, QWidget* parent);
    QLabel* title_label;
    QLabel* answer_label;
};

class BluetoothDeviceWidget : public FuzzyWidget {
    Q_OBJECT;
    BluetoothDevice device;
public:
    BluetoothDeviceWidget(MainWindow* win, QWidget* parent, const BluetoothDevice& device);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
};

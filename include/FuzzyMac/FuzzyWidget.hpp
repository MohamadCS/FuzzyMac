#pragma once
#include "FuzzyMac/MainWindow.hpp"

#include <QLabel>
#include <QWidget>
#include <variant>

class FuzzyWidget : public QWidget {
    Q_OBJECT;

protected:
    MainWindow* win;

public:
    FuzzyWidget(MainWindow* win, QWidget* parent);
    virtual ~FuzzyWidget() = default;
    virtual std::variant<QListWidgetItem*, FuzzyWidget*> getItem() {
        return {};
    };
    virtual void enterHandler() {};
};

class TextWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    TextWidget(MainWindow* win, QWidget* parent, const QString& value);
    QString getValue() const;

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
    ModeWidget(MainWindow* win, QWidget* parent, const QString& value, Mode mode,
               const std::optional<QIcon>& icon = std::nullopt);
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    void enterHandler() override;

private:
    QString name;
    std::optional<QIcon> icon;
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

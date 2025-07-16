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
    using QWidget::QWidget;
    FuzzyWidget(MainWindow* win);
    virtual ~FuzzyWidget() = default;
    virtual std::variant<QListWidgetItem*, FuzzyWidget*> getItem() {
        return {};
    };
    virtual void enterHandler() {};
};

class TextWidget : public FuzzyWidget {
    Q_OBJECT;

public:
    TextWidget(MainWindow* win, const std::string& value);
    std::string getValue() const;

private:
    QLabel* text;
};

class FileWidget : public FuzzyWidget {
public:
    FileWidget(MainWindow* win, const std::string& path, bool show_icon);
    void enterHandler() override;
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    std::string getPath() const;
    ~FileWidget() {
    }

private:
    std::string path;
    bool show_icon;
};

class ModeWidget : public FuzzyWidget {
public:
    ModeWidget(MainWindow* win, const std::string& value, Mode mode,const std::optional<QIcon>& icon = std::nullopt);
    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;
    void enterHandler() override;

private:
    std::string name;
    std::optional<QIcon> icon;
    Mode mode;
};

class CalculatorWidget : public FuzzyWidget {
    Q_OBJECT;

    void enterHandler() override;

    std::variant<QListWidgetItem*, FuzzyWidget*> getItem() override;

public:
    CalculatorWidget(MainWindow* win);
    MainWindow* win;
    QLabel* title_label;
    QLabel* answer_label;
};

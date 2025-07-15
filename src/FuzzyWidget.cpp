#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/ParseConfig.hpp"

#include <QApplication>
#include <QClipboard>
#include <unistd.h>
#include <variant>

FuzzyWidget::FuzzyWidget(MainWindow* win)
    : QWidget(win),
      win(win) {
}

TextWidget::TextWidget(MainWindow* win, const std::string& value)
    : FuzzyWidget(win) {
    text = new QLabel(value.c_str());
}

std::string TextWidget::getValue() const {
    return text->text().toStdString();
}

std::string FileWidget::getPath() const {
    return path;
}

void FileWidget::enterHandler() {
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << QString::fromStdString(path);
    process->start("open", args);
    win->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> FileWidget::getItem() {
    if (show_icon) {
        return win->createListItem(fs::path(path).filename(), path);
    } else {
        return win->createListItem(fs::path(path).filename());
    }
}

FileWidget::FileWidget(MainWindow* win, const std::string& path, bool show_icon)
    : FuzzyWidget(win),
      path(path),
      show_icon(show_icon) {
}

CalculatorWidget::CalculatorWidget(MainWindow* win)
    : FuzzyWidget(win) {
    title_label = new QLabel(this);
    answer_label = new QLabel(this);
    const auto& config = win->getConfig();
    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    title_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 20px;
        }
    )")
                                   .arg(get<std::string>(config, {"colors", "mode_label", "text"}))
                                   .arg(get<std::string>(config, {"font"})));

    answer_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 30px;
        }
    )")
                                    .arg(get<std::string>(config, {"colors", "mode_label", "text"}))
                                    .arg(get<std::string>(config, {"font"})));

    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    answer_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    title_label->setText("Result");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(title_label);
    layout->addWidget(answer_label);
}

std::variant<QListWidgetItem*, FuzzyWidget*> CalculatorWidget::getItem() {
    return this;
}

void CalculatorWidget::enterHandler() {
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(answer_label->text());
    win->sleep();
};

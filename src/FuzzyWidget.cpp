#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "spdlog/spdlog.h"

#include <QApplication>
#include <QClipboard>
#include <QtConcurrent>

#include <regex>
#include <unistd.h>
#include <variant>

FuzzyWidget::FuzzyWidget(MainWindow* win, QWidget* parent)
    : QWidget(parent),
      win(win) {
}

TextWidget::TextWidget(MainWindow* win, QWidget* parent, const QString& value, const QString& format)
    : FuzzyWidget(win, parent),
      format(format) {
    text = new QLabel(value);
}

QString TextWidget::getSearchPhrase() const {
    return text->text();
}

std::string formatRegex(const std::string& entry, const std::string& user_regex) {
    try {
        std::regex re(user_regex);
        std::smatch match;

        if (std::regex_search(entry, match, re)) {
            // If at least one capture group exists, use the first one
            if (match.size() > 1 && !match[1].str().empty())
                return match[1].str();
            else
                return match[0].str(); // fallback to the whole match
        }
        return entry; // no match → keep original entry
    } catch (const std::regex_error& e) {
        return entry; // fallback
    }
}

std::variant<QListWidgetItem*, FuzzyWidget*> TextWidget::getItem() {
    if (format.isEmpty()) {
        return win->createListItem(text->text());
    } else {
        return win->createListItem(formatRegex(text->text().toStdString(), format.toStdString()).c_str());
    }
}

QString FileWidget::getPath() const {
    return path;
}

void FileWidget::enterHandler() {
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << path;
    process->start("open", args);
    win->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> FileWidget::getItem() {
    if (show_icon) {
        return win->createListItem(QFileInfo(path).fileName(), win->getFileIcon(path));
    } else {
        return win->createListItem(QFileInfo(path).fileName());
    }
}

FileWidget::FileWidget(MainWindow* win, QWidget* parent, const QString& path, bool show_icon)
    : FuzzyWidget(win, parent),
      path(path),
      show_icon(show_icon) {
}

CalculatorWidget::CalculatorWidget(MainWindow* win, QWidget* parent)
    : FuzzyWidget(win, parent) {
    title_label = new QLabel(this);
    answer_label = new QLabel(this);
    const auto& config = win->getConfigManager();
    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);
    title_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 20px;
        }
    )")
                                   .arg(config.get<std::string>({"colors", "mode_label", "text"}))
                                   .arg(config.get<std::string>({"font"})));

    answer_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            font-weight: 500;
            font-family: %2;
            font-size: 30px;
        }
    )")
                                    .arg(config.get<std::string>({"colors", "mode_label", "text"}))
                                    .arg(config.get<std::string>({"font"})));

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

ModeWidget::ModeWidget(MainWindow* win, QWidget* parent, const QString& value, Mode mode, std::function<void()> fn,
                       const std::optional<QIcon>& icon)
    : FuzzyWidget(win, parent),
      name(value),
      mode(mode),
      icon(icon),
      customeEnterHandler{std::move(fn)} {
}
std::variant<QListWidgetItem*, FuzzyWidget*> ModeWidget::getItem() {
    if (icon) {
        return win->createListItem(name, icon.value());
    } else {
        return win->createListItem(name);
    }
}

void ModeWidget::enterHandler() {
    customeEnterHandler();
}

BluetoothDeviceWidget::BluetoothDeviceWidget(MainWindow* win, QWidget* parent, const BluetoothDevice& device)
    : FuzzyWidget(win, parent),
      device(device) {
}

std::variant<QListWidgetItem*, FuzzyWidget*> BluetoothDeviceWidget::getItem() {
    QString conn_prefix = device.is_connected ? "Disconnect from " : "Connect to ";
    return win->createListItem(conn_prefix + device.name, win->getIcons().at("bluetooth"));
}

void BluetoothDeviceWidget::enterHandler() {
    auto x = QtConcurrent::run([this]() { connectToBTDevice(device.addr, !device.is_connected); });
    win->sleep();
}

ActionWidget::ActionWidget(MainWindow* win, QWidget* parent, const QString& desc, const QString& script_path)
    : FuzzyWidget(win, parent),
      desc(desc),
      script_path(script_path) {
}

void ActionWidget::enterHandler() {
    QProcess::startDetached(script_path);
    win->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> ActionWidget::getItem() {
    return win->createListItem(desc, win->getIcons().at("settings"));
}

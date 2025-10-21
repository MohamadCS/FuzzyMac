#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "spdlog/spdlog.h"

#include <QApplication>
#include <QClipboard>
#include <QtConcurrent>

#include <unistd.h>
#include <variant>

CLIWidget::CLIWidget(MainWindow* win, QWidget* parent, const QString& display_value, const QString& value)
    : FuzzyWidget(win, parent),
      value(value) {
    text = new QLabel(display_value);
}

QString CLIWidget::getSearchPhrase() const {
    return value;
}

std::variant<QListWidgetItem*, FuzzyWidget*> CLIWidget::getItem() {
    return win->createListItem(text->text());
}

FuzzyWidget::FuzzyWidget(MainWindow* win, QWidget* parent)
    : QWidget(parent),
      win(win) {
}

TextWidget::TextWidget(MainWindow* win, QWidget* parent, const QString& value)
    : FuzzyWidget(win, parent) {
    text = new QLabel(value);
}

QString TextWidget::getSearchPhrase() const {
    return text->text();
}

std::variant<QListWidgetItem*, FuzzyWidget*> TextWidget::getItem() {
    return win->createListItem(text->text());
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
        QWidget {
            background: %1;
        }
    )")
                                   .arg(config.get<std::string>({"colors", "mode_label", "background"})));

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
    title_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            margin : 0px;
            font-weight: 500;
            font-family: %2;
            font-size: 20px;
        }
    )")
                                   .arg(config.get<std::string>({"colors", "mode_label", "text"}))
                                   .arg(config.get<std::string>({"font"})));
    title_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            margin : 0px;
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
            margin : 0px;
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

ImageWidget::ImageWidget(MainWindow* win, QWidget* parent, const QString& path)
    : FuzzyWidget(win, parent),
      path(path) {

    auto& cfg = win->getConfigManager();
    auto* layout = new QVBoxLayout;
    auto* img_label = new QLabel(this);
    img_watcher = new QFutureWatcher<QPixmap>(this);

    img_label->setScaledContents(true);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(img_label);
    img_label->setFixedSize(224 ,126);

    QObject::connect(img_watcher, &QFutureWatcher<QPixmap>::finished, [this, img_label]() {
        img_label->setPixmap(img_watcher->result());
    });

    auto future = QtConcurrent::run([this, path]() -> QPixmap {
        QImage img = getThumbnailImage(path, 224, 126);
        return QPixmap::fromImage(img);
    });

    img_watcher->setFuture(future);
    setLayout(layout);
}

void ImageWidget::enterHandler() {
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << path;
    process->start("open", args);
    win->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> ImageWidget::getItem() {
    return this;
}

QString ImageWidget::getPath() const {
    return path;
}

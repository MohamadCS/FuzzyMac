#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "spdlog/spdlog.h"

#include <QApplication>
#include <QClipboard>
#include <QtConcurrent>

#include <unistd.h>
#include <variant>

CLIWidget::CLIWidget(QWidget* parent, API* api, const QString& display_value, const QString& value)
    : FuzzyWidget(parent, api),
      value(value) {
    text = new QLabel(display_value);
}

QString CLIWidget::getSearchPhrase() const {
    return value;
}

std::variant<QListWidgetItem*, FuzzyWidget*> CLIWidget::getItem() {
    return api->createListItem(text->text(), std::nullopt);
}

FuzzyWidget::FuzzyWidget(QWidget* parent, API* api)
    : QWidget(parent),
      api(api) {
}

TextWidget::TextWidget(QWidget* parent, API* api, const QString& value)
    : FuzzyWidget(parent, api) {
    text = new QLabel(value);
}

QString TextWidget::getSearchPhrase() const {
    return text->text();
}

std::variant<QListWidgetItem*, FuzzyWidget*> TextWidget::getItem() {
    return api->createListItem(text->text(), std::nullopt);
}

QString FileWidget::getPath() const {
    return path;
}

void FileWidget::enterHandler() {
    QProcess* process = new QProcess(nullptr);
    QStringList args;
    args << path;
    process->start("open", args);
    api->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> FileWidget::getItem() {
    if (show_icon) {
        return api->createListItem(QFileInfo(path).fileName(), api->getFileIcon(path));
    } else {
        return api->createListItem(QFileInfo(path).fileName(), std::nullopt);
    }
}

FileWidget::FileWidget(QWidget* parent, API* api, const QString& path, bool show_icon)
    : FuzzyWidget(parent, api),
      path(path),
      show_icon(show_icon) {
}

CalculatorWidget::CalculatorWidget(QWidget* parent, API* api)
    : FuzzyWidget(parent, api) {
    title_label = new QLabel(this);
    answer_label = new QLabel(this);
    const auto& config = api->getConfigManager();
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
    api->sleep();
};

ModeWidget::ModeWidget(QWidget* parent, API* api, const QString& value, Mode mode, std::function<void()> fn,
                       const std::optional<QIcon>& icon)
    : FuzzyWidget(parent, api),
      name(value),
      mode(mode),
      icon(icon),
      customeEnterHandler{std::move(fn)} {
}
std::variant<QListWidgetItem*, FuzzyWidget*> ModeWidget::getItem() {
    if (icon) {
        return api->createListItem(name, icon.value());
    } else {
        return api->createListItem(name, std::nullopt);
    }
}

void ModeWidget::enterHandler() {
    customeEnterHandler();
}

BluetoothDeviceWidget::BluetoothDeviceWidget(QWidget* parent, API* api, const BluetoothDevice& device)
    : FuzzyWidget(parent, api),
      device(device) {
}

std::variant<QListWidgetItem*, FuzzyWidget*> BluetoothDeviceWidget::getItem() {
    QString conn_prefix = device.is_connected ? "Disconnect from " : "Connect to ";
    return api->createListItem(conn_prefix + device.name, api->getIcons().at("bluetooth"));
}

void BluetoothDeviceWidget::enterHandler() {
    auto x = QtConcurrent::run([this]() { connectToBTDevice(device.addr, !device.is_connected); });
    api->sleep();
}

ActionWidget::ActionWidget(QWidget* parent, API* api, const QString& desc, const QString& script_path)
    : FuzzyWidget(parent, api),
      desc(desc),
      script_path(script_path) {
}

void ActionWidget::enterHandler() {
    QProcess::startDetached(script_path);
    api->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> ActionWidget::getItem() {
    return api->createListItem(desc, api->getIcons().at("settings"));
}

ImageWidget::ImageWidget(QWidget* parent, API* api, const QString& path)
    : FuzzyWidget(parent, api),
      path(path) {

    auto& cfg = api->getConfigManager();
    auto* layout = new QVBoxLayout;
    auto* img_label = new QLabel(this);
    img_watcher = new QFutureWatcher<QPixmap>(this);

    img_label->setScaledContents(true);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(img_label);
    img_label->setFixedSize(224, 126);

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
    api->sleep();
}

std::variant<QListWidgetItem*, FuzzyWidget*> ImageWidget::getItem() {
    return this;
}

QString ImageWidget::getPath() const {
    return path;
}

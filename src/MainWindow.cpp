#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/Animations.hpp"
#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/GlobalShortcut.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/ModeHandlerFactory.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/ResultsPanel.hpp"
#include "FuzzyMac/Server.hpp"

#include "spdlog/spdlog.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QEvent>
#include <QFont>
#include <QGraphicsBlurEffect>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmapCache>
#include <QProcess>
#include <QShortcut>
#include <QStaticText>
#include <QWindow>
#include <QtConcurrent>
#include <algorithm>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>

const ConfigManager& MainWindow::getConfigManager() const {
    return *config_manager;
}

void MainWindow::createWidgets() {
    border_widget = new QWidget(this);
    main_widget = new QWidget(border_widget);

    layout = new QVBoxLayout(main_widget);

    query_edit = new QueryEdit(main_widget);
    results_list = new ResultsPanel(main_widget);
    mode_label = new QLabel(main_widget);
    info_panel = new InfoPanel(main_widget, this);

    // MainWindow settings
    QApplication::setQuitOnLastWindowClosed(false);
    setWindowFlag(Qt::WindowStaysOnTopHint);
    resize(700, 500);

    setAttribute(Qt::WA_TranslucentBackground); // Make background transparent
    setupWindowSettings(this);
    setupWindowDecoration(this, config_manager);

    centerWindow(this);
    setWindowOpacity(1);

    QVBoxLayout* border_layout = new QVBoxLayout(border_widget);
    border_widget->setLayout(border_layout);
    border_layout->addWidget(main_widget);
    if (!show_info_panel) {
        info_panel->hide();
    }

    QHBoxLayout* content_layout = new QHBoxLayout;

    // main layout widgets
    layout->addWidget(query_edit, 0);
    layout->addWidget(mode_label, 0);

    content_layout->addWidget(results_list, 5);
    content_layout->addWidget(info_panel, 8);
    content_layout->setSpacing(0);

    // main layout settings
    layout->addLayout(content_layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSpacing(0);

    mode_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    main_widget->setLayout(layout);
    setCentralWidget(border_widget);
    wakeup();
}

void MainWindow::selectItem(int idx) {
    int eff_idx = std::clamp(idx, 0, results_list->count() - 1);
    int curr_row = results_list->currentRow();

    if (eff_idx != curr_row) {
        results_list->setCurrentRow(eff_idx);
    }

    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

void MainWindow::wakeup() {

    if (config_manager->get<bool>({"animations"})) {
        opacityAnimator(this, 0.0, config_manager->get<float>({"opacity"}), 50);
    }

    show();
    raise();
    activateWindow();
    centerWindow(this);
    query_edit->setFocus();
    query_edit->selectAll();
}

void MainWindow::sleep() {

    auto sleep_ = [this]() {
        deactivateApp();
        hide();
        query_edit->clear();
        changeMode(Mode::APP);
        refreshResults();
    };

    if (config_manager->get<bool>({"animations"})) {
        auto* anim = opacityAnimator(this, config_manager->get<float>({"opacity"}), 0.0, 50);
        connect(anim, &QPropertyAnimation::finished, this, sleep_);
    } else {
        sleep_();
    }
}

void MainWindow::matchModeShortcut(const QString& text) {
    if (mode != Mode::APP) {
        return;
    }

    for (auto& [mode, handler] : mode_handlers) {
        auto prefix = handler->getPrefix();
        if (!prefix.isEmpty() && prefix == text) {
            changeMode(mode);
            return;
        }
    }
}

void MainWindow::onTextChange(const QString& text) {
    // try to see if the current text is a prefix defined by some mode
    matchModeShortcut(text);

    // let the mode handle the query
    mode_handlers[mode]->invokeQuery(query_edit->text());

    // replace the mode label text if possible
    if (const auto& mode_text = mode_handlers[mode]->getModeText(); !mode_text.isEmpty()) {
        mode_label->setText(QString("%1").arg(mode_text));
        mode_label->setVisible(true);
    } else {
        mode_label->setVisible(false);
    }
}

void MainWindow::processResults(const ResultsVec& results) {
    results_list->clear();

    for (auto obj : results) {
        auto item = obj->getItem();

        // Create list items
        if (std::holds_alternative<QListWidgetItem*>(item)) {
            results_list->addItem(std::get<QListWidgetItem*>(item));
        } else {
            auto* obj = std::get<FuzzyWidget*>(item);
            auto* item = createListItem(obj);
            results_list->setItemWidget(item, obj);
        }
    }

    if (results_list->count()) {
        // try to select the first entry
        results_list->setCurrentRow(0);
    }

    // update info panel;
    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

void MainWindow::onResultsListChanged() {
}

void MainWindow::connectEventHandlers() {

    connect(query_edit, &QueryEdit::textChanged, this, &MainWindow::onTextChange);

    connect(
        window()->windowHandle(), &QWindow::screenChanged, this, [this](QScreen* newScreen) { centerWindow(this); });
    connect(results_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (show_info_panel) {
            setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
        }
    });

    connect(config_manager, &ConfigManager::configChange, this, [this]() { loadConfig(); });
    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
}

void MainWindow::toggleInfoPanel() {
    show_info_panel = !show_info_panel;
    info_panel->setHidden(!show_info_panel);

    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

bool MainWindow::keymapDefined(QKeyEvent* ev) const {
    return mode_handlers.at(mode)->getKeymap().defined(ev) || keymap.defined(ev);
}

bool MainWindow::keymapOverides(QKeyEvent* ev) const {
    return mode_handlers.at(mode)->getKeymap().doesOveride(ev) || keymap.doesOveride(ev);
}

void MainWindow::keyPressEvent(QKeyEvent* ev) {
    if (!mode_handlers[mode]->getKeymap().trigger(ev) && !keymap.trigger(ev)) {
    }
}

void MainWindow::createKeybinds() {
    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_I), [this]() { toggleInfoPanel(); });

    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_N), [this]() { selectItem(results_list->currentRow() + 1); });
    keymap.bind(QKeySequence(Qt::MetaModifier | Qt::Key_P), [this]() { selectItem(results_list->currentRow() - 1); });
    keymap.bind(
        QKeySequence(Qt::Key_Backspace),
        [this]() {
            if (getQuery().isEmpty())
                changeMode(Mode::APP);
        },
        false);

    keymap.bind(Qt::Key_Escape, [this]() { this->sleep(); });

    auto* cmd_space = new GlobalHotkeyBridge(this);
    connect(cmd_space, &GlobalHotkeyBridge::activated, this, [this] {
        if (isHidden()) { // HIDE
            wakeup();
        } else {
            sleep();
        }
    });

    cmd_space->registerHotkey(QKeySequence(Qt::MetaModifier | Qt::Key_Space));

    auto* cmd_shift_c = new GlobalHotkeyBridge(this);
    connect(cmd_shift_c, &GlobalHotkeyBridge::activated, this, [this] {
        wakeup();
        changeMode(Mode::CLIP);
    });

    cmd_shift_c->registerHotkey(QKeySequence(Qt::MetaModifier | Qt::ShiftModifier | Qt::Key_C));

    // disableCmdQ();
}

MainWindow::MainWindow(Mode mode, QWidget* parent)
    : QMainWindow(parent),
      mode(mode),
      mode_factory(new ModeHandlerFactory),
      config_manager(new ConfigManager) {

    // In your MainWindow or App initialization
    server = new Server(this);
    server->startServer("fuzzymac_socket");

    const std::vector<Mode> modes = {
        Mode::APP,
        Mode::FILE,
        Mode::WALLPAPER,
        Mode::CLIP,
        Mode::CLI,
    };

    for (auto mode : modes) {
        mode_handlers[mode] = mode_factory->create(mode, this);
    }

    show_info_panel = config_manager->get<bool>({"info_panel"});

    createWidgets();
    connectEventHandlers();
    loadConfig();
    createKeybinds();

    sleep();
}

MainWindow::~MainWindow() {
    delete mode_factory;
    delete config_manager;
}

void MainWindow::onApplicationStateChanged(Qt::ApplicationState state) {
    if (state == Qt::ApplicationInactive) {
        hide();
    }
}

void MainWindow::loadConfig() {
    QPixmapCache::clear();
    query_edit->loadConfig();
    results_list->loadConfig();
    loadStyle();
    for (auto& [mode, handler] : mode_handlers) {
        handler->load();
    }
    onTextChange(query_edit->text());
}

QListWidgetItem* MainWindow::createListItem(QWidget* widget) {
    QListWidgetItem* item = new QListWidgetItem(results_list);
    item->setSizeHint(widget->sizeHint());
    return item;
}

QListWidgetItem* MainWindow::createListItem(const QString& name, const std::optional<QIcon>& icon) {
    QListWidgetItem* item = nullptr;
    if (icon) {
        item = new QListWidgetItem(icon.value(), name);
    } else {
        item = new QListWidgetItem(name);
    }

    return item;
}

void MainWindow::clearResultList() {
    results_list->clear();
}

int MainWindow::getCurrentResultIdx() const {
    return results_list->currentRow();
}

int MainWindow::getResultsNum() const {
    return results_list->count();
}

void MainWindow::refreshResults() {
    onTextChange(query_edit->text());
}

QIcon MainWindow::createIcon(const QString& path, const QColor& color) const {
    QPixmap pixmap(path); // Load image or SVG as pixmap
    if (pixmap.isNull()) {
        return QIcon();
    }

    QPixmap tinted(pixmap.size());
    tinted.fill(Qt::transparent); // Ensure transparency preserved

    QPainter painter(&tinted);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawPixmap(0, 0, pixmap); // Draw original

    // Apply color mask using SourceIn
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    painter.end();

    return QIcon(tinted);
}

QIcon MainWindow::getFileIcon(const QString& path) const {
    return icon_provider.icon(QFileInfo(path));
}

const ModeHandler* MainWindow::getCurrentModeHandler() const {
    return mode_handlers.at(mode).get();
}

const ModeHandler* MainWindow::getModeHandler(Mode mode) const {
    return mode_handlers.at(mode).get();
}

QString MainWindow::getQuery() const {
    return query_edit->text();
}

void MainWindow::loadStyle() {

    // update icons
    icons = {
        {"clipboard",
         createIcon(
             ":/res/icons/clipboard.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},

        {"clear_clipboard",
         createIcon(
             ":/res/icons/clipboard_clear.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},

        {"search_files",
         createIcon(
             ":/res/icons/file_search.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},

        {"text",
         createIcon(
             ":/res/icons/text.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},
        {"wallpaper",
         createIcon(
             ":/res/icons/wallpaper.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},
        {"bluetooth",
         createIcon(
             ":/res/icons/bluetooth.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},
        {"settings",
         createIcon(
             ":/res/icons/settings.svg",
             QColor(QString::fromStdString(getConfigManager().get<std::string>({"colors", "results_list", "text"}))))},

    };

    setWindowOpacity(config_manager->get<float>({"opacity"}));
    setupWindowDecoration(this, config_manager);

    // update border color size
    auto border_size = config_manager->get<int>({"border_size"});
    border_widget->layout()->setContentsMargins(border_size, border_size, border_size, border_size);

    //  border_widget->setStyleSheet(QString(R"(
    //          background: %1;
    // new-session -ds "Desktop" -c "~/Desktop/"   )")
    //                                   .arg(config_manager->get<std::string>({"colors", "outer_border"})));

    main_widget->setStyleSheet(QString(R"(
            background: %1;
            margin: 0px;
            padding: 0px;
    )")
                                   .arg(config_manager->get<std::string>({"colors", "inner_border"})));

    mode_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            background: %2;
            font-weight: 500;
            margin: 0px;
            font-size: 16px;
            font-family: %3;
            padding: 2px;
            border-bottom: 0px solid %4;
        }
    )")
                                  .arg(config_manager->get<std::string>({"colors", "mode_label", "text"}))
                                  .arg(config_manager->get<std::string>({"colors", "mode_label", "background"}))
                                  .arg(config_manager->get<std::string>({"font"}))
                                  .arg(config_manager->get<std::string>({"colors", "inner_border"})));
}

void MainWindow::clearQuery() {
    query_edit->setText("");
    refreshResults();
}

void MainWindow::changeMode(Mode new_mode) {

    // there is no need to change;

    if (new_mode == mode) {
        return;
    }

    mode_handlers[mode]->onModeExit();

    mode = new_mode;
    clearQuery();
}

void MainWindow::setInfoPanelContent(InfoPanelContent* content) {
    info_panel->setContent(content);

    // if there is no info, hide the panel
    if (content == nullptr) {
        info_panel->hide();
        return;
    }

    info_panel->show();
}

std::vector<FuzzyWidget*> MainWindow::getModesWidgets() const {
    std::vector<FuzzyWidget*> res{};
    for (auto& [_, handler] : mode_handlers) {
        auto widgets = handler->createMainModeWidgets();
        res.insert(res.end(), widgets.begin(), widgets.end());
    }

    return res;
}

std::map<QString, QIcon> MainWindow::getIcons() {
    return icons;
}

void MainWindow::handleNewRequest() {
    wakeup();
    spdlog::info("Changig mode");
    changeMode(Mode::CLI);
    spdlog::info("loading");
    mode_handlers[Mode::CLI]->load();
}

Server* MainWindow::getServer() const {
    return server;
}

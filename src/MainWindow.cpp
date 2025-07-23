#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/Animations.hpp"
#include "FuzzyMac/ConfigManager.hpp"
#include "FuzzyMac/FuzzyWidget.hpp"
#include "FuzzyMac/InfoPanel.hpp"
#include "FuzzyMac/MacGlobShortcuts.hpp"
#include "FuzzyMac/ModeHandler.hpp"
#include "FuzzyMac/ModeHandlerFactory.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/ResultsPanel.hpp"

#include "toml++/impl/parser.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QEvent>
#include <QFont>
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
#include <memory>
#include <variant>

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

    QVBoxLayout* border_layout = new QVBoxLayout(border_widget);
    border_widget->setLayout(border_layout);

    resize(700, 500);

    border_layout->addWidget(main_widget);

    if (!show_info_panel) {
        info_panel->hide();
    }
    setWindowFlag(Qt::WindowStaysOnTopHint);


    QApplication::setQuitOnLastWindowClosed(false);
    makeWindowFloating(this);

    QHBoxLayout* content_layout = new QHBoxLayout;

    layout->addWidget(query_edit, 0);
    layout->addWidget(mode_label, 0);

    content_layout->addWidget(results_list, 5);
    content_layout->addWidget(info_panel, 8);
    content_layout->setSpacing(0);

    layout->addLayout(content_layout);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSpacing(0);
    mode_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    results_list->setDragEnabled(true);
    results_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    results_list->setSelectionMode(QAbstractItemView::SingleSelection);

    main_widget->setLayout(layout);
    setCentralWidget(border_widget);
    wakeup();
}

void MainWindow::nextItem() {
    int next = std::min(results_list->count() - 1, results_list->currentRow() + 1);
    if (next == results_list->currentRow()) {
        return;
    }

    if (next != results_list->currentRow()) {
        results_list->setCurrentRow(next);
    }
    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

void MainWindow::openItem() {
    if (getResultsNum() > 0 && getCurrentResultIdx() != -1) {
        mode_handlers[mode]->enterHandler();
    }
}

void MainWindow::quickLock() {
    mode_handlers[mode]->handleQuickLook();
}

void MainWindow::prevItem() {
    int prev = std::max(0, results_list->currentRow() - 1);

    if (prev == results_list->currentRow()) {
        return;
    }

    if (prev != results_list->currentRow()) {
        results_list->setCurrentRow(prev);
    }

    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

void MainWindow::wakeup() {

    if (config_manager->get<bool>({"animations"})) {
        obacityAnimator(this, 0.0, 1.0, 150);
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
    };

    if (config_manager->get<bool>({"animations"})) {
        auto* anim = obacityAnimator(this, 1.0, 0.0, 150);
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
    if (mode != Mode::CLI) {
        matchModeShortcut(text);
    }


    // let the mode handle the query
    mode_handlers[mode]->invokeQuery(query_edit->text());

    // replace the mode label text if possible
    if (const auto& mode_text = mode_handlers[mode]->handleModeText(); !mode_text.isEmpty()) {
        mode_label->setText(QString("%1").arg(mode_text));
        mode_label->show();
    } else {
        mode_label->hide();
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

    // try to select the first entry
    if (results_list->count()) {
        results_list->setCurrentRow(0);
    }

    // update info panel;
    if (show_info_panel) {
        setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
    }
}

void MainWindow::connectEventHandlers() {

    connect(query_edit, &QueryEdit::textChanged, this, &MainWindow::onTextChange);

    connect(
        window()->windowHandle(), &QWindow::screenChanged, this, [this](QScreen* newScreen) { centerWindow(this); });
    connect(results_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) { openItem(); });
    connect(results_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (show_info_panel) {
            setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
        }
    });

    connect(config_manager, &ConfigManager::configChange, this, [this]() { loadConfig(); });

#ifndef CLI_TOOL
    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
#endif
}

void MainWindow::createKeybinds() {
    // Global shortcuts

    if (mode != Mode::CLI) {
        registerGlobalHotkey(this);
        new QShortcut(Qt::Key_Escape, this, [this]() { this->sleep(); });
        // disableCmdQ();
    }

#ifndef CLI_TOOL
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_I), this, [this]() {
        show_info_panel = !show_info_panel;
        info_panel->setHidden(!show_info_panel);

        if (show_info_panel) {
            setInfoPanelContent(mode_handlers[mode]->getInfoPanelContent());
        }
    });
#endif

    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_N), this, SLOT(nextItem()));
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_P), this, SLOT(prevItem()));

    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Y), this, SLOT(quickLock()));
    new QShortcut(Qt::Key_Return, this, SLOT(openItem()));
    connect(query_edit, &QueryEdit::requestAppCopy, this, [this]() { copyToClipboard(); });
}

MainWindow::MainWindow(Mode mode, QWidget* parent)
    : QMainWindow(parent),
      mode(mode),
      mode_factory(new ModeHandlerFactory),
      config_manager(new ConfigManager) {

#ifndef CLI_TOOL
    for (auto mode : {Mode::APP, Mode::FILE, Mode::CLIP}) {
        mode_handlers[mode] = mode_factory->create(mode, this);
    }
#else
    mode_handlers[Mode::CLI] = mode_factory->create(Mode::CLI, this);
    mode = Mode::CLI;
#endif

    show_info_panel = config_manager->get<bool>({"info_panel"});

    createWidgets();
    connectEventHandlers();
    loadConfig();
    createKeybinds();

    if (mode != Mode::CLI) {
        sleep();
    } else {
        wakeup();
    }
}

MainWindow::~MainWindow() {
    delete mode_factory;
    delete config_manager;
}

#ifndef CLI_TOOL
void MainWindow::onApplicationStateChanged(Qt::ApplicationState state) {
    if (state == Qt::ApplicationInactive) {
        hide();
    }
}
#endif

void MainWindow::loadConfig() {
    QPixmapCache::clear();
    query_edit->loadConfig();
    results_list->loadConfig();
    loadStyle();
    mode_handlers[mode]->load();
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

void MainWindow::copyToClipboard() {
    mode_handlers[mode]->handleCopy();
}

void MainWindow::copyPathToClipboard() {
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

    };

    auto border_size = config_manager->get<int>({"border_size"});
    border_widget->layout()->setContentsMargins(border_size, border_size, border_size, border_size);
    border_widget->setStyleSheet(QString(R"(
            background: %1;
            padding: 0px;
    )")
                                     .arg(config_manager->get<std::string>({"colors", "outer_border"})));

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
            font-family: %3;
            padding: 2px;
            border-bottom: 2px solid %4;
        }
    )")
                                  .arg(config_manager->get<std::string>({"colors", "mode_label", "text"}))
                                  .arg(config_manager->get<std::string>({"colors", "mode_label", "background"}))
                                  .arg(config_manager->get<std::string>({"font"}))
                                  .arg(config_manager->get<std::string>({"colors", "inner_border"})));
}

void MainWindow::changeMode(Mode new_mode) {

    // there is no need to change;
    if (new_mode == mode) {
        return;
    }

    mode = new_mode;

    query_edit->setText("");
    refreshResults();

    // animate if transitioning to a new mode
    if (new_mode != Mode::APP && config_manager->get<bool>({"animations"})) {
        bounceAnimator(this, 0.05, 200);
    }
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

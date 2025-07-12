#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/MacGlobShortcuts.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/QueryInput.hpp"
#include "FuzzyMac/ResultsPanel.hpp"
#include "FuzzyMac/Utils.hpp"

#include "toml++/impl/parser.hpp"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QFont>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QStaticText>
#include <QWindow>
#include <QtConcurrent>
#include <filesystem>
#include <memory>

void MainWindow::processConfigFile() {
}

const toml::table& MainWindow::getConfig() const {
    return config;
}

void MainWindow::createWidgets() {
    central = new QWidget(this);
    layout = new QVBoxLayout(central);

    query_input = new QueryInput(central);
    results_list = new ResultsPanel(central);
    // search_refresh_timer = new QTimer;
    // search_refresh_timer->setSingleShot(true);
}

void MainWindow::setupLayout() {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    resize(600, 400);
    QApplication::setQuitOnLastWindowClosed(false);

    makeWindowFloating(this);
    layout->addWidget(query_input, 0);
    layout->addWidget(results_list, 0);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    results_list->setDragEnabled(true);
    results_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    results_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    results_list->setSelectionMode(QAbstractItemView::SingleSelection);

    central->setLayout(layout);
    setCentralWidget(central);

    wakeup();
}

void MainWindow::nextItem() {
    int next = std::min(results_list->count() - 1, results_list->currentRow() + 1);
    results_list->setCurrentRow(next);
}

void MainWindow::openItem() {
    mode_handler[mode]->enterHandler();
}

void MainWindow::quickLock() {
    mode_handler[mode]->handleQuickLock();
}

void MainWindow::prevItem() {
    int prev = std::max(0, results_list->currentRow() - 1);
    results_list->setCurrentRow(prev);
}

void MainWindow::wakeup() {
    show();
    raise();
    activateWindow();
    centerWindow(this);
    query_input->setFocus();
}

void MainWindow::sleep() {
    deactivateApp();
    hide();
}

void MainWindow::onTextChange(const QString& text) {
    if (mode != Mode::CLI) {
        if (text.size() > 0 && text[0] == ' ') {
            mode = Mode::FILE;
        } else {
            mode = Mode::APP;
        }
    }

    if (results_watcher->isRunning()) {
        results_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, text]() { return mode_handler[mode]->getResults(text); });

    results_watcher->setFuture(future);
}

void MainWindow::connectEventHandlers() {

    results_watcher = new QFutureWatcher<std::vector<QListWidgetItem*>>(this);

    connect(query_input, &QueryInput::textChanged, this, &MainWindow::onTextChange);

    connect(results_watcher, &QFutureWatcher<QStringList>::finished, this, [this]() {
        auto results = results_watcher->result();
        results_list->clear();
        for (auto* item : results) {
            results_list->addItem(item);
        }
        if (results.size() >= 1) {
            results_list->setCurrentRow(0);
        }
    });

    connect(
        window()->windowHandle(), &QWindow::screenChanged, this, [this](QScreen* newScreen) { centerWindow(this); });
    connect(results_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) { openItem(); });

#ifndef CLI_TOOL
    QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, &MainWindow::onApplicationStateChanged);
#endif
}

void MainWindow::createKeybinds() {
    // Global shortcuts

    if (mode != Mode::CLI) {
        registerGlobalHotkey(this);
    }

    disableCmdQ();
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_N), this, SLOT(nextItem()));
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_P), this, SLOT(prevItem()));

    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Y), this, SLOT(quickLock()));
    new QShortcut(Qt::Key_Return, this, SLOT(openItem()));
    new QShortcut(Qt::Key_Escape, this, [this]() { this->sleep(); });

    connect(query_input, &QueryInput::requestAppCopy, this, [this]() { copyToClipboard(); });
}

MainWindow::MainWindow(Mode mode, QWidget* parent)
    : QMainWindow(parent),
      mode(mode),
      config(default_config) {

    mode_handler.emplace(Mode::CLI, std::make_unique<CLIModeHandler>(this));
    mode_handler.emplace(Mode::APP, std::make_unique<AppModeHandler>(this));
    mode_handler.emplace(Mode::FILE, std::make_unique<FileModeHandler>(this));
    createWidgets();
    setupLayout();
    connectEventHandlers();
    loadConfig();

    createKeybinds();

    if (mode != Mode::CLI) {
        deactivateApp();
        hide();
    } else {
        wakeup();
    }

    fs::path home_path = std::getenv("HOME");
    fs::path config_path = home_path / ".config" / "FuzzyMac" / "config.toml";
    config_file_watcher = new QFileSystemWatcher(this);
    config_file_watcher->addPath(QString::fromStdString(config_path));
    connect(config_file_watcher, &QFileSystemWatcher::fileChanged, this, [this]() { loadConfig(); });
}

MainWindow::~MainWindow() {
}

#ifndef CLI_TOOL
void MainWindow::onApplicationStateChanged(Qt::ApplicationState state) {
    if (state == Qt::ApplicationInactive) {
        hide();
    }
}
#endif

void MainWindow::loadConfig() {

    std::vector<std::string> p_ = {"$HOME/.config/FuzzyMac/config.toml"};
    expandPaths(p_);
    std::string config_path = p_[0];

    if (fs::exists(config_path)) {
        toml::table new_config = config;
        // reload config file
        try {
            new_config = toml::parse_file(config_path);
        } catch (const toml::parse_error& err) {
            QMessageBox::information(nullptr, "Error", "It looks that you have a syntax error in config file, falling back to default config");
            new_config = config;
        }
        config = new_config;
    }

    // reload widgets
    query_input->loadConfig();
    results_list->loadConfig();

    int curr_selection = results_list->currentRow();
    QString curr_query = query_input->text();

    // reload the mode handler
    mode_handler[mode]->load();

    // simulate a text change in order to update results
    onTextChange(query_input->text());
}

void MainWindow::addToResultList(const std::string& name, std::optional<fs::path> path) {
    results_list->addItem(createListItem(name, path));
}

QListWidgetItem* MainWindow::createListItem(const std::string& name, std::optional<fs::path> path) {
    QListWidgetItem* item = nullptr;
    if (path.has_value()) {
        QFileInfo file_info(QString::fromStdString(path->string()));
        item = new QListWidgetItem(icon_provider.icon(std::move(file_info)), QString::fromStdString(name));
    } else {
        item = new QListWidgetItem(QString::fromStdString(name));
    }

    return item;
}

void MainWindow::copyToClipboard() {
    mode_handler[mode]->handleCopy();
}

void MainWindow::copyPathToClipboard() {
}

void MainWindow::clearResultList() {
    results_list->clear();
}

int MainWindow::getCurrentResultIdx() const {
    return results_list->currentRow();
}

int MainWindow::resultsNum() const {
    return results_list->count();
}

void MainWindow::refreshResults() {
    onTextChange(query_input->text());
}

QIcon MainWindow::getFileIcon(const std::string& path) const {
    return icon_provider.icon(QFileInfo(QString::fromStdString(path)));
}

ModeHandler* MainWindow::getModeHandler() const {
    return mode_handler.at(mode).get();
}

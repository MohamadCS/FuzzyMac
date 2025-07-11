#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/MacGlobShortcuts.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "toml++/impl/parser.hpp"

#include <QApplication>
#include <QDebug>
#include <QFont>
#include <QGuiApplication>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QStaticText>
#include <QtConcurrent>
#include <memory>

void MainWindow::processConfigFile() {
    fs::path home_path = std::getenv("HOME");
    fs::path config_path = home_path / ".config" / "FuzzyMac" / "config.toml";
    config = toml::parse_file(config_path.string());
}

const toml::table& MainWindow::getConfig() const {
    return config;
}



void MainWindow::createWidgets() {
    central = new QWidget(this);
    layout = new QVBoxLayout(central);

    query_input = new QLineEdit(central);
    results_list = new QListWidget(central);
    // search_refresh_timer = new QTimer;
    // search_refresh_timer->setSingleShot(true);
}


void MainWindow::setupLayout() {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    if (mode == Mode::CLI) {
        setWindowFlag(Qt::FramelessWindowHint);
        setWindowFlag(Qt::FramelessWindowHint);
        // setWindowFlag(Qt::Tool);
    } else {
        resize(400, 400);
        makeWindowFloating(this);
    }
    //

    layout->addWidget(query_input, 0);
    layout->addWidget(results_list, 0);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    central->setLayout(layout);

    query_input->setFocus();
    activateWindow();
    raise();

    setCentralWidget(central);
    setFixedSize(400, 300);
}

void MainWindow::nextItem() {
    int next = std::min(results_list->count() - 1, results_list->currentRow() + 1);
    results_list->setCurrentRow(next);
}

void MainWindow::openItem() {
    mode_handler[mode]->enterHandler(results_list);
}

void MainWindow::quickLock() {
    mode_handler[mode]->handleQuickLock(results_list);
}

void MainWindow::prevItem() {
    int prev = std::max(0, results_list->currentRow() - 1);
    results_list->setCurrentRow(prev);
}

void MainWindow::wakeup() {
    show();
    raise();
    activateWindow();
    query_input->setFocus();
}

void MainWindow::sleep() {
    deactivateApp();
    hide();
}

void MainWindow::connectEventHandlers() {

    results_watcher = new QFutureWatcher<QStringList>(this);

    connect(query_input, &QLineEdit::textChanged, this, [this](const QString& text) {
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

        QFuture<QStringList> future = QtConcurrent::run(
            [this, text]() -> QStringList { return mode_handler[mode]->getResults(text, this->results_list); });

        results_watcher->setFuture(future);
    });

    connect(results_watcher, &QFutureWatcher<QStringList>::finished, this, [this]() {
        QStringList results = results_watcher->result();
        results_list->clear();
        results_list->addItems(results);
        if (results.size() >= 1) {
            results_list->setCurrentRow(0);
        }
    });

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
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_Q), this, SLOT(quickLock()));
    new QShortcut(Qt::Key_Return, this, SLOT(openItem()));

    auto escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() { this->sleep(); });
}

void MainWindow::fillData() {
    mode_handler[mode]->fillData(results_list);
    if (results_list->count() > 0) {
        results_list->setCurrentRow(0);
    }
}

MainWindow::MainWindow(Mode mode, QWidget* parent)
    : QMainWindow(parent),
      mode(mode) {

    processConfigFile();
    mode_handler.emplace(Mode::CLI, std::make_unique<CLIModeHandler>(&config));
    mode_handler.emplace(Mode::APP, std::make_unique<AppModeHandler>(&config));
    mode_handler.emplace(Mode::FILE, std::make_unique<FileModeHandler>(&config));
    createWidgets();
    setupLayout();
    setupStyles();
    createKeybinds();
    connectEventHandlers();
    fillData();

    if (mode != Mode::CLI) {
        deactivateApp();
        hide();
    } else {
        wakeup();
    }
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

void MainWindow::setupStyles() {

    query_input->setStyleSheet(QString(R"(
                                    QLineEdit {
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        background: %4;
                                        border : none;
                                        padding: 0px;
                                    })")
                                   .arg(get<std::string>(config, {"colors", "query_input", "selection_background"}))
                                   .arg(get<std::string>(config, {"colors", "query_input", "selection"}))
                                   .arg(get<std::string>(config, {"colors", "query_input", "text"}))
                                   .arg(get<std::string>(config, {"colors", "query_input", "background"})));

    results_list->setStyleSheet(QString(R"(
                                    QListWidget {
                                        background: %4;
                                        selection-background-color : %1;
                                        selection-color : %2;
                                        color: %3;
                                        border: none;
                                        padding: 0px;
                                    })")
                                    .arg(get<std::string>(config, {"colors", "results_list", "selection_background"}))
                                    .arg(get<std::string>(config, {"colors", "results_list", "selection"}))
                                    .arg(get<std::string>(config, {"colors", "results_list", "text"}))
                                    .arg(get<std::string>(config, {"colors", "results_list", "background"})));

    QFont font(get<std::string>(config, {"font"}).c_str(), 25);
    query_input->setFont(font);
    font.setPointSize(15);

    results_list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    results_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    results_list->setSelectionMode(QAbstractItemView::SingleSelection);

    results_list->setFont(font);
}



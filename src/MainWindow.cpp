#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/MacGlobShortcuts.hpp"
#include "FuzzyMac/ModHandler.hpp"
#include "FuzzyMac/NativeMacHandlers.hpp"
#include "FuzzyMac/ParseConfig.hpp"
#include "FuzzyMac/QueryEdit.hpp"
#include "FuzzyMac/ResultsPanel.hpp"
#include "FuzzyMac/Utils.hpp"

#include "toml++/impl/parser.hpp"

#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QFont>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QShortcut>
#include <QStaticText>
#include <QWindow>
#include <QtConcurrent>
#include <filesystem>
#include <memory>

const toml::table& MainWindow::getConfig() const {
    return config;
}

void MainWindow::createWidgets() {
    central = new QWidget(this);
    layout = new QVBoxLayout(central);

    query_edit = new QueryEdit(central);
    results_list = new ResultsPanel(central);
    mode_label = new QLabel(central);
}

void MainWindow::setupLayout() {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    resize(600, 400);
    QApplication::setQuitOnLastWindowClosed(false);

    makeWindowFloating(this);
    layout->addWidget(query_edit, 0);
    layout->addWidget(mode_label, 0);
    layout->addWidget(results_list, 0);
    mode_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

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
    // QRect end_rect = geometry();
    //
    // QRect start_rect(end_rect.center().x(), end_rect.center().y(), 0, 0);
    //
    // setGeometry(start_rect);
    // setWindowOpacity(0.0);
    show();

    // QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity",this);
    // anim->setDuration(150);
    // anim->setStartValue(0.0);
    // anim->setEndValue(1.0);
    // anim->start();
    //
    // // TODO: make a special class for animations
    // QPropertyAnimation* scale_anim = new QPropertyAnimation(this, "geometry",this);
    // scale_anim->setDuration(320);
    // scale_anim->setStartValue(start_rect);
    // scale_anim->setEndValue(end_rect);
    // scale_anim->setEasingCurve(QEasingCurve::OutCirc);
    // scale_anim->start();

    raise();
    activateWindow();
    centerWindow(this);
    query_edit->setFocus();
}

void MainWindow::sleep() {
    // QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    // anim->setDuration(150);
    // anim->setStartValue(1.0);
    // anim->setEndValue(0.0);
    // anim->start();

    // connect(anim, &QPropertyAnimation::finished, this, [this]() {
            deactivateApp();
            hide();
    // });
}

void MainWindow::onTextChange(const QString& text) {
    if (mode != Mode::CLI) {
        if (text.startsWith(' ')) {
            query_edit->setText("→ ");
            mode = Mode::FILE;
        } else if (text.isEmpty()) {
            mode = Mode::APP;
        }
    }

    if (results_watcher->isRunning()) {
        results_watcher->cancel();
    }

    auto future = QtConcurrent::run([this, text]() { return mode_handler[mode]->getResults(text); });

    results_watcher->setFuture(future);
    mode_label->setText(QString("%1").arg(mode_handler[mode]->handleModeText()));
}

void MainWindow::connectEventHandlers() {

    results_watcher = new QFutureWatcher<std::vector<QListWidgetItem*>>(this);

    connect(query_edit, &QueryEdit::textChanged, this, &MainWindow::onTextChange);

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

    connect(query_edit, &QueryEdit::requestAppCopy, this, [this]() { copyToClipboard(); });
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
            new_config = config;
        }
        config = new_config;
    }

    // reload widgets
    query_edit->loadConfig();
    results_list->loadConfig();

    auto border_size = get<int>(config, {"border_size"});
    layout->setContentsMargins(border_size, border_size, border_size, border_size);

    // TODO: move to another function
    setStyleSheet(QString(R"(
        QMainWindow {
            background: %1;
        }
    )")
                      .arg(get<std::string>(config, {"colors", "background"})));

    // TODO: move to another function
    mode_label->setStyleSheet(QString(R"(
        QLabel {
            color : %1;
            background: %2;
            font-weight: 500;
            font-family: %3;
            border-radius: 10px;
        }
    )")
                                  .arg(get<std::string>(config, {"colors", "mode_label", "text"}))
                                  .arg(get<std::string>(config, {"colors", "mode_label", "background"}))
                                  .arg(get<std::string>(config, {"font"})));

    int curr_selection = results_list->currentRow();
    QString curr_query = query_edit->text();

    // reload the mode handler
    mode_handler[mode]->load();

    // simulate a text change in order to update results
    onTextChange(query_edit->text());
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

int MainWindow::getResultsNum() const {
    return results_list->count();
}

void MainWindow::refreshResults() {
    onTextChange(query_edit->text());
}

QIcon MainWindow::getFileIcon(const std::string& path) const {
    return icon_provider.icon(QFileInfo(QString::fromStdString(path)));
}

ModeHandler* MainWindow::getModeHandler() const {
    return mode_handler.at(mode).get();
}

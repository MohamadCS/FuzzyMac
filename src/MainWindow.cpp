#include "FuzzyMac/MainWindow.hpp"
#include "FuzzyMac/MacGlobShortcuts.hpp"
#include "FuzzyMac/ModeHandler.hpp"
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
#include <QPainter>
#include <QProcess>
#include <QShortcut>
#include <QStaticText>
#include <QWindow>
#include <QtConcurrent>
#include <filesystem>
#include <memory>
#include <variant>

const toml::table& MainWindow::getConfig() const {
    return config;
}

void MainWindow::createWidgets() {
    central = new QWidget(this);
    layout = new QVBoxLayout(central);

    query_edit = new QueryEdit(central);
    results_list = new ResultsPanel(central);
    mode_label = new QLabel(central);
    // timer->setSingleShot(true);
}

void MainWindow::setupLayout() {
    setWindowFlag(Qt::WindowStaysOnTopHint);
    resize(600, 400);
    QApplication::setQuitOnLastWindowClosed(false);
    makeWindowFloating(this);

    layout->addWidget(query_edit, 0);
    layout->addWidget(mode_label, 0);
    layout->addWidget(results_list, 0);
    layout->setSpacing(0);
    mode_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

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

    if (next != results_list->currentRow()) {
        results_list->setCurrentRow(next);
    }
}

void MainWindow::openItem() {
    mode_handler[mode]->enterHandler();
}

void MainWindow::quickLock() {
    mode_handler[mode]->handleQuickLook();
}

void MainWindow::prevItem() {
    int prev = std::max(0, results_list->currentRow() - 1);

    if (prev != results_list->currentRow()) {
        results_list->setCurrentRow(prev);
    }
}

void MainWindow::wakeup() {

    if (get<bool>(config, {"animations"})) {
        setWindowOpacity(0.0);
        show();
        QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity", this);
        anim->setDuration(150);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->start();
    } else {
        setWindowOpacity(1);
        show();
    }

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

    if (get<bool>(config, {"animations"})) {
        QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity", this);
        anim->setDuration(150);
        anim->setStartValue(1.0);
        anim->setEndValue(0.0);
        anim->start();
        connect(anim, &QPropertyAnimation::finished, this, sleep_);
    } else {
        sleep_();
    }
}

void MainWindow::matchModeShortcut(const QString& text) {
    if (mode != Mode::APP) {
        return;
    }

    for (auto& [mode, handler] : mode_handler) {
        auto prefix = handler->getPrefix();
        if (!prefix.isEmpty() && prefix == text) {
            changeMode(mode);
            return;
        }
    }
}

void MainWindow::onTextChange(const QString& text) {
    qDebug() << "text changed to " << text;
    matchModeShortcut(text);

    mode_handler[mode]->invokeQuery(text);

    // replace mode text info if available
    if (const auto& mode_text = mode_handler[mode]->handleModeText(); !mode_text.isEmpty()) {
        mode_label->setText(QString("%1").arg(mode_text));
        mode_label->show();
    } else {
        mode_label->hide();
    }

    qDebug() << "Finished On text change";
}

void MainWindow::processResults(const ResultsVec& results) {

    qDebug() << "Processing Results";
    results_list->clear();
    qDebug() << results.size();
    for (auto obj : results) {
        auto item = obj->getItem();
        if (std::holds_alternative<QListWidgetItem*>(item)) {
            results_list->addItem(std::get<QListWidgetItem*>(item));
        } else {
            auto* obj = std::get<FuzzyWidget*>(item);
            auto* item = createListItem(obj);
            results_list->setItemWidget(item, obj);
        }
    }
    qDebug() << "Finished processing results";

    if (results_list->count()) {
        results_list->setCurrentRow(0);
    }
}

void MainWindow::connectEventHandlers() {

    connect(query_edit, &QueryEdit::textChanged, this, &MainWindow::onTextChange);

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
        new QShortcut(Qt::Key_Escape, this, [this]() { this->sleep(); });
        disableCmdQ();
    }

    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_N), this, SLOT(nextItem()));
    new QShortcut(QKeySequence(Qt::MetaModifier | Qt::Key_P), this, SLOT(prevItem()));

    new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_Y), this, SLOT(quickLock()));
    new QShortcut(Qt::Key_Return, this, SLOT(openItem()));
    connect(query_edit, &QueryEdit::requestAppCopy, this, [this]() { copyToClipboard(); });
}

MainWindow::MainWindow(Mode mode, QWidget* parent)
    : QMainWindow(parent),
      mode(mode),
      config(default_config) {

    mode_handler.emplace(Mode::FILE, std::make_unique<FileModeHandler>(this));
    mode_handler.emplace(Mode::APP, std::make_unique<AppModeHandler>(this));
    // mode_handler.emplace(Mode::CLI, std::make_unique<CLIModeHandler>(this));

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

    QStringList p_{"$HOME/.config/FuzzyMac/config.toml"};
    expandPaths(p_);
    QString config_path = p_[0];

    if (QFileInfo(config_path).exists()) {
        toml::table new_config = config;
        // reload config file
        try {
            new_config = toml::parse_file(config_path.toStdString());
        } catch (const toml::parse_error& err) {
            new_config = config;
        }
        config = new_config;
    }

    // reload widgets
    query_edit->loadConfig();
    results_list->loadConfig();

    loadStyle();

    int curr_selection = results_list->currentRow();
    QString curr_query = query_edit->text();

    // reload the mode handler
    mode_handler[mode]->load();

    // simulate a text change in order to update results
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
    return mode_handler.at(mode).get();
}

const ModeHandler* MainWindow::getModeHandler(Mode mode) const {
    return mode_handler.at(mode).get();
}

QString MainWindow::getQuery() const {
    return query_edit->text();
}

bool MainWindow::isWidgetCurrentSelection(QWidget* widget) const {
    QListWidgetItem* selectedItem = results_list->currentItem();
    if (selectedItem) {
        QWidget* curr_selection = results_list->itemWidget(selectedItem);
        if (curr_selection == widget) {
            return true;
        }
    }
    return false;
}

void MainWindow::loadStyle() {
    auto border_size = get<int>(config, {"border_size"});
    layout->setContentsMargins(border_size, border_size, border_size, border_size);
    setStyleSheet(QString(R"(
        QMainWindow {
            background: %1;
        }
    )")
                      .arg(get<std::string>(config, {"colors", "background"})));

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
}

void MainWindow::changeMode(Mode new_mode) {
    qDebug() << "changing mode";

    if (new_mode == mode) {
        return;
    }

    mode = new_mode;
    query_edit->setText("");
    refreshResults();

    if (new_mode == Mode::APP) {
        return;
    }

    if (get<bool>(config, {"animations"})) {
        QRect original = geometry();
        int dx = original.width() * 0.05;
        int dy = original.height() * 0.05;

        QRect enlarged =
            QRect(original.x() - dx / 2, original.y() - dy / 2, original.width() + dx, original.height() + dy);

        QPropertyAnimation* anim = new QPropertyAnimation(this, "geometry");
        anim->setDuration(300);
        anim->setKeyValues({{0.0, original}, {0.5, enlarged}, {1.0, original}});
        anim->setEasingCurve(QEasingCurve::OutBack);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

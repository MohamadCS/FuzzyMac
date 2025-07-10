#include "FuzzyMac/MainWindow.hpp"
#include <QApplication>

#include <QLabel>

int main(int argc, char* argv[]) {
#ifdef CLI_TOOL
    Mode mode = Mode::CLI;
#else
    Mode mode = Mode::APP;
#endif

    QApplication app(argc, argv);
    MainWindow main_win(mode);
    main_win.show();
    return app.exec();
}

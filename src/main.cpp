#include "FuzzyMac/MainWindow.hpp"
#include <QApplication>


int main(int argc, char* argv[]) {
    Mode mode = Mode::APP;
    QApplication app(argc, argv);
    MainWindow main_win(mode);
    main_win.show();
    return app.exec();
}

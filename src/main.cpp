#include <string>
#include "wx/init.h"
#include <map>

#include "../include/CLI/CLI11.hpp"
#include "../include/App/App.hpp"


#ifdef CLI_TOOL 

enum Flag {
    DEFAULT,
    SHELL,
    BASENAME
};

const std::map<Flag, std::string> FLAG_NAME{
    {DEFAULT, "--default"},
    {SHELL, "--shell"},
    {BASENAME, "--basename"},
};

struct AppCli {
    CLI::App cli_app{"FuzzyMac"};
    bool basename = false;

    AppCli() = default;
    int run(int argc, char* argv[]);
};


int AppCli::run(int argc, char* argv[]) {

    bool default_run = false;
    cli_app.add_flag(FLAG_NAME.at(Flag::DEFAULT), default_run, "Run without GUI");

    cli_app.add_flag(FLAG_NAME.at(Flag::BASENAME), basename, "Show basename only");
    CLI11_PARSE(cli_app, argc, argv);



    return 0;
}
#endif

int main (int argc, char *argv[]) {
#ifdef CLI_TOOL 
    AppCli cli;
    cli.run(argc, argv);
#endif

    if (!wxEntryStart(argc, argv)) {
        std::cerr << "Failed to start wxWidgets.\n";
        return 1;
    }

    // Create and register your wxApp
    App* app = new App();
    wxApp::SetInstance(app);

    if (!app->CallOnInit()) {
        std::cerr << "wxApp failed to initialize.\n";
        wxEntryCleanup();
        return 1;
    }

     int code = app->OnRun();

    app->OnExit();
    wxEntryCleanup();
    return 0;
}


#include "../include/App/AppCli.hpp"




int AppCli::run(int argc, char* argv[]) {

    bool default_run = false;
    cli_app.add_flag(FLAG_NAME.at(Flag::DEFAULT), default_run, "Run without GUI");

    cli_app.add_flag(FLAG_NAME.at(Flag::BASENAME), basename, "Show basename only");


    CLI11_PARSE(cli_app, argc, argv);

    return 0;
}

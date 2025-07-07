#include "../include/App/App.hpp"
#include "../include/App/AppCli.hpp"
#include "../include/App/MainFrame.hpp"
#include "wx/arrstr.h"

#include "wx/app.h"
#include "wx/utils.h"
#include <wx/snglinst.h>
#include <string>

wxIMPLEMENT_APP(App);

static wxArrayString runCommand(const std::string& cmd) {
    wxArrayString output;
    wxArrayString errors;
    wxExecute(cmd, output,errors);
    return output;
}



static void loadConfig(State& state) {
}

static void processOptions(State& state,const AppCli& cli) {
    if(cli.basename) {
        for(auto& entry : state.entries) {
            entry = std::filesystem::path(entry.ToStdString()).filename().string();
        }
    }
}

static void processInput(wxArrayString& vec) {
    vec = {};
    std::string line;

    while(std::getline(std::cin,line)) {
        vec.push_back(line);
    }
}


bool App::OnInit() {
    AppCli cli;
    auto status = cli.run(argc, argv);

    wxSingleInstanceChecker* checker  = new wxSingleInstanceChecker;

    if(checker->IsAnotherRunning()) {
        delete checker;
        checker = nullptr;
        return false;
    }

    loadConfig(state);
    processInput(state.entries);
    processOptions(state, cli);


    MainFrame*  frame =  new MainFrame(&state,"hello");
    frame->Show();
    return true;
} 

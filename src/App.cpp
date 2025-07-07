#include "../include/App/App.hpp"
#include "../include/App/MainFrame.hpp"

#include "wx/arrstr.h"

#include "wx/app.h"
#include "wx/log.h"
#include "wx/snglinst.h"
#include "wx/utils.h"
#include <filesystem>
#include <string>

#ifndef CLI_TOOL
#include "../include/App/GlobalKeys.hpp"
#endif

namespace fs = std::filesystem;

IMPLEMENT_APP_NO_MAIN(App)

static wxArrayString runCommand(const std::string& cmd) {
    wxArrayString output;
    wxArrayString errors;
    wxExecute(cmd, output, errors);
    return output;
}

static void loadConfig(State& state) {
}

// static void processOptions(State& state,const AppCli& cli) {
//     if(cli.basename) {
//         for(auto& entry : state.entries) {
//             entry = std::filesystem::path(entry.ToStdString()).filename().string();
//         }
//     }
// }

static void processInput(wxArrayString& vec) {
#ifdef CLI_TOOL
    vec = {};
    std::string line;

    while (std::getline(std::cin, line)) {
        vec.push_back(line);
    }
#else
    const fs::path apps_path = "/Applications";
    if (!fs::exists(apps_path)) {
        wxLogError(std::format("Path: {} does not exist.", apps_path.string()));
    }

    for (const auto& entry : fs::directory_iterator(apps_path)) {
        if (entry.path().extension() == ".app") {
            vec.push_back(entry.path().filename().string());
        }
    }

#endif
}

bool App::OnInit() {
    bool is_cli = false;
#ifdef CLI_TOOL
    is_cli = true;
#else
    wxSingleInstanceChecker* checker = new wxSingleInstanceChecker;
    if (checker->IsAnotherRunning()) {
        delete checker;
        checker = nullptr;
        return false;
    }
#endif

    loadConfig(state);
    processInput(state.entries);

    MainFrame* frame = new MainFrame(&state, "hello", is_cli);
    frame->Show();

#ifndef CLI_TOOL
    registerGlobalHotkey(frame);
#endif

    return true;
}

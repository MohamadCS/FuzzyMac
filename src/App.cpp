#include "../include/App/App.hpp"
#include "../include/App/MainFrame.hpp"
#include "../include/App/GlobalKeys.hpp"
#include "../include/App/MacNativeHandler.hpp"

#include "wx/arrstr.h"

#include "wx/app.h"
#include "wx/log.h"
#include "wx/snglinst.h"
#include "wx/utils.h"
#include <filesystem>
#include <string>


namespace fs = std::filesystem;

IMPLEMENT_APP_NO_MAIN(App)

static wxArrayString runCommand(const std::string& cmd) {
    wxArrayString output;
    wxArrayString errors;
    wxExecute(cmd, output, errors);
    return output;
}

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
            vec.push_back(entry.path().string());
        }
    }

#endif
}

bool App::OnInit() {
    wxSingleInstanceChecker* checker = new wxSingleInstanceChecker;
    if (checker->IsAnotherRunning()) {
        delete checker;
        checker = nullptr;
        return false;
    }


    wxArrayString entries;

    processInput(entries);

    MainFrame* frame = new MainFrame(&state, "FuzzyMac", std::move(entries));
    frame->Show();

    if constexpr(!is_cli) {
        registerGlobalHotkey(frame);
        DeactivateApp();
        frame->Hide();
    }

    return true;
}

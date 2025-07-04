#pragma once

#include "App.hpp"
#include "State.hpp"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>

class SearchFrame : public wxFrame {
public:
    SearchFrame(State* state, const std::string& title);

private:
    State* _state;
    wxPanel* _main_panel;
    wxTextCtrl* _search_text_ctrl;
    wxScrolled<wxPanel>* _search_results_panel;

    void createWidgets();
    void placeWidgets();
    void applyConfig();
    void createBindings();

};

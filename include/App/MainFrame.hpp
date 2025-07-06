#pragma once

#include "State.hpp"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include "ResultEntry.hpp"
#include <wx/textctrl.h>

class MainFrame : public wxFrame {
public:
    MainFrame(State* state, const std::string& title);

private:
    State* state;
    std::size_t selected_idx;

    wxPanel* main_panel;
    std::vector<ResultEntry*> results_vec;
    wxTextCtrl* search_query_text_ctrl;
    wxScrolled<wxPanel>* results_panel;

    void createWidgets();
    void placeWidgets();
    void applyConfig();
    void createBindings();
    void fillData();
    void onKeyDown(wxKeyEvent& event);

    void select(int i);
    void unselect(int i);

};

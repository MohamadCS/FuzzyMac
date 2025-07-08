#pragma once

#include "ResultEntry.hpp"
#include "State.hpp"
#include "wx/arrstr.h"
#include "wx/timer.h"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>

class MainFrame : public wxFrame {
public:
    MainFrame(State* state, const std::string& title, wxArrayString&& arr);
    void wakeup() {
        search_query_text_ctrl->SetFocus();
    }

private:
    State* state;
    std::size_t selected_idx;

    wxPanel* frame_panel;
    wxPanel* main_panel;

    wxArrayString entries; // Contains the raw entries
    std::vector<ResultEntry*> result_entries;

    wxTextCtrl* search_query_text_ctrl;
    wxTimer result_reload_timer;

    wxScrolled<wxPanel>* results_panel;

    void createWidgets();
    void placeWidgets();
    void applyConfig();
    void createBindings();
    void fillData();
    void fillDataCustom();
    void fillFileSearchData();
    void onKeyDown(wxKeyEvent& event);

    void select(int i);
    void unselect(int i);
};

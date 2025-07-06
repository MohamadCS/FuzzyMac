#include "../include/App/MainFrame.hpp"
#include "../include/App/Algorithms.hpp"
#include "../include/App/ResultEntry.hpp"

#include "wx/app.h"
#include "wx/colour.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/log.h"
#include "wx/nonownedwnd.h"
#include "wx/panel.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include <algorithm>
#include <unistd.h>
#include <vector>

MainFrame::MainFrame(State* state, const std::string& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxFRAME_SHAPED),
      state(state),
      selected_idx(0) {

    createWidgets();
    placeWidgets();
    createBindings();
    applyConfig();
    fillData();
}

void MainFrame::createWidgets() {
    main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    main_panel->SetSizer(new wxBoxSizer(wxVERTICAL));

    search_query_text_ctrl =
        new wxTextCtrl(main_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);

    results_panel = new wxScrolled<wxPanel>(main_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    results_panel->SetSizer(new wxBoxSizer(wxVERTICAL));
}

void MainFrame::placeWidgets() {
    Center();
    main_panel->GetSizer()->Add(search_query_text_ctrl, wxSizerFlags(0).Expand());
    main_panel->GetSizer()->Add(results_panel, wxSizerFlags(1).Expand());
}

void MainFrame::applyConfig() {
    results_panel->SetBackgroundColour(state->color_scheme.results_panel_bg);
    results_panel->SetForegroundColour(state->color_scheme.results_panel_fg);

    search_query_text_ctrl->SetForegroundColour(state->color_scheme.searchbar_fg);
    search_query_text_ctrl->SetBackgroundColour(state->color_scheme.searchbar_bg);

    auto font = state->font;
    font.SetPointSize(state->query_font_size);
    search_query_text_ctrl->SetFont(font);
}

void MainFrame::select(int i) {
    results_vec[i]->SetBackgroundColour(state->color_scheme.selected_result_bg);
    results_vec[i]->SetForegroundColour(this->state->color_scheme.selected_result_fg);
}

void MainFrame::unselect(int i) {
    results_vec[i]->SetBackgroundColour(state->color_scheme.result_bg);
    results_vec[i]->SetForegroundColour(state->color_scheme.result_fg);
}

void MainFrame::fillData() {
    results_vec = {};
    for (auto& file : state->entries) {
        auto* entry = new ResultEntry(state, this->results_panel, file.ToStdString());
        results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
        results_vec.push_back(entry);
    }
    if(results_vec.size() > 1) {
        select(0);
    }
}

void MainFrame::createBindings() {
    /*
     * When a change in the query is detected, recalculate the score of each
     * entry in the given list, and order them by decsending order by score,
     * any entry with a score of less than 0 will be discarded.
     * */

    search_query_text_ctrl->Bind(wxEVT_KEY_DOWN, &MainFrame::onKeyDown, this);

    search_query_text_ctrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {

        std::vector<std::pair<int, int>> final{};
        this->selected_idx = 0;

        // calcluate scores

        std::string query = this->search_query_text_ctrl->GetValue().MakeLower().ToStdString();

        auto files_ = this->state->entries;
        for (int i = 0; i < files_.size(); ++i) {
            files_[i].MakeLower();
            int score = fuzzyScore(files_[i], query);
            if (score >= 0) {
                final.push_back({i, score});
            }
        }

        // sort in descending order according to score
        std::sort(final.begin(), final.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });

        // clear last results
        this->results_panel->GetSizer()->Clear(true);
        results_vec = {};

        // Add entries to the final result
        for (auto& [file_idx, _] : final) {
            auto* entry = new ResultEntry(state, this->results_panel, state->entries[file_idx].ToStdString());
            results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
            results_vec.push_back(entry);
        }

        if(results_vec.size() >= 1) {
            select(0);
        }

        // // refresh
        results_panel->Layout();
        results_panel->GetSizer()->Layout();
        results_panel->FitInside(); // automatically calc the virtual size
    });
}

// TODO: ORGANIZE this miss.
void MainFrame::onKeyDown(wxKeyEvent& event) {
    auto modifiers = event.GetModifiers();
    if ((modifiers & wxMOD_RAW_CONTROL) && event.GetKeyCode() == 'C') {
        wxTheApp->Exit();
    } else if ((modifiers & wxMOD_RAW_CONTROL) && event.GetKeyCode() == 'N') {
        if (results_vec.empty() || selected_idx == results_vec.size() - 1) {
            return;
        } else {
            auto next = selected_idx + 1;
            select(next);
            unselect(selected_idx);
            selected_idx++;
        }
    } else if ((modifiers & wxMOD_RAW_CONTROL) && event.GetKeyCode() == 'P') {
        if (results_vec.empty() || selected_idx == 0) {
            return;
        } else {
            auto prev = selected_idx - 1;
            select(prev);
            unselect(selected_idx);
            selected_idx--;
            results_panel->Layout();
            results_panel->Refresh();
        }

    } else if (event.GetKeyCode() == WXK_RETURN) {
        std::string out = results_vec.empty() ? "" : results_vec[selected_idx]->getValue();
        std::cout << out;
        wxTheApp->Exit();
    } else {
        event.Skip();
    }


    if (results_panel->GetVirtualSize().GetHeight() > results_panel->GetClientSize().GetHeight()) {
        results_panel->SetScrollRate(0, results_vec[0]->GetSize().GetHeight());
        results_panel->Scroll(0, selected_idx);
    }


    results_panel->Layout();
    results_panel->Refresh();
}

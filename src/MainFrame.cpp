#include "../include/App/MainFrame.hpp"
#include "../include/App/Algorithms.hpp"
#include "../include/App/MacNativeHandler.hpp"
#include "../include/App/ResultEntry.hpp"

#include "wx/app.h"
#include "wx/arrstr.h"
#include "wx/colour.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/log.h"
#include "wx/nonownedwnd.h"
#include "wx/panel.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include "wx/timer.h"
#include "wx/toplevel.h"
#include "wx/utils.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

MainFrame::MainFrame(State* state, const std::string& title, wxArrayString&& arr)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,
              wxFRAME_SHAPED | wxNO_BORDER | wxFRAME_NO_TASKBAR),
      state(state),
      selected_idx(0),
      entries(std::move(arr)) {

    createWidgets();
    placeWidgets();
    createBindings();
    applyConfig();
    fillData();

    search_query_text_ctrl->SetFocus();
}

void MainFrame::createWidgets() {
    frame_panel = new wxPanel(this);
    main_panel = new wxPanel(frame_panel);

    frame_panel->SetSizer(new wxBoxSizer(wxVERTICAL));
    main_panel->SetSizer(new wxBoxSizer(wxVERTICAL));

    frame_panel->GetSizer()->Add(main_panel, wxSizerFlags().Expand().Border(wxALL, state->border_width));

    search_query_text_ctrl =
        new wxTextCtrl(main_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);

    results_panel = new wxScrolled<wxPanel>(main_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    results_panel->SetSizer(new wxBoxSizer(wxVERTICAL));
}

void MainFrame::placeWidgets() {
    Center();
    main_panel->GetSizer()->Add(search_query_text_ctrl, wxSizerFlags(0).Expand());
    main_panel->GetSizer()->Add(results_panel, wxSizerFlags(1).Expand());
    SetWindowStyleFlag(wxSTAY_ON_TOP);
}

void MainFrame::applyConfig() {
    results_panel->SetBackgroundColour(state->color_scheme.results_panel_bg);
    results_panel->SetForegroundColour(state->color_scheme.results_panel_fg);

    search_query_text_ctrl->SetForegroundColour(state->color_scheme.searchbar_fg);
    search_query_text_ctrl->SetBackgroundColour(state->color_scheme.searchbar_bg);

    main_panel->SetForegroundColour(state->color_scheme.results_panel_fg);
    main_panel->SetBackgroundColour(state->color_scheme.results_panel_bg);

    frame_panel->SetForegroundColour(state->color_scheme.border_fg);
    frame_panel->SetBackgroundColour(state->color_scheme.border_bg);

    auto font = state->font;
    font.SetPointSize(state->query_font_size);
    search_query_text_ctrl->SetFont(font);
}

void MainFrame::select(int i) {
    result_entries[i]->SetBackgroundColour(state->color_scheme.selected_result_bg);
    result_entries[i]->SetForegroundColour(this->state->color_scheme.selected_result_fg);
}

void MainFrame::unselect(int i) {
    result_entries[i]->SetBackgroundColour(state->color_scheme.result_bg);
    result_entries[i]->SetForegroundColour(state->color_scheme.result_fg);
}

void MainFrame::createBindings() {
    /*
     * When a change in the query is detected, recalculate the score of each
     * entry in the given list, and order them by decsending order by score,
     * any entry with a score of less than 0 will be discarded.
     * */

    search_query_text_ctrl->Bind(wxEVT_TEXT, [this](wxCommandEvent&) {
        const auto& query = this->search_query_text_ctrl->GetValue();
        int time = query.size() >= 1 && query[0] == ' ' && !is_cli ? 200 : 50;
        result_reload_timer.Start(time, true);

    });

    search_query_text_ctrl->Bind(wxEVT_KEY_DOWN, &MainFrame::onKeyDown, this);

    result_reload_timer.Bind(wxEVT_TIMER, [this](wxTimerEvent& evt) {
        results_panel->GetSizer()->Clear(true);
        result_entries = {};

        if constexpr (is_cli) {
            fillDataCustom();
        } else {
            const wxString& query = this->search_query_text_ctrl->GetValue();
            if (query.size() >= 2 && query[0] == ' ') {
                fillFileSearchData();
            } else {
                fillDataCustom();
            }
        }

        if (result_entries.size() >= 1) {
            select(0);
        }

        results_panel->Layout();
        results_panel->GetSizer()->Layout();
        results_panel->FitInside(); // automatically calc the virtual size
    });
}

// TODO: ORGANIZE this miss.
void MainFrame::onKeyDown(wxKeyEvent& event) {
    auto modifiers = event.GetModifiers();
    if (event.GetKeyCode() == WXK_ESCAPE) {

        if constexpr (is_cli) {
            wxTheApp->Exit();
        } else {
            DeactivateApp();
            Hide();
        }

    } else if ((modifiers & wxMOD_RAW_CONTROL) && event.GetKeyCode() == 'N') {
        if (result_entries.empty() || selected_idx == result_entries.size() - 1) {
            return;
        } else {
            auto next = selected_idx + 1;
            select(next);
            unselect(selected_idx);
            selected_idx++;
        }
    } else if ((modifiers & wxMOD_RAW_CONTROL) && event.GetKeyCode() == 'P') {
        if (result_entries.empty() || selected_idx == 0) {
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

        if constexpr (is_cli) {
            std::string out = result_entries.empty() ? "" : result_entries[selected_idx]->getValue();
            std::cout << out;
            wxTheApp->Exit();
        } else {
            wxExecute(
                std::format("open \"{}\"", result_entries[selected_idx]->getValue())); // get the original original
            DeactivateApp();
            Hide();
        }
    } else {
        event.Skip();
    }

    if (results_panel->GetVirtualSize().GetHeight() > results_panel->GetClientSize().GetHeight()) {
        results_panel->SetScrollRate(0, result_entries[0]->GetSize().GetHeight());
        results_panel->Scroll(0, selected_idx);
    }

    results_panel->Layout();
    results_panel->Refresh();
}

void MainFrame::fillData() {
    result_entries = {};
    int i = 0;
    for (auto& file : entries) {
        auto* entry = new ResultEntry(state, this->results_panel, file);
        results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
        result_entries.push_back(entry);
    }
}

void MainFrame::fillDataCustom() {

    const wxString& query = search_query_text_ctrl->GetValue().MakeLower();

    std::vector<std::pair<int, int>> final{};
    selected_idx = 0;

    auto files_ = entries;
    // add files with score >= 0
    for (int i = 0; i < files_.size(); ++i) {
        files_[i].MakeLower();
        int score = fuzzyScore(files_[i], query);
        if (score >= 0) {
            final.push_back({i, score});
        }
    }

    // sort in descending order according to score
    std::sort(final.begin(), final.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });

    // Add entries to the final result
    for (auto& [file_idx, _] : final) {
        auto* entry = new ResultEntry(state, results_panel, entries[file_idx]);
        results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
        result_entries.push_back(entry);
    }
}

void MainFrame::fillFileSearchData() {

    wxString query = search_query_text_ctrl->GetValue();
    query = query.SubString(1, query.size() - 1);

    std::vector<std::string> paths{fs::absolute(std::format("{}/Library/Mobile Documents/", std::getenv("HOME")))};

    auto files = spotlightSearch(std::format("kMDItemDisplayName == '{}*'c", query.ToStdString()), paths);

    for (const auto& file : files) {
        auto* entry = new ResultEntry(state, results_panel, file);
        results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
        result_entries.push_back(entry);
    }
}

#include "../include/App/SearchFrame.hpp"
#include "../include/App/Algorithms.hpp"
#include "../include/App/SearchResultPanel.hpp"

#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/panel.h"
#include "wx/scrolwin.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"
#include <unistd.h>

SearchFrame::SearchFrame(State* state, const std::string& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED),
      _state(state) {

    createWidgets();
    placeWidgets();
    createBindings();
    applyConfig();
}

void SearchFrame::createWidgets() {
    _main_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    _main_panel->SetSizer(new wxBoxSizer(wxVERTICAL));

    _search_text_ctrl =
        new wxTextCtrl(_main_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);

    _search_results_panel = new wxScrolled<wxPanel>(_main_panel, wxID_ANY);
    _search_results_panel->SetSizer(new wxBoxSizer(wxVERTICAL));
}

void SearchFrame::placeWidgets() {
    Center();
    _main_panel->GetSizer()->Add(_search_text_ctrl, wxSizerFlags(0).Expand());
    _main_panel->GetSizer()->Add(_search_results_panel, wxSizerFlags(1).Expand());
}

void SearchFrame::applyConfig() {
    _search_results_panel->SetScrollRate(0, 5);
    _search_text_ctrl->SetForegroundColour(_state->color_scheme.searchbar_fg);
    _search_text_ctrl->SetBackgroundColour(_state->color_scheme.searchbar_bg);
    auto font = _state->font;
    font.SetPointSize(_state->query_font_size);
    _search_text_ctrl->SetFont(font);
}

void SearchFrame::createBindings() {
    const std::vector<std::string> files = {"hello.cpp", "codegen.cpp", "extract.hpp", "hello_world.cpp"};

    /*
     * When a change in the query is detected, recalculate the score of each
     * entry in the given list, and order them by decsending order by score,
     * any entry with a score of less than 0 will be discarded.
     * */
    _search_text_ctrl->Bind(wxEVT_TEXT, [files, this](wxCommandEvent&) {
        std::vector<std::pair<std::string, int>> final{};

        // calcluate scores
        for (auto file : files) {
            int score = fuzzyScore(file, this->_search_text_ctrl->GetValue().ToStdString());
            if (score >= 0) {
                final.push_back({file, score});
            }
        }

        // sort in descending order according to score
        std::sort(final.begin(), final.end(), [](const auto& lhs, const auto& rhs) { return lhs.second > rhs.second; });

        // clear last results
        this->_search_results_panel->GetSizer()->Clear(true);

        // Add entries to the final result
        for (auto& [file, _] : final) {
            auto* entry = new SearchResultPanel(_state, this->_search_results_panel, file);
            _search_results_panel->GetSizer()->Add(entry, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 0));
        }

        // refresh
        _search_results_panel->Layout();
        _search_results_panel->Refresh();
        _search_results_panel->GetSizer()->Layout();
    });
}

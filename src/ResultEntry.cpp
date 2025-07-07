#include "../include/App/ResultEntry.hpp"
#include "wx/colour.h"
#include "wx/dcclient.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/panel.h"
#include "wx/sizer.h"

ResultEntry::ResultEntry(State* state, wxWindow* parent, const wxString& value)
    : state(state),
      wxPanel(parent) {

    SetSizer(new wxBoxSizer(wxHORIZONTAL));

    SetBackgroundColour(state->color_scheme.result_bg);
    SetForegroundColour(state->color_scheme.result_fg);

    SetMinSize(wxSize(100, 50));

    value_static_text = new wxStaticText(this, wxID_ANY, value, wxDefaultPosition, wxDefaultSize);

    GetSizer()->Add(value_static_text, wxSizerFlags().CenterVertical());
    value_static_text->SetFont(state->font);

    Bind(wxEVT_PAINT, &ResultEntry::onPaint, this);
}

std::string ResultEntry::getValue() const {
    return value_static_text->GetLabel().ToStdString();
}

void ResultEntry::onPaint(wxPaintEvent&) {
    wxPaintDC dc(this);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRoundedRectangle(wxRect(wxDefaultPosition, GetSize()), 0);
}

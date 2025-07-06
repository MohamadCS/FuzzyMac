#include "../include/App/ResultEntry.hpp"
#include "wx/colour.h"
#include "wx/dcclient.h"
#include "wx/event.h"
#include "wx/gdicmn.h"
#include "wx/panel.h"
#include "wx/sizer.h"

ResultEntry::ResultEntry(State* state, wxWindow* parent, const std::string& value)
    : _state(state),
      wxPanel(parent),
      _value(value) {

    SetSizer(new wxBoxSizer(wxHORIZONTAL));

    SetBackgroundColour(_state->color_scheme.result_bg);
    SetForegroundColour(_state->color_scheme.result_fg);

    SetMinSize(wxSize(100, 50));

    _value_static_text = new wxStaticText(this, wxID_ANY, value, wxDefaultPosition, wxDefaultSize);

    GetSizer()->Add(_value_static_text, wxSizerFlags().CenterVertical());
    _value_static_text->SetFont(_state->font);

    Bind(wxEVT_PAINT, &ResultEntry::onPaint, this);
}

std::string ResultEntry::getValue() const {
    return _value_static_text->GetLabel().ToStdString();
}

void ResultEntry::onPaint(wxPaintEvent&) {
    wxPaintDC dc(this);
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRoundedRectangle(wxRect(wxDefaultPosition, GetSize()), 0);
}

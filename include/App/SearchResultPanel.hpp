#pragma once

#include "State.hpp"
#include "wx/event.h"
#include "wx/textentry.h"
#include "wx/panel.h"
#include "wx/stattext.h"

class SearchResultPanel : public wxPanel {
public:
    SearchResultPanel(State* state,wxWindow* parent, const std::string& value); 
private:
    State* _state;
    std::string _value;
    wxStaticText* _value_static_text;

    void onPaint(wxPaintEvent&);

};

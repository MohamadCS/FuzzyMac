#pragma once

#include "State.hpp"
#include "wx/event.h"
#include "wx/textentry.h"
#include "wx/panel.h"
#include "wx/stattext.h"

class ResultEntry : public wxPanel {
public:
    ResultEntry(State* state,wxWindow* parent, const wxString& value); 
    std::string getValue() const;
private:
    State* state;
    std::string value;
    wxStaticText* value_static_text;

    void onPaint(wxPaintEvent&);

};

#pragma once

#include "State.hpp"
#include "wx/event.h"
#include "wx/textentry.h"
#include "wx/panel.h"
#include "wx/stattext.h"

class ResultEntry : public wxPanel {
public:
    ResultEntry(State* state,wxWindow* parent, const std::string& value); 
    std::string getValue() const;
private:
    State* _state;
    std::string _value;
    wxStaticText* _value_static_text;


    void onPaint(wxPaintEvent&);

};

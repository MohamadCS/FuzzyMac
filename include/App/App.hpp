#pragma once

#include <wx/app.h>
#include "State.hpp"




class App : public wxApp {
public:
    bool OnInit() override;
private:
    State _state;
};


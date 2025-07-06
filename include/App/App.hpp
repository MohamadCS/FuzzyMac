#pragma once

#include <wx/app.h>
#include "State.hpp"




struct App : public wxApp {
    bool OnInit() override;
    State state;
};




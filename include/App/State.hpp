#pragma once

#include "wx/colour.h"
#include "wx/font.h"


struct ColorScheme {
    wxColor searchbar_fg = wxColor(87, 82, 121);
    wxColor searchbar_bg = wxColor(242, 233, 222);
    wxColor result_fg = wxColor(87, 82, 121);
    wxColor result_bg = wxColor(242, 233, 222);
    wxColor results_panel_fg = wxColor(87, 82, 121);
    wxColor results_panel_bg = wxColor(242, 233, 222);
};

struct State {
    ColorScheme color_scheme;
    wxFont font = wxFont(wxFontInfo(14).FaceName("JetBrainsMono Nerd Font"));
    int results_font_size = 14;
    int query_font_size = 25;
};


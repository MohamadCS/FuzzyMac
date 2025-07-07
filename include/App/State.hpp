#pragma once

#include "wx/colour.h"
#include "wx/font.h"


#ifndef CLI_TOOL
constexpr bool is_cli = false;
#else 
constexpr bool is_cli = true;
#endif


struct ColorScheme {
    wxColor searchbar_fg = wxColor(87, 82, 121);
    wxColor searchbar_bg = wxColor(242, 233, 222);
    wxColor result_fg = wxColor(87, 82, 121);
    wxColor result_bg = wxColor(242, 233, 222);
    wxColor selected_result_fg = wxColor(87, 82, 121);
    wxColor selected_result_bg = wxColor(206, 202, 205);
    wxColor results_panel_fg = wxColor(87, 82, 121);
    wxColor results_panel_bg = wxColor(242, 233, 222);
    wxColor border_bg = wxColor(87, 82, 121);
    wxColor border_fg = wxColor(87, 82, 121);
};

struct State {
    ColorScheme color_scheme;
    wxFont font = wxFont(wxFontInfo(14).FaceName("JetBrainsMono Nerd Font"));
    int results_font_size = 14;
    int query_font_size = 30;
    int border_width = 3;
};

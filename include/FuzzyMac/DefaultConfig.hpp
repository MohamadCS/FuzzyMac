#pragma once

#include <toml++/toml.h>

const auto DEFAULT_CONFIG = toml::parse(R"(
font = "JetBrainsMono Nerd Font"

border_size = 2 
animations = false
corner_radius = 20 
info_panel = false
opacity = 1.00

[colors]
outer_border = "#545464"
inner_border = "#dddddb"

[colors.query_input]

selection = "#545464"
selection_background = "#e2e1df"
text = "#545464"
background = "#f2f1ef"

[colors.results_list]
selection = "#545464"
selection_background = "#e2e1df"
text = "#545464"
background = "#f2f1ef"
scrollbar_color = "#545464"
scrollbar_hold_color = "#4d699b"


[colors.mode_label]
text = "#43436c"
background = "#f2f1ef"

[mode.apps]
dirs = [
  "/Applications/",
  "/System/Applications",
  "/Applications/Utilities/",
  "/System/Applications/Utilities",
]
show_icons = true
apps = [
  "/System/Library/CoreServices/Finder.app",
  "/System/Volumes/Preboot/Cryptexes/App/System/Applications/Safari.app",
]

[mode.files]
show_icons = true
dirs = [
  "$HOME/Library/Mobile Documents/com~apple~CloudDocs/",
  "$HOME/Pictures/",
]

[mode.clipboard]

blacklist = ["Bitwarden", "Ghostty", "FuzzyMac"]
)");

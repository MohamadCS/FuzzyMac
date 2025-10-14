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
outer_border = "#4d699b"
inner_border = "#f2f1ef"

[colors.query_input]

selection = "#545464"
selection_background = "#c7d7e0"
text = "#545464"
background = "#f2f1ef"

[colors.results_list]
selection = "#545464"
selection_background = "#c7d7e0"
text = "#545464"
background = "#f2f1ef"
scrollbar_color = "#4d699b"
scrollbar_hold_color = "#4e8ca2"


[colors.mode_label]
text = "#545464"
background = "#f2f1ef"

[mode.apps]
dirs = [
  "/System/Applications",
  "/Applications/",
  "/Applications/Utilities/",
  "/System/Applications/Utilities",
]
show_icons = true
apps = ["/System/Library/CoreServices/Finder.app"]

script_paths = ["$HOME/.config/FuzzyMac/scripts/"]

[mode.files]
show_icons = true
dirs = [
  "$HOME/Library/Mobile Documents/com~apple~CloudDocs/",
  "$HOME/Pictures/",
]

[mode.clipboard]
blacklist = ["Bitwarden", "FuzzyMac", "Ghostty"]
limit = 100

[mode.wallpaper]
show_icons = true
paths = ["$HOME/Pictures/Wallpapers/"]
)");
